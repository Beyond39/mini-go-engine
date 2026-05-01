#include "mainwindow.h"
#include "../pages/HomePage.h"
#include "../pages/GamePage.h"

#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(nullptr),
      homepage(nullptr),
      gamepage(nullptr)
{
    resize(1400, 900);
    setWindowTitle("围棋AI对弈平台");

    setupPages();
    setupConnections();
}

MainWindow::~MainWindow(){
}

void MainWindow::setupPages()
{
    stackedWidget = new QStackedWidget(this);

    homepage = new HomePage(this);
    gamepage = new GamePage(this);

    stackedWidget->addWidget(homepage);
    stackedWidget->addWidget(gamepage);

    setCentralWidget(stackedWidget);
    stackedWidget->setCurrentWidget(homepage);
}

void MainWindow::setupConnections(){
    connect(homepage, &HomePage::startAIGameRequested, this, [=]() {
        QString mode = homepage->getSelectedMode();
        QString color = homepage->getSelectedColor();

        if (mode == "人机对局") {
            if (color == "我执黑") {
                gamepage->setAIMode(true, Stone::WHITE);
            } 
            else {
                gamepage->setAIMode(true, Stone::BLACK);
            }
        } 
        else {
            gamepage->setAIMode(false, Stone::WHITE);
        }

        gamepage->startNewGame();
        stackedWidget->setCurrentWidget(gamepage);
    });

   connect(homepage, &HomePage::sgfSelected, this, [this](const QString& path) {
        gamepage->setAIMode(false, Stone::WHITE);
        stackedWidget->setCurrentWidget(gamepage);
        gamepage->loadSGFFile(path);
    });

    connect(gamepage, &GamePage::backToHomeRequested, this, [=]() {
        stackedWidget->setCurrentWidget(homepage);
    });
}
