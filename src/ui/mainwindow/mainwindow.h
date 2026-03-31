#pragma once 

#include <QMainWindow>

class QStackedWidget ;
class HomePage ;
class GamePage ;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr) ;
    ~MainWindow() ;

private:
    QStackedWidget *stackedWidget ;
    HomePage *homepage ;
    GamePage *gamepage ;

    void setupPages() ;
    void setupConnections() ;
};

