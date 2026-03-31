#include "GamePage.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

GamePage::GamePage(QWidget *parent)
    : QWidget(parent),
      backButton(nullptr)
{
    setupUI();
    setupConnections();
}

void GamePage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("这是对局页 GamePage", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    backButton = new QPushButton("返回首页", this);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(backButton);
}

void GamePage::setupConnections()
{
    connect(backButton, &QPushButton::clicked, this, [=]() {
        emit backToHomeRequested();
    });
}

