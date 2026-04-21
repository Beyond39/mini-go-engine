#pragma once

#include <QWidget>
#include <QListWidget>
#include <QFutureWatcher>

#include "core/Game.h" 
#include "sgf/sgf_utils.h"

class QPushButton;
class BoardWidget ;
class QListWidget ;
class QLabel ;
class PythonEvaluator ;
class MCTS ;

class GamePage : public QWidget
{
    Q_OBJECT

public:
    explicit GamePage(QWidget *parent = nullptr);
    void loadSGFFile(const QString& path) ;
    void setAIMode(bool enabled , Stone aicolor) ;
    void startNewGame() ;
    void goToStep(int num) ;
    void goForward() ;
    void goBackward() ;
    ~GamePage();

signals:
    void backToHomeRequested();

private:
    Game game ;
    std::vector<Move> currentMoves ;
    int replayIndex = 0 ;

    bool aiEnabled = false;
    Stone aiColor = Stone::WHITE;

    QFutureWatcher<Move>* aiWatcher;
    bool aiThinking = false;

    std::unique_ptr<PythonEvaluator> evaluator;
    std::unique_ptr<MCTS> mcts;

    QPushButton *backButton ;
    QPushButton *passButton ;
    QPushButton *undoButton;
    QPushButton *resignButton;
    QPushButton *restartButton;
    QPushButton *openSGFButton ;
    QPushButton *saveSGFButton ;
    BoardWidget *boardwidget ;
    QPushButton *stepForward ;
    QPushButton *stepBackward ;

    QLabel *statusLabel ;
    QLabel *currentTurnLabel ;
    QLabel *winRateLabel ;
    QLabel *scoreLabel ;
    QListWidget *moveListWidget ;

    void setupUI() ;
    void setupConnections() ;
    void resetInfoPanel() ;
    void updatePage() ;
    void tryAIMove();

    QString moveToString(int x, int y) const ;
    QString stoneToString(Stone color) const ;
};