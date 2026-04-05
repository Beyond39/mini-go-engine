#pragma once

#include <QWidget>

class QPushButton ; 
class QListWidget ;
class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    void startAIGameRequested() ;

private:
    QPushButton *startButton ;
    QPushButton *continueButton;
    QListWidget *popularListWidget;
    QListWidget *archiveListWidget;

    void setupUI() ;
    void setupConnections() ;
};