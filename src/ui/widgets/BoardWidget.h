#pragma once 

#include <QWidget>
#include <QPoint>
#include <QRectF>
#include <QString>
#include "../core/Game.h"

class QPainter;
class QMouseEvent;
class QPaintEvent;
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
    void playAIMove() ;

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
    QPoint lastmove ;
    bool finished;
    int boardPadding ;
    Game game ;
    QString finishText;
    Board board ;
    Stone aicolor ;
    bool aiEnabled ;

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
} ;