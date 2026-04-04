#pragma once

#include <QWidget>
#include <QListWidget>

class QPushButton;
class BoardWidget ;
class QLabel ;

class GamePage : public QWidget
{
    Q_OBJECT

public:
    explicit GamePage(QWidget *parent = nullptr);

signals:
    void backToHomeRequested();

private:
    QPushButton *backButton;
    BoardWidget *boardwidget ;

    QLabel *statusLabel;
    QLabel *currentTurnLabel;
    QLabel *winRateLabel;
    QLabel *scoreLabel;
    
    QListWidget *moveListWidget;

    void setupUI();
    void setupConnections();
    QString moveToString(int x, int y) const ;
};