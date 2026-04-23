#include "GamePage.h"
#include "BoardWidget.h"
#include "core/Board.h"
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
#include <QDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QFileDialog>
#include <QtConcurrent>

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
      stepBackward(nullptr)
{
    setupUI();
    setupConnections();
    resetInfoPanel();

    aiWatcher = new QFutureWatcher<Move>(this);

    connect(aiWatcher, &QFutureWatcher<Move>::finished, this, [this]() {
        aiThinking = false;
        boardwidget->setEnabled(true);

        Move aiMove = aiWatcher->result();

        if (aiMove.isPass) {
            game.playPass();
        } else {
            game.playMove(aiMove.x, aiMove.y);
        }

        updatePage();
        statusLabel->setText("AI 已完成落子");
    });

    QString appDir = QCoreApplication::applicationDirPath();
    QDir dir(appDir);

    dir.cdUp();

    QString scriptPath = dir.filePath("python-ai/infer.py");
    QString modelPath  = dir.filePath("python-ai/checkpoints/go_model_best.pth");

    evaluator = std::make_unique<PythonEvaluator>(
        "python",
        scriptPath,
        modelPath
    );

    mcts = std::make_unique<MCTS>(10000, evaluator.get());
}

GamePage::~GamePage() = default;
void GamePage::setupUI()
{
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
    
    QString str = aiEnabled ? "人机对局" : "双人对局" ;
    auto *titleLabel = new QLabel(str , this) ;
    QFont titleFont ;
    titleFont.setPointSize(16) ;
    titleFont.setBold(true) ;
    titleLabel->setFont(titleFont) ;

    statusLabel = new QLabel("黑方：玩家   白方：AI   模式：人机对局" , this) ;

    topBarLayout->addWidget(backButton, 0, 0, Qt::AlignLeft) ;
    topBarLayout->addWidget(titleLabel, 0, 0, Qt::AlignCenter) ;
    topBarLayout->addWidget(statusLabel, 0, 0 , Qt::AlignRight) ;


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

    auto *boardTitle = new QLabel("棋盘区域" ,boardPanel);
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

    gameInfoLayout->addWidget(new QLabel("当前模式：人机对局", gameInfoBox)) ;
    gameInfoLayout->addWidget(new QLabel("黑方：玩家", gameInfoBox)) ;
    gameInfoLayout->addWidget(new QLabel("白方：AI", gameInfoBox)) ;

    currentTurnLabel = new QLabel("当前轮到：黑方", gameInfoBox) ;
    gameInfoLayout->addWidget(currentTurnLabel) ;
    gameInfoLayout->addWidget(new QLabel("用时：60分钟 / 3次30秒", gameInfoBox)) ;
    
    // AI分析区
    auto *analysisBox = new QGroupBox("AI 分析" , rightPanel) ;
    auto *analysisLayout = new QVBoxLayout(analysisBox) ;
    analysisLayout->setSpacing(6);

    winRateLabel = new QLabel("胜率：黑 50.0%", analysisBox);
    scoreLabel = new QLabel("目差：黑领先 0.0 目", analysisBox);
    analysisLayout->addWidget(winRateLabel);
    analysisLayout->addWidget(scoreLabel);

    auto *chartPlaceholder = new QFrame(analysisBox) ;
    chartPlaceholder->setObjectName("chartPlaceholder");  
    chartPlaceholder->setMinimumHeight(180) ;

    auto *chartLayout = new QVBoxLayout(chartPlaceholder) ;
    auto *chartText = new QLabel("胜率折线图区", chartPlaceholder) ;
    chartText->setAlignment(Qt::AlignCenter) ;

    chartLayout->addStretch() ;
    chartLayout->addWidget(chartText) ;
    chartLayout->addStretch() ;

    analysisLayout->addWidget(chartPlaceholder) ;

    // 手术记录区
    auto *moveListBoxGroup = new QGroupBox("手数记录", rightPanel);
    auto *moveListLayout = new QVBoxLayout(moveListBoxGroup);

    moveListWidget = new QListWidget(moveListBoxGroup);
    moveListLayout->addWidget(moveListWidget);

    // 操作区
    auto *controlBox = new QGroupBox("操作", rightPanel) ;
    auto *controlLayout = new QVBoxLayout(controlBox) ;

    passButton = new QPushButton("停一手", controlBox) ;
    undoButton = new QPushButton("悔棋", controlBox) ;
    resignButton = new QPushButton("认输", controlBox) ;
    restartButton = new QPushButton("重新开始", controlBox) ;
    openSGFButton = new QPushButton("打开棋谱", controlBox) ;
    saveSGFButton = new QPushButton("保存棋谱", controlBox) ;
    stepForward = new QPushButton("上一手", controlBox) ;
    stepBackward = new QPushButton("下一手", controlBox) ;

    passButton->setMinimumHeight(40) ;
    undoButton->setMinimumHeight(40) ;
    resignButton->setMinimumHeight(40) ;
    restartButton->setMinimumHeight(40) ;
    openSGFButton->setMinimumHeight(40);
    saveSGFButton->setMinimumHeight(40) ;
    stepForward->setMinimumHeight(40) ;
    stepBackward->setMinimumHeight(40) ; 

    controlLayout->addWidget(passButton) ;
    controlLayout->addWidget(undoButton) ;
    controlLayout->addWidget(resignButton) ;
    controlLayout->addWidget(restartButton) ;
    controlLayout->addWidget(openSGFButton) ;
    controlLayout->addWidget(saveSGFButton) ;
    controlLayout->addWidget(stepForward) ;
    controlLayout->addWidget(stepBackward) ;

    // 右侧拼装
    rightLayout->addWidget(gameInfoBox) ; 
    rightLayout->addWidget(analysisBox) ;
    rightLayout->addWidget(moveListBoxGroup);
    rightLayout->addWidget(controlBox) ;

    // 左右拼装
    contentLayout->addWidget(boardPanel, 5);
    contentLayout->addWidget(rightPanel, 3);

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
    winRateLabel->setText("胜率：黑 50.0%");
    scoreLabel->setText("目差：黑领先 0.0 目");
}

void GamePage::setupConnections()
{
    connect(backButton, &QPushButton::clicked, this, [=]() {
        emit backToHomeRequested();
    });

    connect(passButton, &QPushButton::clicked, boardwidget, &BoardWidget::passTurn);
    connect(undoButton, &QPushButton::clicked, boardwidget, &BoardWidget::undoLastMove);
    connect(restartButton, &QPushButton::clicked,  this, &GamePage::startNewGame);
    connect(resignButton, &QPushButton::clicked, boardwidget, &BoardWidget::resignCurrentPlayer);
    connect(stepForward, &QPushButton::clicked, this ,GamePage::goForward) ;
    connect(stepBackward, &QPushButton::clicked, this , GamePage::goBackward) ;

    connect(boardwidget, &BoardWidget::movePlayed, this, [this](int x, int y, Stone color) {
        game = boardwidget->getGame();
        
        int moveNumber = moveListWidget->count() + 1;
        QString text = QString("%1. %2 %3")
            .arg(moveNumber)
            .arg(stoneToString(color))
            .arg(moveToString(x, y));
        moveListWidget->addItem(text);
        currentTurnLabel->setText(QString("当前轮到：%1方").arg(stoneToString(game.getCurrentPlayer())));
        statusLabel->setText(QString("最近一步：%1").arg(text));

        tryAIMove() ;
    });

    connect(boardwidget, &BoardWidget::passPlayed, this, [this](Stone color) {
        game = boardwidget->getGame();

        int moveNumber = moveListWidget->count() + 1;
        QString text = QString("%1. %2 停一手")
            .arg(moveNumber)
            .arg(stoneToString(color));
        moveListWidget->addItem(text);
        currentTurnLabel->setText(QString("当前轮到：%1方").arg(stoneToString(game.getCurrentPlayer())));
        statusLabel->setText(QString("最近一步：%1").arg(text));

        tryAIMove() ;
    });

    connect(boardwidget, &BoardWidget::moveUndone, this, [this]() {
        game = boardwidget->getGame();
        aiThinking = false;
        boardwidget->setEnabled(true);
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

    connect(saveSGFButton, &QPushButton::clicked , this, [this](){
        if (game.getHistory().empty()){
            QMessageBox::information(this, "提示" , "当前没有可以保存的棋谱") ;
            return  ;
        } 

        QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") ;

        QString defaultPath = QDir::currentPath() + "/data/play_sgf/game_" + timestamp + ".sgf";

        QString filename = QFileDialog::getOpenFileName(
            this ,
            "保存 SGF 文件" ,
            defaultPath ,
            "SGF Files (*.sgf)"
        );

        if (filename.isEmpty()){
            return ;
        }

        std::vector<Move> movesToSave = convertHistoryToMoves(game.getHistory()) ;
        bool success = SGFWriter::saveMainLine(filename.toLocal8Bit().toStdString(), movesToSave);

        if (success){
            QMessageBox::information(this, "成功", "棋谱保存成功！");
        }
        else{
            QMessageBox::information(this ,"失败" , "棋谱保存失败") ;
        }
    });
}

QString GamePage::moveToString(int x, int y) const
{
    QString columns = "ABCDEFGHJKLMNOPQRST"; 

    if (x < 0 || x >= columns.size()) {
        return "(" + QString::number(x) + "," + QString::number(y) + ")";
    }

    QString col = QString(columns[x]); 
    QString row = QString::number(19 - y); 

    return col + row; 
}

QString GamePage::stoneToString(Stone color) const
{
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

    updatePage() ;
}

void GamePage::updatePage(){
    boardwidget->loadGame(game) ;

    moveListWidget->clear() ;
    const auto& history = game.getHistory();

    for (int i = 0; i < history.size(); ++i) {
        const RecordMove& move = history[i];

        QString text;

        QString colorText = (move.color == Stone::BLACK) ? "黑" : "白";

        if (move.isPass) {
            text = QString("%1. %2 停一手")
                   .arg(i + 1)
                   .arg(colorText);
        } else {
            text = QString("%1. %2 (%3,%4)")
                   .arg(i + 1)
                   .arg(colorText)
                   .arg(move.x)
                   .arg(move.y);
        }

        moveListWidget->addItem(text);
    }

    QString turnText;

    if (game.getCurrentPlayer() == Stone::BLACK) {
        turnText = "当前轮到：黑方";
    } else {
        turnText = "当前轮到：白方";
    }

    statusLabel->setText(turnText);
}

void GamePage::setAIMode(bool enabled, Stone color){
    aiEnabled = enabled ;
    aiColor = color ;
    boardwidget->setAIEnabled(enabled) ;
    boardwidget->setAIcolor(color) ;

    if (aiEnabled) {
        if (aiColor == Stone::WHITE) {
            statusLabel->setText("模式：人机对局 | 黑方：玩家 | 白方：AI");
        } else {
            statusLabel->setText("模式：人机对局 | 黑方：AI | 白方：玩家");
        }
    } else {
        statusLabel->setText("模式：双人对局 | 黑方：玩家 | 白方：玩家");
    }
    
    winRateLabel->setText("胜率：黑 50.0%");
    scoreLabel->setText("目差：黑领先 0.0 目");
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
    statusLabel->setText("已重新开始");

}

void GamePage::goToStep(int n){
    if (n < 0){
        n = 0 ;
    }

    if (n > (int)currentMoves.size()){
        n = (int)currentMoves.size() ;
    }

    int time = 0 ;
    game.reset() ;

    while (time < n){
        if (currentMoves[time].isPass) {
            game.playPass();
        } else {
            int x = currentMoves[time].x;
            int y = currentMoves[time].y;
            game.playMove(x, y);
        }
        time++;
    }
    replayIndex = n ;

    updatePage() ;
    boardwidget->loadGame(game) ;
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

void GamePage::tryAIMove(){
    if (!aiEnabled || !mcts)
        return;

    if (game.getCurrentPlayer() != aiColor)
        return;

    if (aiThinking){
        return ; 
    }

    aiThinking = true ;

    boardwidget->setEnabled(false); 
    statusLabel->setText("AI 思考中");

    Game gameCopy = game;

    auto future = QtConcurrent::run([this, gameCopy]() mutable {
        return mcts->getbestMove(gameCopy);
    });

    aiWatcher->setFuture(future);
}

