#include "BoardWidget.h"

#include <QPainter>
#include <QMouseEvent>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QPen>
#include <QSizePolicy>
#include <QtMath>

BoardWidget::BoardWidget(QWidget *parent)
    : QWidget(parent),
      currentPlayer(Stone::BLACK),
      lastmove(-1, -1),
      boardPadding(36)
{
    setMinimumSize(650, 650);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_OpaquePaintEvent);
}

QSize BoardWidget::sizeHint() const
{
    return QSize(760, 760);
}

void BoardWidget::resetBoard(){
    board = Board() ;
    currentPlayer = Stone::BLACK ;
    update() ;
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
    const qreal step = gridStep();
    const qreal radius = step * 0.46;

    for (int x = 0; x < Board::SIZE; ++x) {
        for (int y = 0; y < Board::SIZE; ++y) {
            const Stone stone = board.get(x, y);

            // 关键修复：空位不画
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
            QRadialGradient gradient(center - QPointF(radius * 0.35, radius * 0.35),
                                     radius * 1.2);

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

            // 最后一手高亮
            if (lastmove == QPoint(x, y)) {
                painter.setPen(Qt::NoPen);
                painter.setBrush(stone == Stone::BLACK
                                 ? QColor(255, 210, 70)
                                 : QColor(220, 60, 60));

                const qreal markerRadius = qMax(3.0, step * 0.11);
                painter.drawEllipse(center, markerRadius, markerRadius);
            }

            painter.restore();
        }
    }
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
void BoardWidget::mousePressEvent(QMouseEvent *event)
{
    QPoint boardPos = pixelToBoard(event->pos());

    int x = boardPos.x();
    int y = boardPos.y();

    if (x < 0 || x >= Board::SIZE || y < 0 || y >= Board::SIZE) {
        return;
    }

    Stone playedColor = currentPlayer;

    if (board.playMove(x, y, currentPlayer)) {
        emit movePlayed(x, y, playedColor);

        currentPlayer = board.opponent(currentPlayer);
        update();
    }
}