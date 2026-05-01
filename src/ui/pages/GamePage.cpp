#include "GamePage.h"
#include "BoardWidget.h"
#include "core/Board.h"
#include "WinRateChartWidget.h"
#include "ReviewDialog.h"
#include "../sgf/sgf_writer.h"
#include "../sgf/sgf_utils.h"
#include "../../ai/PythonEvaluator.h"
#include "../../ai/MCTS.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFont>
#include <QFrame>
#include <QGroupBox>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QDialog>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>
#include <QtGlobal>

#include <cmath>

GamePage::GamePage(QWidget *parent)
    : QWidget(parent),
      backButton(nullptr),
      passButton(nullptr),
      undoButton(nullptr),
      resignButton(nullptr),
      restartButton(nullptr),
      boardwidget(nullptr),
      statusLabel(nullptr),
      currentTurnLabel(nullptr),
      winRateLabel(nullptr),
      scoreLabel(nullptr),
      moveListWidget(nullptr),
      openSGFButton(nullptr),
      saveSGFButton(nullptr),
      stepForward(nullptr) ,
      stepBackward(nullptr) ,
      winRateChart(nullptr) ,
      reviewButton(nullptr)
{
    setupUI();
    setupConnections();
    resetInfoPanel();

    aiWatcher = new QFutureWatcher<Move>(this);

    connect(aiWatcher, &QFutureWatcher<Move>::finished, this, [this]() {
        Move aiMove = aiWatcher->result();

        aiThinking = false;
        boardwidget->setEnabled(true);

        bool applied = false;
        QString message;

        if (!aiEnabled) {
            updatePage();
            return;
        }

        if (game.getCurrentPlayer() != aiColor) {
            updatePage();
            return;
        }

        if (aiMove.isPass || aiMove.x < 0 || aiMove.y < 0) {
            applied = game.playPass();
            message = "AI 选择停一手";
        } else {
            applied = game.playMove(aiMove.x, aiMove.y);
            message = QString("AI 落子：%1").arg(moveToString(aiMove.x, aiMove.y));
        }

        if (!applied) {
            game.playPass();
            message = "AI 返回非法落点，已自动停一手";
        }

        boardwidget->loadGame(game);
        syncReplayCacheFromGame();
        rebuildWinRateCurve(replayIndex);
        updatePage();

        if (isTwoPasses()) {
            currentTurnLabel->setText("当前状态：对局结束");
            message = "双方连续停一手，对局结束";
        }

        statusLabel->setText(message);
    });

    scriptPath = "E:/vscode-code/GoEngine/python-ai/infer.py";
    modelPath = "E:/vscode-code/GoEngine/python-ai/checkpoints/go_model_best.pth";
}

GamePage::~GamePage() = default;

void GamePage::setupUI(){
    // 给当前页面命名，方便样式表只作用在这个页面上
    setObjectName("gamePage");

    // 整个页面的基础样式
    setStyleSheet(R"(
        QWidget#gamePage {
            background-color: #efeae2;
            color: #2c2c2c;
        }

        QFrame#boardPanel, QFrame#rightPanel {
            background: #f8f5ef;
            border: 1px solid #d6cec2;
            border-radius: 12px;
        }

        QGroupBox {
            font-size: 14px;
            font-weight: 600;
            color: #4b4035;
            border: 1px solid #d8d0c4;
            border-radius: 10px;
            margin-top: 12px;
            padding-top: 10px;
            background: #fcfbf8;
        }

        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 6px;
        }

        QPushButton {
            background: #ffffff;
            border: 1px solid #cfc6ba;
            border-radius: 8px;
            padding: 8px 12px;
        }

        QPushButton:hover {
            background: #f3eee6;
        }

        QPushButton:pressed {
            background: #e8dfd3;
        }

        QPushButton:disabled {
            background: #f3f0ea;
            color: #999186;
        }

        QListWidget {
            background: #ffffff;
            border: 1px solid #e0d8cc;
            border-radius: 8px;
            padding: 4px;
        }

        QListWidget::item {
            padding: 7px 6px;
            border-radius: 6px;
        }

        QListWidget::item:hover {
            background: #f5efe6;
        }

        QListWidget::item:selected {
            background: #8a6a43;
            color: #fffaf2;
        }
    )");

    // 第一部分的界面
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20,15,20,15) ;
    mainLayout->setSpacing(15) ;

    auto *topBarLayout = new QGridLayout() ;
    topBarLayout->setColumnStretch(0, 1);
    topBarLayout->setColumnStretch(1, 1);
    topBarLayout->setColumnStretch(2, 1);

    backButton = new QPushButton("返回首页" , this) ;
    backButton->setMinimumHeight(36);
    
    auto *titleLabel = new QLabel("围棋对局", this);
    QFont titleFont ;
    titleFont.setPointSize(16) ;
    titleFont.setBold(true) ;
    titleLabel->setFont(titleFont) ;

    statusLabel = new QLabel("准备开始", this);
    statusLabel->setMinimumWidth(360);
    statusLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    topBarLayout->addWidget(backButton, 0, 0, Qt::AlignLeft) ;
    topBarLayout->addWidget(titleLabel, 0, 1, Qt::AlignCenter) ;
    topBarLayout->addWidget(statusLabel, 0, 2, Qt::AlignRight) ;


    // 第二部分主体区域，左边棋盘，右边构建信息
    auto *contentLayout = new QHBoxLayout() ;
    contentLayout->setSpacing(15) ;

    // 棋盘部分
    auto *boardPanel = new QFrame(this) ;
    boardPanel->setObjectName("boardPanel");
    boardPanel->setMinimumSize(700,700) ;

    auto *boardLayout = new QVBoxLayout(boardPanel) ;
    boardLayout->setContentsMargins(12, 12, 12, 12);
    boardLayout->setSpacing(8);

    auto *boardTitle = new QLabel("棋盘" ,boardPanel);
    boardTitle->setAlignment(Qt::AlignCenter) ;

    boardwidget = new BoardWidget(boardPanel) ;

    boardLayout->addWidget(boardTitle) ;
    boardLayout->addWidget(boardwidget, 1);

    // 右边信息区域
    auto *rightPanel = new QFrame(this) ;
    rightPanel->setObjectName("rightPanel");
    rightPanel->setMinimumWidth(420) ;

    auto *rightLayout = new QVBoxLayout(rightPanel) ;
    rightLayout->setContentsMargins(12, 12, 12, 12);
    rightLayout->setSpacing(15) ;

    // 对局信息部分
    auto *gameInfoBox = new QGroupBox("对局信息", rightPanel) ;
    auto *gameInfoLayout = new QVBoxLayout(gameInfoBox) ;

    gameInfoLayout->addWidget(new QLabel("模式：人机对局 / 本地双人", gameInfoBox));
    gameInfoLayout->addWidget(new QLabel("说明：黑白双方轮流落子，支持 SGF 保存与复盘", gameInfoBox));
    currentTurnLabel = new QLabel("当前轮到：黑方", gameInfoBox);
    gameInfoLayout->addWidget(currentTurnLabel);
    
    // AI分析区
    auto *analysisBox = new QGroupBox("AI 分析" , rightPanel) ;
    auto *analysisLayout = new QVBoxLayout(analysisBox) ;
    analysisLayout->setSpacing(6);

    winRateLabel = new QLabel("胜率：黑 50.0% / 白 50.0%", analysisBox);
    scoreLabel = new QLabel("形势：均势", analysisBox);
    winRateChart = new WinRateChartWidget(analysisBox);

    analysisLayout->addWidget(winRateLabel);
    analysisLayout->addWidget(scoreLabel);
    analysisLayout->addWidget(winRateChart);

    // 手术记录区
    auto *moveListBoxGroup = new QGroupBox("手数记录", rightPanel);
    auto *moveListLayout = new QVBoxLayout(moveListBoxGroup);

    moveListWidget = new QListWidget(moveListBoxGroup);
    moveListLayout->addWidget(moveListWidget);

    // 操作区
    auto *controlBox = new QGroupBox("操作", rightPanel) ;
    auto *controlLayout = new QVBoxLayout(controlBox) ;
    controlLayout->setSpacing(8);

    passButton = new QPushButton("停一手", controlBox) ;
    undoButton = new QPushButton("悔棋", controlBox) ;
    resignButton = new QPushButton("认输", controlBox) ;
    restartButton = new QPushButton("重新开始", controlBox) ;
    openSGFButton = new QPushButton("打开棋谱", controlBox) ;
    saveSGFButton = new QPushButton("保存棋谱", controlBox) ;
    reviewButton = new QPushButton("对局研究", controlBox);
    stepForward = new QPushButton("上一手", controlBox) ;
    stepBackward = new QPushButton("下一手", controlBox) ;

    QList<QPushButton*> buttons = {
        passButton, undoButton, resignButton, restartButton,
        openSGFButton, saveSGFButton, reviewButton, stepForward, stepBackward
    };
    for (QPushButton* button : buttons) {
        button->setMinimumHeight(38);
    }

    controlLayout->addWidget(passButton) ;
    controlLayout->addWidget(undoButton) ;
    controlLayout->addWidget(resignButton) ;
    controlLayout->addWidget(restartButton) ;
    controlLayout->addWidget(openSGFButton) ;
    controlLayout->addWidget(saveSGFButton) ;
    controlLayout->addWidget(reviewButton);
    controlLayout->addWidget(stepForward) ;
    controlLayout->addWidget(stepBackward) ;

    // 右侧拼装
    rightLayout->addWidget(gameInfoBox) ; 
    rightLayout->addWidget(analysisBox) ;
    rightLayout->addWidget(moveListBoxGroup);
    rightLayout->addWidget(controlBox) ;

    // 左右拼装
    contentLayout->addWidget(boardPanel, 5);
    contentLayout->addWidget(rightPanel, 2);

    // 总体拼装
    mainLayout->addLayout(topBarLayout);
    mainLayout->addLayout(contentLayout, 1);
    
}

void GamePage::resetInfoPanel(){
    moveListWidget->clear();
    currentTurnLabel->setText("当前轮到：黑方");
    if (aiEnabled) {
        if (aiColor == Stone::WHITE) {
            statusLabel->setText("模式：人机对局 | 黑方：玩家 | 白方：AI");
        } else {
            statusLabel->setText("模式：人机对局 | 黑方：AI | 白方：玩家");
        }
    } else {
        statusLabel->setText("模式：双人对局 | 黑方：玩家 | 白方：玩家");
    }
    winRates.clear();
    winRates.push_back(50.0);
    if (winRateChart) {
        winRateChart->setRates(winRates);
    }

    winRateLabel->setText("胜率：黑 50.0% / 白 50.0%");
    scoreLabel->setText("形势：均势");
}

void GamePage::setupConnections(){
    connect(backButton, &QPushButton::clicked, this, [=]() {
        emit backToHomeRequested();
    });

    connect(passButton, &QPushButton::clicked, boardwidget, &BoardWidget::passTurn);
    connect(undoButton, &QPushButton::clicked, boardwidget, &BoardWidget::undoLastMove);
    connect(restartButton, &QPushButton::clicked,  this, &GamePage::startNewGame);
    connect(resignButton, &QPushButton::clicked, boardwidget, &BoardWidget::resignCurrentPlayer);
    connect(openSGFButton, &QPushButton::clicked, this, &GamePage::openSGFByDialog);
    connect(saveSGFButton, &QPushButton::clicked, this, &GamePage::saveSGFByDialog);
    connect(reviewButton, &QPushButton::clicked, this, &GamePage::openReviewDialog);
    connect(stepForward, &QPushButton::clicked, this ,GamePage::goForward) ;
    connect(stepBackward, &QPushButton::clicked, this , GamePage::goBackward) ;

    connect(moveListWidget, &QListWidget::itemClicked, this, [this](QListWidgetItem* item) {
        if (!item || aiThinking || currentMoves.empty()) {
            return;
        }
        goToStep(moveListWidget->row(item) + 1);
    });

    connect(boardwidget, &BoardWidget::movePlayed, this, [this](int x, int y, Stone color) {
        game = boardwidget->getGame();
        syncReplayCacheFromGame();
        rebuildWinRateCurve(replayIndex);
        updatePage();
        statusLabel->setText(QString("最近一步：%1 %2").arg(stoneToString(color), moveToString(x, y)));

        tryAIMove();
    });

    connect(boardwidget, &BoardWidget::passPlayed, this, [this](Stone color) {
        game = boardwidget->getGame();
        syncReplayCacheFromGame();
        rebuildWinRateCurve(replayIndex);
        updatePage();
        statusLabel->setText(QString("最近一步：%1 停一手").arg(stoneToString(color)));

        if (!isTwoPasses()) {
            tryAIMove();
        }
    });

    connect(boardwidget, &BoardWidget::moveUndone, this, [this]() {
        game = boardwidget->getGame();
        aiThinking = false;
        boardwidget->setEnabled(true);
        syncReplayCacheFromGame();
        rebuildWinRateCurve(replayIndex);
        updatePage();
        statusLabel->setText("已悔棋一手");
    });

    connect(boardwidget, &BoardWidget::turnChanged, this, [this](Stone nextPlayer) {
        currentTurnLabel->setText(QString("当前轮到：%1方").arg(stoneToString(nextPlayer)));
    });

    connect(boardwidget, &BoardWidget::illegalAction, this, [this](const QString &message) {
        statusLabel->setText(message);
    });

    connect(boardwidget, &BoardWidget::gameReset, this, [this]() {
        currentMoves.clear();
        replayIndex = 0;
        aiThinking = false;
        boardwidget->setEnabled(true);
        resetInfoPanel();
    });

    connect(boardwidget, &BoardWidget::gameOver, this, [this](const QString &message) {
        currentTurnLabel->setText("当前轮到：对局结束");
        statusLabel->setText(message);
    });
}

QString GamePage::moveToString(int x, int y) const{
    QString columns = "ABCDEFGHJKLMNOPQRST"; 

    if (x < 0 || x >= columns.size()) {
        return QString("(%1,%2)").arg(x).arg(y);
    }

    return QString("%1%2").arg(columns[x]).arg(19 - y);
}

QString GamePage::stoneToString(Stone color) const{
    if (color == Stone::BLACK) {
        return "黑";
    }
    if (color == Stone::WHITE) {
        return "白";
    }
    return "空";
}

void GamePage::loadSGFFile(const QString& path){
    SGFTree* tree = SGFParser::parse(path.toLocal8Bit().toStdString()) ;

    if (!tree || !tree->root) {
        QMessageBox::warning(this, "错误", "棋谱解析失败");
        delete tree;
        return;
    }

    currentMoves = extractMainLine(tree->root) ;
    delete tree ;

    if (!game.loadFromMoves(currentMoves)){
        QMessageBox::warning(this, "错误", "棋谱恢复失败");
        return;
    }

    replayIndex = static_cast<int>(currentMoves.size());
    boardwidget->loadGame(game);
    rebuildWinRateCurve(replayIndex);
    updatePage();
    statusLabel->setText("棋谱已载入");
}

void GamePage::updatePage(){
    boardwidget->loadGame(game);
    refreshMoveList();

    if (game.getCurrentPlayer() == Stone::BLACK) {
        currentTurnLabel->setText("当前轮到：黑方");
    } else {
        currentTurnLabel->setText("当前轮到：白方");
    }

    updateAnalysisLabels();
}

void GamePage::refreshMoveList()
{
    moveListWidget->clear();

    std::vector<Move> movesToShow = currentMoves;
    if (movesToShow.empty() && !game.getHistory().empty()) {
        movesToShow = convertHistoryToMoves(game.getHistory());
    }

    for (int i = 0; i < (int)movesToShow.size(); ++i) {
        const Move& move = movesToShow[i];
        QString colorText = stoneToString(move.stone);
        if (move.stone == Stone::EMPTY) {
            colorText = (i % 2 == 0) ? "黑" : "白";
        }

        QString text;
        if (move.isPass) {
            text = QString("%1. %2 停一手").arg(i + 1).arg(colorText);
        } else {
            text = QString("%1. %2 %3").arg(i + 1).arg(colorText, moveToString(move.x, move.y));
        }

        auto* item = new QListWidgetItem(text, moveListWidget);
        if (i >= replayIndex) {
            item->setForeground(QColor(145, 137, 126));
        }
    }

    if (replayIndex > 0 && replayIndex <= moveListWidget->count()) {
        moveListWidget->setCurrentRow(replayIndex - 1);
    } else {
        moveListWidget->clearSelection();
    }
}

void GamePage::setAIMode(bool enabled, Stone color){
    aiEnabled = enabled;
    aiColor = color;
    boardwidget->setAIEnabled(enabled);
    boardwidget->setAIcolor(color);

    resetInfoPanel();
}

void GamePage::startNewGame(){
    game.reset();
    currentMoves.clear();
    replayIndex = 0;

    aiThinking = false;
    boardwidget->setEnabled(true);

    boardwidget->resetBoard();
    boardwidget->loadGame(game); 

    resetInfoPanel();
    syncReplayCacheFromGame();
    rebuildWinRateCurve(replayIndex);
    updatePage();
    statusLabel->setText("已重新开始");

    if (aiEnabled && aiColor == Stone::BLACK){
        QTimer::singleShot(200, this, [this]() {
            tryAIMove();
        });
    }
}

void GamePage::goToStep(int n){
    if (currentMoves.empty()) {
        currentMoves = convertHistoryToMoves(game.getHistory());
    }

    n = qBound(0, n, static_cast<int>(currentMoves.size()));

    Game replayGame;
    for (int i = 0; i < n; ++i) {
        const Move& move = currentMoves[i];
        bool ok = move.isPass ? replayGame.playPass() : replayGame.playMove(move.x, move.y);
        if (!ok) {
            break;
        }
    }

    game = replayGame;
    replayIndex = n;
    rebuildWinRateCurve(replayIndex);
    updatePage();
    statusLabel->setText(QString("已跳转到第 %1 手").arg(replayIndex));
} 

void GamePage::goForward(){
    std::vector<RecordMove> moves = game.getHistory() ;
    int n = moves.size() ;
    goToStep(n-1) ;
}

void GamePage::goBackward(){
    std::vector<RecordMove> moves = game.getHistory() ;
    int n = moves.size() ;
    goToStep(n+1) ;
}

void GamePage::tryAIMove()
{
    if (!aiEnabled || game.getCurrentPlayer() != aiColor || aiThinking) {
        return;
    }

    aiThinking = true;
    boardwidget->setEnabled(false);
    statusLabel->setText("AI 思考中...");

    Game gameCopy = game;
    const QString pythonExe = "D:/conda/envs/goengine-ai/python.exe";
    const QString localScriptPath = scriptPath;
    const QString localModelPath = modelPath;

    auto future = QtConcurrent::run(
        [gameCopy, pythonExe, localScriptPath, localModelPath]() mutable -> Move {
            PythonEvaluator evaluator(pythonExe, localScriptPath, localModelPath);
            PythonEvaluator* evaluatorPtr = nullptr;

            if (evaluator.start()) {
                evaluatorPtr = &evaluator;
            }

            MCTS search(evaluatorPtr ? 80 : 60, evaluatorPtr);
            Move bestMove = search.getbestMove(gameCopy);

            evaluator.stop();
            return bestMove;
        }
    );

    aiWatcher->setFuture(future);
}

void GamePage::openSGFByDialog()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "打开 SGF 棋谱",
        QDir::currentPath(),
        "SGF Files (*.sgf);;All Files (*)"
    );

    if (!filename.isEmpty()) {
        setAIMode(false, Stone::WHITE);
        loadSGFFile(filename);
    }
}

void GamePage::saveSGFByDialog()
{
    if (game.getHistory().empty()) {
        QMessageBox::information(this, "提示", "当前没有可以保存的棋谱");
        return;
    }

    const QString saveDir = QDir::currentPath() + "/data/play_sgf";
    QDir().mkpath(saveDir);

    const QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    const QString defaultPath = saveDir + "/game_" + timestamp + ".sgf";

    QString filename = QFileDialog::getSaveFileName(
        this,
        "保存 SGF 文件",
        defaultPath,
        "SGF Files (*.sgf)"
    );

    if (filename.isEmpty()) {
        return;
    }

    if (!filename.endsWith(".sgf", Qt::CaseInsensitive)) {
        filename += ".sgf";
    }

    const std::vector<Move> movesToSave = convertHistoryToMoves(game.getHistory());
    const bool success = SGFWriter::saveMainLine(filename.toLocal8Bit().toStdString(), movesToSave);

    if (success) {
        QMessageBox::information(this, "成功", "棋谱保存成功！");
    } else {
        QMessageBox::warning(this, "失败", "棋谱保存失败");
    }
}

void GamePage::openReviewDialog(){
    ReviewDialog dialog(game, this);
    dialog.exec();
}

void GamePage::syncReplayCacheFromGame(){
    currentMoves = convertHistoryToMoves(game.getHistory());
    replayIndex = (int)currentMoves.size();
}

bool GamePage::isTwoPasses() const
{
    const auto& history = game.getHistory();
    return history.size() >= 2 && history[history.size() - 1].isPass && history[history.size() - 2].isPass;
}

void GamePage::rebuildWinRateCurve(int uptoStep){
    winRates.clear();
    winRates.push_back(50.0);

    Game tmp;
    const int limit = qBound(0, uptoStep, (int)currentMoves.size());

    for (int i = 0; i < limit; ++i) {
        const Move& move = currentMoves[i];
        const bool ok = move.isPass ? tmp.playPass() : tmp.playMove(move.x, move.y);
        if (!ok) {
            break;
        }

        winRates.push_back(estimateBlackWinRate(tmp));
    }

    if (winRateChart) {
        winRateChart->setRates(winRates);
    }
    updateAnalysisLabels();
}

double GamePage::estimateBlackLead(const Game& state) const{
    const Board& board = state.getBoard();

    double blackScore = 0.0;
    double whiteScore = 6.5; 

    const int dx[4] = {1, -1, 0, 0};
    const int dy[4] = {0, 0, 1, -1};

    for (int x = 0; x < Board::SIZE; ++x) {
        for (int y = 0; y < Board::SIZE; ++y) {
            Stone s = board.get(x, y);
            if (s == Stone::BLACK) {
                blackScore += 1.0;
                continue;
            }
            if (s == Stone::WHITE) {
                whiteScore += 1.0;
                continue;
            }

            bool nearBlack = false;
            bool nearWhite = false;
            for (int k = 0; k < 4; ++k) {
                const int nx = x + dx[k];
                const int ny = y + dy[k];
                if (nx < 0 || nx >= Board::SIZE || ny < 0 || ny >= Board::SIZE) {
                    continue;
                }

                Stone ns = board.get(nx, ny);
                if (ns == Stone::BLACK) {
                    nearBlack = true;
                } else if (ns == Stone::WHITE) {
                    nearWhite = true;
                }
            }

            if (nearBlack && !nearWhite) {
                blackScore += 0.35;
            } else if (nearWhite && !nearBlack) {
                whiteScore += 0.35;
            }
        }
    }

    return blackScore - whiteScore;
}

double GamePage::estimateBlackWinRate(const Game& state) const{
    const double lead = estimateBlackLead(state);
    const double rate = 100.0 / (1.0 + std::exp(-lead / 10.0));
    return qBound(3.0, rate, 97.0);
}

void GamePage::updateAnalysisLabels(){
    double blackRate = 50.0;
    if (!winRates.empty()) {
        blackRate = winRates.back();
    } else {
        blackRate = estimateBlackWinRate(game);
    }

    const double whiteRate = 100.0 - blackRate;
    winRateLabel->setText(QString("胜率：黑 %1% / 白 %2%")
                          .arg(blackRate, 0, 'f', 1)
                          .arg(whiteRate, 0, 'f', 1));

    const double lead = estimateBlackLead(game);
    if (std::abs(lead) < 0.6) {
        scoreLabel->setText("形势：均势");
    } else if (lead > 0) {
        scoreLabel->setText(QString("形势：黑领先约 %1 目").arg(lead, 0, 'f', 1));
    } else {
        scoreLabel->setText(QString("形势：白领先约 %1 目").arg(-lead, 0, 'f', 1));
    }
}
