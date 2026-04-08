#pragma once

#include <QWidget>
#include <QListWidget>

#include "Game.h" 
#include "sgf_utils.h"

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
    void loadSGFFile(const QString& path) ;

signals:
    void backToHomeRequested();

private:
    Game game ;
    std::vector<Move> currentMoves ;

    QPushButton *backButton ;
    QPushButton *passButton ;
    QPushButton *undoButton;
    QPushButton *resignButton;
    QPushButton *restartButton;
    QPushButton *openSGFButton ;
    QPushButton *saveSGFButton ;
    BoardWidget *boardwidget ;

    QLabel *statusLabel ;
    QLabel *currentTurnLabel ;
    QLabel *winRateLabel ;
    QLabel *scoreLabel ;
    QListWidget *moveListWidget ;

    void setupUI() ;
    void setupConnections() ;
    void resetInfoPanel() ;
    void updatePage() ;

    QString moveToString(int x, int y) const ;
    QString stoneToString(Stone color) const ;
};