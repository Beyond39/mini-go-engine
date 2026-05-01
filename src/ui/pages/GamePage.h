#pragma once

#include <QWidget>
#include <QListWidget>
#include <QFutureWatcher>
#include <QVector>

#include "core/Game.h" 
#include "sgf/sgf_utils.h"

class QPushButton;
class BoardWidget ;
class QListWidget ;
class QLabel ;
class WinRateChartWidget;
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

    QFutureWatcher<Move>* aiWatcher = nullptr;
    bool aiThinking = false;

    QString scriptPath;
    QString modelPath;

    QPushButton *backButton ;
    QPushButton *passButton ;
    QPushButton *undoButton;
    QPushButton *resignButton;
    QPushButton *restartButton;
    QPushButton *openSGFButton ;
    QPushButton *saveSGFButton ;
    QPushButton* reviewButton ;
    BoardWidget *boardwidget ;
    QPushButton *stepForward ;
    QPushButton *stepBackward ;
    
    QLabel *statusLabel ;
    QLabel *currentTurnLabel ;
    QLabel *winRateLabel ;
    QLabel *scoreLabel ;
    QListWidget *moveListWidget ;
    WinRateChartWidget* winRateChart;

    QVector<double> winRates;

    void setupUI() ;
    void setupConnections() ;
    void resetInfoPanel() ;
    void updatePage() ;
    void tryAIMove();

    void refreshMoveList();
    void openSGFByDialog();
    void saveSGFByDialog();
    void openReviewDialog();

    void syncReplayCacheFromGame();
    void rebuildWinRateCurve(int uptoStep);
    void updateAnalysisLabels();

    bool isTwoPasses() const;
    double estimateBlackLead(const Game& state) const;
    double estimateBlackWinRate(const Game& state) const;

    QString moveToString(int x, int y) const ;
    QString stoneToString(Stone color) const ;
};
 