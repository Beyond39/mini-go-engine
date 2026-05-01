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
#include <QListWidgetItem>
#include <QDir>
#include <QDirIterator>
#include <QFileDialog>
#include <QFileInfo>

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
        QPushButton#primaryButton {
            background: #6f5435;
            color: #fffaf2;
            border: 1px solid #5c452c;
            font-weight: 700;
        }
        QPushButton:hover {
            background: #f3eee6;
        }
        QPushButton#primaryButton:hover { 
            background: #7d6040; 
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

        QListWidget::item:selected {
            background: #8a6a43;
            color: #fffaf2;
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

    continueButton = new QPushButton("继续最近对局", heroCard) ;
    continueButton->setEnabled(false);
    continueButton->setToolTip("当前版本未保存最近对局状态，建议使用“打开棋谱”。");

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
    auto* sgfCard = new QFrame(this);
    sgfCard->setObjectName("listCard");

    auto* sgfLayout = new QVBoxLayout(sgfCard);
    sgfLayout->setContentsMargins(18, 16, 18, 16);
    sgfLayout->setSpacing(12);

    auto* sgfTitle = new QLabel("本地棋谱", sgfCard);
    sgfTitle->setObjectName("sectionTitle");

    auto* sgfHint = new QLabel("双击真实存在的 SGF 文件进入复盘", sgfCard);
    sgfHint->setObjectName("pageSubTitle");

    popularListWidget = new QListWidget(sgfCard);
    popularListWidget->setMinimumHeight(300);

    sgfLayout->addWidget(sgfTitle);
    sgfLayout->addWidget(sgfHint);
    sgfLayout->addWidget(popularListWidget, 1);

    auto* featureCard = new QFrame(this);
    featureCard->setObjectName("listCard");

    auto* featureLayout = new QVBoxLayout(featureCard);
    featureLayout->setContentsMargins(18, 16, 18, 16);
    featureLayout->setSpacing(12);

    auto* featureTitle = new QLabel("项目功能", featureCard);
    featureTitle->setObjectName("sectionTitle");

    auto* featureHint = new QLabel("不再展示虚假的历史棋谱，改为展示真实功能状态", featureCard);
    featureHint->setObjectName("pageSubTitle");

    archiveListWidget = new QListWidget(featureCard);
    archiveListWidget->addItem("已实现：落子、提子、禁自杀、简单打劫");
    archiveListWidget->addItem("已实现：SGF 打开 / 保存 / 主线复盘");
    archiveListWidget->addItem("新增：对局研究小窗口，可自由试下");
    archiveListWidget->addItem("新增：QPainter 胜率曲线展示");
    archiveListWidget->addItem("建议：时间/读秒本版不展示，避免半成品");

    featureLayout->addWidget(featureTitle);
    featureLayout->addWidget(featureHint);
    featureLayout->addWidget(archiveListWidget, 1);

    listsRow->addWidget(sgfCard, 1);
    listsRow->addWidget(featureCard, 1);

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

    connect(popularListWidget, &QListWidget::itemDoubleClicked, this , [this](QListWidgetItem *item){
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
    popularListWidget->clear();

    int count = 0;
    const int maxShow = 200;

    auto addFilesFrom = [&](const QString& rootPath, const QString& tag) {
        QDir root(rootPath);
        if (!root.exists()) {
            return;
        }

        QDirIterator it(root.absolutePath(), QStringList() << "*.sgf", QDir::Files, QDirIterator::Subdirectories);

        while (it.hasNext() && count < maxShow) {
            const QString path = it.next();
            const QFileInfo info(path);
            auto* item = new QListWidgetItem(QString("[%1] %2").arg(tag, info.fileName()), popularListWidget);
            item->setData(Qt::UserRole, path);
            item->setToolTip(path);
            ++count;
        }
    };

    addFilesFrom("data/play_sgf", "play");
    addFilesFrom("data/train_sgf", "train");

    if (count == 0) {
        auto* item = new QListWidgetItem("暂无本地 SGF：可点击上方“打开棋谱”手动选择", popularListWidget);
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
    }
}

QString HomePage::getSelectedMode() const{
    return modeComboBox->currentText() ;
}

QString HomePage::getSelectedColor() const{
    return colorComboBox->currentText();
}
