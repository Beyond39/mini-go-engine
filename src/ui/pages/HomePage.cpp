#include "HomePage.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

HomePage::HomePage(QWidget *parent)
    : QWidget(parent),
      startButton(nullptr)
{
    setupUI();
    setupConnections();
}

void HomePage::setupUI()
{
    auto *mainLayout = new QVBoxLayout(this);

    auto *titleLabel = new QLabel("围棋平台界面", this);
    titleLabel->setAlignment(Qt::AlignCenter);

    startButton = new QPushButton("开始人机对局", this);

    mainLayout->addWidget(titleLabel);
    mainLayout->addWidget(startButton);
}

void HomePage::setupConnections()
{
    connect(startButton, &QPushButton::clicked, this, [=]() {
        emit startAIGameRequested();
    });
}
