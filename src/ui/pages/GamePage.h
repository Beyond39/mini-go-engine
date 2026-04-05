#pragma once

#include <QWidget>
#include <QListWidget>

class QPushButton;
class BoardWidget ;
class QListWidget ;
class QLabel ;

enum class Stone;

class GamePage : public QWidget
{
    Q_OBJECT

public:
    explicit GamePage(QWidget *parent = nullptr);

signals:
    void backToHomeRequested();

private:
    QPushButton *backButton ;
    QPushButton *passButton ;
    QPushButton *undoButton;
    QPushButton *resignButton;
    QPushButton *restartButton;
    BoardWidget *boardwidget ;

    QLabel *statusLabel ;
    QLabel *currentTurnLabel ;
    QLabel *winRateLabel ;
    QLabel *scoreLabel ;
    QListWidget *moveListWidget ;

    void setupUI() ;
    void setupConnections() ;
    void resetInfoPanel() ;
    QString moveToString(int x, int y) const ;
    QString stoneToString(Stone color) const ;
};