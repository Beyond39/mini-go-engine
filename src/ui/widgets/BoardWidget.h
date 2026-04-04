#pragma once 

#include <QWidget>
#include "Board.h"

class BoardWidget : public QWidget 
{
    Q_OBJECT

public:
    explicit BoardWidget(QWidget *parent = nullptr) ;
    void resetBoard() ;
    QSize sizeHint() const override;

signals:
    void movePlayed(int x, int y,Stone playedColor);

protected:
    void paintEvent(QPaintEvent *event) override ;
    void mousePressEvent(QMouseEvent *event) override ;

private:
    Board board ;
    Stone currentPlayer ;
    QPoint lastmove ;
    int boardPadding ;

    int margin ;
    int cellSize ;

    void drawBoard(QPainter &painter) ;
    void drawStarPoints(QPainter &painter);
    void drawStones(QPainter &painter);

    QRectF boardRect() const;
    qreal gridStep() const;
    QPointF boardPoint(int x, int y) const;
    QPoint pixelToBoard(const QPoint &pos) const;
} ;