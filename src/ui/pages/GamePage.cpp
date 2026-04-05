#include "GamePage.h"
#include "BoardWidget.h"
#include "Board.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFont>
#include <QFrame>
#include <QGroupBox>


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
      moveListWidget(nullptr)
{
    setupUI();
    setupConnections();
    resetInfoPanel();
}

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

    auto *titleLabel = new QLabel("人机对局" , this) ;
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

    boardwidget = new BoardWidget(boardPanel);

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

    passButton->setMinimumHeight(40) ;
    undoButton->setMinimumHeight(40) ;
    resignButton->setMinimumHeight(40) ;
    restartButton->setMinimumHeight(40) ;

    controlLayout->addWidget(passButton) ;
    controlLayout->addWidget(undoButton) ;
    controlLayout->addWidget(resignButton) ;
    controlLayout->addWidget(restartButton) ;

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
    statusLabel->setText("模式：人机对局 | 黑方：玩家 | 白方：AI");
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
    connect(restartButton, &QPushButton::clicked, boardwidget, &BoardWidget::resetBoard);
    connect(resignButton, &QPushButton::clicked, boardwidget, &BoardWidget::resignCurrentPlayer);

    connect(boardwidget, &BoardWidget::movePlayed, this, [this](int x, int y, Stone color) {
        int moveNumber = moveListWidget->count() + 1;
        QString text = QString("%1. %2 %3")
            .arg(moveNumber)
            .arg(stoneToString(color))
            .arg(moveToString(x, y));
        moveListWidget->addItem(text);
        statusLabel->setText(QString("最近一步：%1").arg(text));
    });

    connect(boardwidget, &BoardWidget::passPlayed, this, [this](Stone color) {
        int moveNumber = moveListWidget->count() + 1;
        QString text = QString("%1. %2 停一手")
            .arg(moveNumber)
            .arg(stoneToString(color));
        moveListWidget->addItem(text);
        statusLabel->setText(QString("最近一步：%1").arg(text));
    });

    connect(boardwidget, &BoardWidget::moveUndone, this, [this]() {
        if (moveListWidget->count() > 0) {
            delete moveListWidget->takeItem(moveListWidget->count() - 1);
        }
        statusLabel->setText("已悔棋一手");
    });

    connect(boardwidget, &BoardWidget::turnChanged, this, [this](Stone nextPlayer) {
        currentTurnLabel->setText(QString("当前轮到：%1方").arg(stoneToString(nextPlayer)));
    });

    connect(boardwidget, &BoardWidget::illegalAction, this, [this](const QString &message) {
        statusLabel->setText(message);
    });

    connect(boardwidget, &BoardWidget::gameReset, this, [this]() {
        resetInfoPanel();
    });

    connect(boardwidget, &BoardWidget::gameOver, this, [this](const QString &message) {
        currentTurnLabel->setText("当前轮到：对局结束");
        statusLabel->setText(message);
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