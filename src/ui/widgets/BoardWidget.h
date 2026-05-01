#pragma once 

#include <QWidget>
#include <QPoint>
#include <QRectF>
#include <QString>
#include <QSoundEffect>
#include "../core/Game.h"


class QPainter;
class QMouseEvent;
class QPaintEvent;
class QSoundEffect;
class BoardWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr) ;
    void resetBoard() ;
    QSize sizeHint() const override;
    Stone currentPlayer() const;
    bool isGameFinished() const;
    void setAIEnabled(bool Enabled) ;
    void setAIcolor(Stone color) ;
    const Game& getGame() const;
    int getMoveNumberAt(int x, int y) const;
    bool forcePlayMove(int x, int y) ;
    bool forcePass();

public slots:
    void undoLastMove();
    void passTurn();
    void resignCurrentPlayer();
    void loadGame(const Game& loadedGame) ;

signals:
    void movePlayed(int x, int y,Stone playedColor);
    void moveUndone();
    void passPlayed(Stone color);
    void turnChanged(Stone nextPlayer);
    void illegalAction(const QString &message);
    void gameReset();
    void gameOver(const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override ;
    void mousePressEvent(QMouseEvent *event) override ;

private:
    Game game ;
    QPoint lastmove ;
    bool finished;
    int boardPadding ;
    QString finishText;
    Stone aicolor ;
    bool aiEnabled ;
    QSoundEffect* stoneSound = nullptr;

    int margin ;
    int cellSize ;

    void drawBoard(QPainter &painter) ;
    void drawStarPoints(QPainter &painter);
    void drawStones(QPainter &painter);
    void drawFinishedOverlay(QPainter &painter);

    QRectF boardRect() const;
    qreal gridStep() const;
    QPointF boardPoint(int x, int y) const;
    QPoint pixelToBoard(const QPoint &pos) const;
    void updateLastMoveFromHistory();
    void checkGameEndAfterPass();

    void setupSounds();
    void playStoneSound();
} ;