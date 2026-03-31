#pragma once

#include <QWidget>

class QPushButton ; 

class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    void startAIGameRequested() ;

private:
    QPushButton *startButton ;

    void setupUI() ;
    void setupConnections() ;
};