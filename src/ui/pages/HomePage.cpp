#include "HomePage.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      startButton(nullptr),
      continueButton(nullptr),
      popularListWidget(nullptr),
      archiveListWidget(nullptr)
{
    setupUI();
    setupConnections();
}

void HomePage::setupUI()
{
    setObjectName("homePage");

    setStyleSheet(R"(
        QWidget#homePage {
            background: #efeae2;
            color: #2c2c2c;
        }
        QFrame#heroCard, QFrame#listCard {
            background: #fcfbf8;
            border: 1px solid #d8d0c4;
            border-radius: 14px;
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
        QListWidget {
            background: #ffffff;
            border: 1px solid #e0d8cc;
            border-radius: 10px;
            padding: 6px;
        }
    )");

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(28, 24, 28, 24);
    mainLayout->setSpacing(18);

    auto *heroCard = new QFrame(this);
    heroCard->setObjectName("heroCard");
    auto *heroLayout = new QVBoxLayout(heroCard);
    heroLayout->setContentsMargins(24, 20, 24, 20);
    heroLayout->setSpacing(12);

    auto *titleLabel = new QLabel("围棋 AI 对弈平台", heroCard);
    QFont titleFont;
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    auto *subTitleLabel = new QLabel("支持人机对局、棋谱复盘、后续可扩展 SGF 解析与 AI 分析", heroCard);

    auto *buttonRow = new QHBoxLayout();
    startButton = new QPushButton("开始人机对局", heroCard);
    continueButton = new QPushButton("继续最近对局", heroCard);
    buttonRow->addWidget(startButton);
    buttonRow->addWidget(continueButton);
    buttonRow->addStretch();

    heroLayout->addWidget(titleLabel);
    heroLayout->addWidget(subTitleLabel);
    heroLayout->addLayout(buttonRow);

    auto *listsRow = new QHBoxLayout();
    listsRow->setSpacing(18);

    auto *popularCard = new QFrame(this);
    popularCard->setObjectName("listCard");
    auto *popularLayout = new QVBoxLayout(popularCard);
    popularLayout->setContentsMargins(18, 16, 18, 16);
    auto *popularTitle = new QLabel("最近流行棋谱", popularCard);
    popularListWidget = new QListWidget(popularCard);
    popularListWidget->addItem("LG杯：申真谞 vs 柯洁");
    popularListWidget->addItem("春兰杯：朴廷桓 vs 辜梓豪");
    popularListWidget->addItem("应氏杯：一力辽 vs 许家元");
    popularListWidget->addItem("名人战：芈昱廷 vs 杨鼎新");
    popularLayout->addWidget(popularTitle);
    popularLayout->addWidget(popularListWidget);

    auto *archiveCard = new QFrame(this);
    archiveCard->setObjectName("listCard");
    auto *archiveLayout = new QVBoxLayout(archiveCard);
    archiveLayout->setContentsMargins(18, 16, 18, 16);
    auto *archiveTitle = new QLabel("本地存档", archiveCard);
    archiveListWidget = new QListWidget(archiveCard);
    archiveListWidget->addItem("2026-04-04 | 人机对局 | 第 12 手");
    archiveListWidget->addItem("2026-04-03 | 复盘模式 | 已完成");
    archiveListWidget->addItem("2026-04-02 | 训练样本导入记录");
    archiveLayout->addWidget(archiveTitle);
    archiveLayout->addWidget(archiveListWidget);

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
}
