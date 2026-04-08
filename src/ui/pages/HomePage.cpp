#include "HomePage.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QComboBox>
#include <QFont>
#include <QMessageBox>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QDir>
#include <QFileDialog>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      startButton(nullptr),
      continueButton(nullptr),
      openSgfButton(nullptr),
      rulesButton(nullptr),
      popularListWidget(nullptr),
      archiveListWidget(nullptr),
      statusTagLabel(nullptr),
      modeComboBox(nullptr),
      colorComboBox(nullptr)
{
    setupUI();
    setupConnections();
    loadSGFList() ;
}

void HomePage::setupUI()
{
    setObjectName("homePage");

    setStyleSheet(R"(
        QWidget#homePage {
            background: #efeae2;
            color: #2c2a25;
        }
        QFrame#heroCard, QFrame#listCard {
            background: #fcfbf8;
            border: 1px solid #d8d0c4;
            border-radius: 16px;
        }
            QLabel#pageTitle {
            color: #1f1a16;
        }
            QLabel#pageSubTitle {
            color: #5f564d;
        }

        QLabel#sectionTitle {
            font-size: 16px;
            font-weight: 700;
            color: #3f362e;
        }

        QLabel#tagLabel {
            background: #ede3d4;
            color: #6a5743;
            border: 1px solid #d9cbb9;
            border-radius: 10px;
            padding: 4px 10px;
        }
        QPushButton {
            background: #ffffff;
            border: 1px solid #cfc6ba;
            border-radius: 8px;
            padding: 10px 14px;
        }
        QPushButton:hover {
            background: #f3eee6;
        }
        QPushButton:pressed {
            background: #e9dfd0;
        }
            QListWidget {
            background: #ffffff;
            border: 1px solid #e0d8cc;
            border-radius: 12px;
            padding: 8px;
            outline: none;
        }
        QListWidget::item {
            padding: 8px 6px;
            border-radius: 6px;
        }

        QListWidget::item:hover {
            background: #f5efe6;
        }

        QComboBox {
            background: #ffffff;
            border: 1px solid #d7cdbf;
            border-radius: 8px;
            padding: 6px 10px;
            min-height: 34px;
        }
    )");

    // 顶部配置区 
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    auto *heroCard = new QFrame(this);
    heroCard->setObjectName("heroCard");
    auto *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(24, 20, 24, 20);
    heroLayout->setSpacing(16);

    auto *topRow = new QHBoxLayout() ;
    topRow->setSpacing(12) ;

    auto *titleLayout = new QVBoxLayout() ;
    titleLayout->setSpacing(6) ;

    auto *titleLabel = new QLabel("围棋 AI 对弈平台", heroCard);
    titleLabel->setObjectName("pageTitle") ;
    QFont titleFont;
    titleFont.setPointSize(26);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *subTitleLabel = new QLabel("支持人机对局、棋谱复盘、后续可扩展 SGF 解析与 AI 分析", heroCard);
    subTitleLabel->setObjectName("pageSubTitle") ;
    QFont subFont;
    subFont.setPointSize(11);
    subTitleLabel->setFont(subFont);
    
    titleLayout->addWidget(titleLabel);
    titleLayout->addWidget(subTitleLabel);

    statusTagLabel = new QLabel("当前版本：基础平台版", heroCard);
    statusTagLabel->setObjectName("tagLabel");
    statusTagLabel->setAlignment(Qt::AlignCenter);

    topRow->addLayout(titleLayout, 1);
    topRow->addWidget(statusTagLabel, 0, Qt::AlignTop);

    // 中部处理区
    auto *quickCard = new QFrame(heroCard) ;
    quickCard->setObjectName("quickCard") ;

    auto *quickLayout = new QGridLayout(quickCard) ;
    quickLayout->setContentsMargins(18,16,18,16) ;
    quickLayout->setHorizontalSpacing(14) ;
    quickLayout->setVerticalSpacing(12) ;

    auto *modeLabel = new QLabel("对局模式" , quickCard) ;
    modeLabel->setObjectName("sectionTitle") ;
    modeComboBox = new QComboBox(quickCard) ;
    modeComboBox->addItem("人机对局") ;
    modeComboBox->addItem("本地双人对局") ;
    
    auto *colorLabel = new QLabel("执子选择" , quickCard) ;
    colorLabel->setObjectName("sectionTitle") ;
    colorComboBox = new QComboBox(quickCard) ;
    colorComboBox->addItem("我执黑") ;
    colorComboBox->addItem("我执白") ;

    quickLayout->addWidget(modeLabel, 0 , 0 ) ;
    quickLayout->addWidget(modeComboBox, 1 , 0) ;
    quickLayout->addWidget(colorLabel, 0 , 1) ;
    quickLayout->addWidget(colorComboBox, 1 ,1 ) ;

    // 底部按钮区
    auto *buttonRow = new QHBoxLayout() ;
    buttonRow->setSpacing(12) ;

    startButton = new QPushButton("开始游戏", heroCard) ;
    startButton->setObjectName("primaryButton") ; 

    sgfListWidget = new QListWidget(this) ;

    continueButton = new QPushButton("继续最近对局", heroCard) ;
    openSgfButton = new QPushButton("打开棋谱", heroCard) ;
    rulesButton = new QPushButton("规则说明", heroCard) ;

    buttonRow->addWidget(startButton) ;
    buttonRow->addWidget(continueButton) ;
    buttonRow->addWidget(openSgfButton) ;
    buttonRow->addWidget(rulesButton) ;
    buttonRow->addStretch() ;

    heroLayout->addLayout(topRow) ;
    heroLayout->addWidget(quickCard) ;
    heroLayout->addLayout(buttonRow) ;
    
    // 下方栏目区
    auto *listsRow = new QHBoxLayout() ;
    listsRow->setSpacing(18) ;
    
    // 热门棋谱 
    auto *popularCard = new QFrame(this) ;
    popularCard->setObjectName("listCard") ;

    auto *popularLayout = new QVBoxLayout(popularCard) ;
    popularLayout->setContentsMargins(18, 16, 18, 16) ;
    popularLayout->setSpacing(12) ;

    auto *popularTitle = new QLabel("最近流行棋谱", popularCard) ;
    popularTitle->setObjectName("sectionTitle");

    auto *popularHint = new QLabel("可作为复盘与后续训练样本的入口", popularCard);
    popularHint->setObjectName("pageSubTitle");

    popularListWidget = new QListWidget(popularCard) ;
    popularListWidget->addItem("LG杯：申真谞 vs 柯洁");
    popularListWidget->addItem("春兰杯：朴廷桓 vs 辜梓豪");
    popularListWidget->addItem("应氏杯：一力辽 vs 许家元");
    popularListWidget->addItem("名人战：芈昱廷 vs 杨鼎新");
    popularListWidget->setMinimumHeight(300) ;

    popularLayout->addWidget(popularTitle);
    popularLayout->addWidget(popularHint) ;
    popularLayout->addWidget(popularListWidget , 1);
    
    // 本地存档
    auto *archiveCard = new QFrame(this);
    archiveCard->setObjectName("listCard");

    auto *archiveLayout = new QVBoxLayout(archiveCard);
    archiveLayout->setContentsMargins(18, 16, 18, 16);
    archiveLayout->setSpacing(12) ;

    auto *archiveTitle = new QLabel("本地存档", archiveCard);
    archiveTitle->setObjectName("sectionTitle") ;

    auto *archiveHint = new QLabel("继续上次对局，或载入历史记录继续复盘", archiveCard);
    archiveHint->setObjectName("pageSubTitle");

    archiveListWidget = new QListWidget(archiveCard);
    archiveListWidget->addItem("2026-04-04 | 人机对局 | 第 12 手");
    archiveListWidget->addItem("2026-04-03 | 复盘模式 | 已完成");
    archiveListWidget->addItem("2026-04-02 | 训练样本导入记录");

    archiveLayout->addWidget(archiveTitle);
    archiveLayout->addWidget(archiveHint) ;
    archiveLayout->addWidget(archiveListWidget, 1);

    listsRow->addWidget(popularCard, 1);
    listsRow->addWidget(archiveCard, 1);

    mainLayout->addWidget(heroCard);
    mainLayout->addLayout(listsRow, 1);
}

void HomePage::setupConnections()
{
    connect(startButton, &QPushButton::clicked, this, [=]() {
        emit startAIGameRequested();
    });

    connect(continueButton, &QPushButton::clicked, this, [this]() {
        emit startAIGameRequested();
    });

    connect(rulesButton, &QPushButton::clicked, this, [this]() {
        QMessageBox::information(
            this,
            "规则说明",
            "围棋基本规则：\n"
            "1. 黑白双方轮流落子。\n"
            "2. 无气的棋子会被提走。\n"
            "3. 禁止自杀，支持打劫判断。\n"
            "4. 双方连续停一手可视作终局。\n"
            "5. 当前平台支持基础对局、悔棋、棋谱扩展。"
        );
    });

    connect(openSgfButton, &QPushButton::clicked, this, [this]() {
        QString filename = QFileDialog::getOpenFileName(
            this,
            "打开SGF棋谱",
            QDir::currentPath(), 
            "SGF Files (*.sgf);;All Files (*)"
        );

        if (!filename.isEmpty()) {
            emit sgfSelected(filename);
        }
    });

    connect(sgfListWidget, &QListWidget::itemDoubleClicked, this , [this](QListWidgetItem *item){
        if (!item){
            return ;
        }

        QString fullpath = item->data(Qt::UserRole).toString() ;
        if (!fullpath.isEmpty()){
            emit sgfSelected(fullpath) ;
        }
    });

    connect(modeComboBox, &QComboBox::currentTextChanged, this, [this](const QString &text) {
        bool enableColor = (text == "人机对局");
        colorComboBox->setEnabled(enableColor);
    });
}

void HomePage::loadSGFList(){
    QDir traindir("data/train_sgf") ;
    QDir playdir("data/play_sgf") ;

    if (traindir.exists()) {
        QStringList files = traindir.entryList(QStringList() << "*.sgf", QDir::Files);
        for (const QString &file : files) {
            QListWidgetItem* item = new QListWidgetItem("[train] " + file, sgfListWidget);
            item->setData(Qt::UserRole, traindir.absoluteFilePath(file)); 
        }
    }

    if (playdir.exists()) {
        QStringList files = playdir.entryList(QStringList() << "*.sgf", QDir::Files);
        for (const QString &file : files) {
            QListWidgetItem* item = new QListWidgetItem("[play] " + file, sgfListWidget);
            item->setData(Qt::UserRole, playdir.absoluteFilePath(file));
        }
    }
}
