#include "GamePage.h"
#include "BoardWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QFont>
#include <QFrame>
#include <QGroupBox>


GamePage::GamePage(QWidget *parent)
    : QWidget(parent),
      backButton(nullptr),
      boardwidget(nullptr)
{
    setupUI();
    setupConnections();
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

    auto *statusLabel = new QLabel("黑方：玩家   白方：AI   模式：人机对局" , this) ;

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

    winRateLabel = new QLabel("胜率：黑 50.0%", analysisBox);
    scoreLabel = new QLabel("目差：黑领先 0.0 目", analysisBox);

    auto *chartPlaceholder = new QFrame(analysisBox) ;
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
    moveListWidget->addItem("1. 黑 D4");
    moveListWidget->addItem("2. 白 Q16");

    moveListLayout->addWidget(moveListWidget);

    // 操作区
    auto *controlBox = new QGroupBox("操作", rightPanel) ;
    auto *controlLayout = new QVBoxLayout(controlBox) ;

    auto *passButton = new QPushButton("停一手", controlBox) ;
    auto *undoButton = new QPushButton("悔棋", controlBox) ;
    auto *resignButton = new QPushButton("认输", controlBox) ;
    auto *restartButton = new QPushButton("重新开始", controlBox) ;

    passButton->setMinimumHeight(40) ;
    undoButton->setMinimumHeight(40) ;
    resignButton->setMinimumHeight(40) ;
    restartButton->setMinimumHeight(40) ;

    controlLayout->addWidget(passButton) ;
    controlLayout->addWidget(undoButton) ;
    controlLayout->addWidget(resignButton) ;
    controlLayout->addWidget(restartButton) ;

    connect(restartButton, &QPushButton::clicked, this, [=]() {
        boardwidget->resetBoard();
        moveListWidget->clear();
        currentTurnLabel->setText("当前轮到：黑方");
        statusLabel->setText("黑方：玩家   白方：AI   模式：人机对局");
        winRateLabel->setText("胜率：黑 50.0%");
        scoreLabel->setText("目差：黑领先 0.0 目");
    });

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

void GamePage::setupConnections()
{
    connect(backButton, &QPushButton::clicked, this, [=]() {
        emit backToHomeRequested();
    });

    connect(boardwidget, &BoardWidget::movePlayed, this, [=](int x, int y, Stone color) {
        QString colorText;
        QString nextText;

        if (color == Stone::BLACK) {
            colorText = "黑";
            nextText = "白";
        } else {
            colorText = "白";
            nextText = "黑";
        }

        int moveNumber = moveListWidget->count() + 1;
        QString moveText = QString::number(moveNumber) + ". " + colorText + " " + moveToString(x, y);

        moveListWidget->addItem(moveText);
        currentTurnLabel->setText("当前轮到：" + nextText + "方");
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

