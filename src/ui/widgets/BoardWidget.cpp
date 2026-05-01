#include "BoardWidget.h"
#include "../../ai/MCTS.h"
#include "../../ai/MCTSNode.h"

#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPen>
#include <QSizePolicy>
#include <QtMath>
#include <QDir>
#include <QFileInfo>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent),
      lastmove(-1, -1),
      boardPadding(36),
      finished(false),
      aicolor(Stone::WHITE),
      aiEnabled(false) 
{
    setMinimumSize(650, 650);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setupSounds();
}

QSize BoardWidget::sizeHint() const
{
    return QSize(760, 760);
}

void BoardWidget::resetBoard(){
    game.reset();
    lastmove = QPoint(-1, -1);
    finished = false;
    finishText.clear();
    update();

    emit gameReset();
    emit turnChanged(game.getCurrentPlayer());

}

Stone BoardWidget::currentPlayer() const{
    return game.getCurrentPlayer() ;
}

QRectF BoardWidget::boardRect() const
{
    const qreal available = qMin(width(), height()) - boardPadding * 2.0;
    const qreal step = qMax(24.0, available / qreal(Board::SIZE - 1));
    const qreal side = step * (Board::SIZE - 1);

    const qreal left = (width() - side) / 2.0;
    const qreal top  = (height() - side) / 2.0;

    return QRectF(left, top, side, side);
}

bool BoardWidget::isGameFinished() const{
    return finished ;
}

void BoardWidget::updateLastMoveFromHistory(){
    lastmove = QPoint(-1, -1);

    const auto &history = game.getHistory();
    for (auto it = history.rbegin(); it != history.rend(); ++it) {
        if (!it->isPass && it->x >= 0 && it->y >= 0) {
            lastmove = QPoint(it->x, it->y);
            return;
        }
    }
}

qreal BoardWidget::gridStep() const
{
    return boardRect().width() / qreal(Board::SIZE - 1);
}

QPointF BoardWidget::boardPoint(int x, int y) const
{
    const QRectF rect = boardRect();
    const qreal step = gridStep();
    return QPointF(rect.left() + x * step, rect.top() + y * step);
}

// 处理相应的悔棋的边界部分
void BoardWidget::undoLastMove()
{
    if (!game.undo()) {
        emit illegalAction("当前没有可悔的棋");
        return;
    }

    finished = false;
    finishText.clear();
    updateLastMoveFromHistory();
    update();

    emit moveUndone();
    emit turnChanged(game.getCurrentPlayer());
}

void BoardWidget::passTurn(){
    if (finished) {
        emit illegalAction("对局已经结束，请重新开始");
        return;
    }

    if (aiEnabled && game.getCurrentPlayer() == aicolor){
        return ;
    }

    Stone playedColor = game.getCurrentPlayer();
    game.playPass();
    lastmove = QPoint(-1, -1);
    update();

    emit passPlayed(playedColor);
    emit turnChanged(game.getCurrentPlayer());

    const auto &history = game.getHistory();
    if (history.size() >= 2 && history[history.size() - 1].isPass && history[history.size() - 2].isPass) {
        finished = true;
        finishText = "双方连续停一手，对局结束";
        emit gameOver(finishText);
        update();
    }

    if (aiEnabled && game.getCurrentPlayer() == aicolor){
        return ;
    }
}

void BoardWidget::resignCurrentPlayer(){
    if (finished) {
        emit illegalAction("对局已经结束，请重新开始");
        return;
    }

    Stone loser = game.getCurrentPlayer();
    Stone winner = game.getBoard().opponent(loser);

    finished = true;
    finishText = QString("%1方认输，%2方胜")
        .arg(loser == Stone::BLACK ? "黑" : "白")
        .arg(winner == Stone::BLACK ? "黑" : "白");

    emit gameOver(finishText);
    update();
}

bool BoardWidget::forcePass()
{
    if (finished) {
        return false;
    }

    Stone playedColor = game.getCurrentPlayer();
    game.playPass();
    lastmove = QPoint(-1, -1);
    update();

    emit passPlayed(playedColor);
    emit turnChanged(game.getCurrentPlayer());

    checkGameEndAfterPass();
    return true;
}

void BoardWidget::checkGameEndAfterPass()
{
    const auto& history = game.getHistory();
    if (history.size() >= 2 &&
        history[history.size() - 1].isPass &&
        history[history.size() - 2].isPass) {
        finished = true;
        finishText = "双方连续停一手，对局结束";
        emit gameOver(finishText);
        update();
    }
}

const Game& BoardWidget::getGame() const
{
    return game;
}

void BoardWidget::drawBoard(QPainter &painter){
    const QRectF gridRect = boardRect();
    const qreal step = gridStep();

    const QRectF woodRect = gridRect.adjusted(-step * 0.65, -step * 0.65,
                                              step * 0.65,  step * 0.65);

    QLinearGradient woodGradient(woodRect.topLeft(), woodRect.bottomRight());
    woodGradient.setColorAt(0.0, QColor(239, 203, 141));
    woodGradient.setColorAt(0.55, QColor(228, 190, 125));
    woodGradient.setColorAt(1.0, QColor(213, 171, 105));

    painter.save();
    painter.setPen(QPen(QColor(160, 122, 76), 1.2));
    painter.setBrush(woodGradient);
    painter.drawRoundedRect(woodRect, 16, 16);

    QPen linePen(QColor(58, 42, 24), 1.15);
    linePen.setCapStyle(Qt::RoundCap);
    painter.setPen(linePen);

    for (int i = 0; i < Board::SIZE; ++i) {
        const qreal x = gridRect.left() + i * step;
        const qreal y = gridRect.top() + i * step;

        painter.drawLine(QPointF(gridRect.left(), y), QPointF(gridRect.right(), y));
        painter.drawLine(QPointF(x, gridRect.top()), QPointF(x, gridRect.bottom()));
    }

    painter.restore();
}

void BoardWidget::drawStarPoints(QPainter &painter) {
    const qreal step = gridStep();
    const qreal radius = qMax(3.5, step * 0.10);
    const int starPositions[3] = {3, 9, 15};

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(45, 30, 18));

    for (int x : starPositions) {
        for (int y : starPositions) {
            const QPointF center = boardPoint(x, y);
            painter.drawEllipse(center, radius, radius);
        }
    }

    painter.restore();
}

void BoardWidget::drawStones(QPainter &painter){
    const Board &board = game.getBoard();
    const qreal step = gridStep();
    const qreal radius = step * 0.46;

    const int totalMoves = (int)game.getHistory().size();
    const int showLastN = 15;  

    for (int x = 0; x < Board::SIZE; ++x) {
        for (int y = 0; y < Board::SIZE; ++y) {
            const Stone stone = board.get(x, y);

            if (stone != Stone::BLACK && stone != Stone::WHITE) {
                continue;
            }

            const QPointF center = boardPoint(x, y);

            painter.save();

            // 阴影
            painter.setPen(Qt::NoPen);
            painter.setBrush(QColor(0, 0, 0, 45));
            painter.drawEllipse(center + QPointF(step * 0.05, step * 0.06), radius, radius);

            // 棋子本体
            QRadialGradient gradient(center - QPointF(radius * 0.35, radius * 0.35),radius * 1.2);

            if (stone == Stone::BLACK) {
                gradient.setColorAt(0.0, QColor(88, 88, 88));
                gradient.setColorAt(0.55, QColor(35, 35, 35));
                gradient.setColorAt(1.0, QColor(10, 10, 10));
                painter.setPen(QPen(QColor(25, 25, 25), 1.0));
            } else {
                gradient.setColorAt(0.0, QColor(255, 255, 255));
                gradient.setColorAt(0.55, QColor(244, 244, 244));
                gradient.setColorAt(1.0, QColor(214, 214, 214));
                painter.setPen(QPen(QColor(140, 140, 140), 1.0));
            }

            painter.setBrush(gradient);
            painter.drawEllipse(center, radius, radius);

            int moveNum = getMoveNumberAt(x, y);

            if (moveNum > 0 && moveNum > totalMoves - showLastN) {
                painter.save();

                QFont font = painter.font();
                font.setBold(true);
                font.setPixelSize((int)(radius * 0.85));
                painter.setFont(font);

                // 最新一手用醒目颜色，其余根据黑白棋反色
                if (moveNum == totalMoves) {
                    painter.setPen(stone == Stone::BLACK
                                   ? QColor(255, 220, 80)
                                   : QColor(210, 40, 40));
                } else {
                    painter.setPen(stone == Stone::BLACK ? Qt::white : Qt::black);
                }

                QRectF textRect(
                    center.x() - radius,
                    center.y() - radius,
                    radius * 2,
                    radius * 2
                );

                painter.drawText(textRect, Qt::AlignCenter, QString::number(moveNum));

                painter.restore();
            }


            // 最后一手高亮
            if (lastmove == QPoint(x, y)) {
                painter.setBrush(Qt::NoBrush);
                painter.setPen(QPen(QColor(45, 91, 150), qMax(2.0, step * 0.065)));
                painter.drawEllipse(center, radius * 0.68, radius * 0.68);

                painter.setPen(Qt::NoPen);
                painter.setBrush(QColor(209, 89, 65));
                painter.drawEllipse(center, qMax(2.6, step * 0.075), qMax(2.6, step * 0.075));
            }
            
            painter.restore() ;
        }
    }
}

void BoardWidget::drawFinishedOverlay(QPainter &painter)
{
    if (!finished) {
        return;
    }

    painter.save();
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(0, 0, 0, 90));
    painter.drawRoundedRect(rect().adjusted(40, 40, -40, -40), 12, 12);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(18);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(rect(), Qt::AlignCenter, finishText);
    painter.restore();
}

void BoardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.fillRect(rect(), QColor(222, 184, 135));

    drawBoard(painter);
    drawStarPoints(painter);
    drawStones(painter);
    drawFinishedOverlay(painter) ;
}

// 这一部分的函数是需要将像素坐标转化为棋盘上面的坐标
QPoint BoardWidget::pixelToBoard(const QPoint &pos) const
{
    const QRectF rect = boardRect();
    const qreal step = gridStep();

    const QRectF clickableRect = rect.adjusted(-step * 0.5, -step * 0.5,
                                                step * 0.5,  step * 0.5);

    if (!clickableRect.contains(pos)) {
        return QPoint(-1, -1);
    }

    const int x = qRound((pos.x() - rect.left()) / step);
    const int y = qRound((pos.y() - rect.top()) / step);

    if (x < 0 || x >= Board::SIZE || y < 0 || y >= Board::SIZE) {
        return QPoint(-1, -1);
    }

    return QPoint(x, y);
}


// 最后的函数将所有界面与鼠标联系起来
void BoardWidget::mousePressEvent(QMouseEvent *event){
    if (event->button() != Qt::LeftButton) {
        return;
    }

    if (finished) {
        emit illegalAction("对局已经结束，请重新开始");
        return;
    }

    if (aiEnabled && game.getCurrentPlayer() == aicolor){
        return ;
    }

    QPoint boardPos = pixelToBoard(event->pos());
    int x = boardPos.x();
    int y = boardPos.y();

    if (x < 0 || x >= Board::SIZE || y < 0 || y >= Board::SIZE) {
        return;
    }

    Stone playedColor = game.getCurrentPlayer();

    if (game.isKo(x, y, playedColor)) {
        emit illegalAction("打劫：此处不能立即提回");
        return;
    }
    
    if (game.playMove(x, y)) {
        lastmove = QPoint(x, y);
        update();
        playStoneSound();
        emit movePlayed(x, y, playedColor);
        emit turnChanged(game.getCurrentPlayer());
    } 
    else {
        emit illegalAction("非法落子：该位置不可下");
    }
}

void BoardWidget::loadGame(const Game& loadedGame){
    game = loadedGame ;
    finished = false ;
    finishText.clear() ;
    updateLastMoveFromHistory() ;
    update() ;

    emit turnChanged(game.getCurrentPlayer()) ;
}

void BoardWidget::setAIEnabled(bool Enabled){
    aiEnabled = Enabled ;
}

void BoardWidget::setAIcolor(Stone color){
    aicolor = color ;
}


int BoardWidget::getMoveNumberAt(int x, int y) const
{
    const std::vector<RecordMove>& history = game.getHistory();

    for (int i = (int)history.size() - 1; i >= 0; --i) {
        const RecordMove& record = history[i];

        if (record.isPass) {
            continue;
        }

        if (record.x == x && record.y == y) {
            return i + 1;
        }
    }

    return 0;
}

bool BoardWidget::forcePlayMove(int x, int y)
{
    if (finished) {
        return false;
    }

    Stone color = game.getCurrentPlayer();

    if (!game.playMove(x, y)) {
        emit illegalAction("AI 返回了非法落点");
        return false;
    }

    lastmove = QPoint(x, y);
    update();
    playStoneSound();

    emit movePlayed(x, y, color);
    emit turnChanged(game.getCurrentPlayer());
    return true;
}

void BoardWidget::setupSounds()
{
    stoneSound = new QSoundEffect(this);

    QString projectRoot = QDir::currentPath();
    QString soundPath = projectRoot + "/assets/sounds/stone.wav";

    if (!QFileInfo::exists(soundPath)) {
        QString appDir = QCoreApplication::applicationDirPath();
        QDir dir(appDir);
        dir.cdUp();
        soundPath = dir.filePath("assets/sounds/stone.wav");
    }

    stoneSound->setSource(QUrl::fromLocalFile(soundPath));
    stoneSound->setVolume(0.45f);

    qDebug() << "stone sound path:" << soundPath;
}

void BoardWidget::playStoneSound()
{
    if (!stoneSound) {
        return;
    }

    stoneSound->stop();
    stoneSound->play();
}