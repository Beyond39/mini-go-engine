#pragma once

#include <QDialog>
#include "../core/Game.h"

class QLabel;
class QListWidget;
class QPushButton;
class BoardWidget;

// 对局研究窗口：复制当前局面用于试下，不会污染主对局。
class ReviewDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ReviewDialog(const Game& baseGame, QWidget* parent = nullptr);

private:
    Game initialGame;

    BoardWidget* reviewBoard = nullptr;
    QListWidget* moveListWidget = nullptr;
    QLabel* statusLabel = nullptr;

    QPushButton* passButton = nullptr;
    QPushButton* undoButton = nullptr;
    QPushButton* resetButton = nullptr;
    QPushButton* closeButton = nullptr;

    void setupUI();
    void setupConnections();
    void refreshMoveList();

    QString stoneToString(Stone color) const;
    QString moveToString(int x, int y) const;
};
