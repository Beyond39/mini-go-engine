#include "ReviewDialog.h"
#include "BoardWidget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QGroupBox>
#include <QFont>

ReviewDialog::ReviewDialog(const Game& baseGame, QWidget* parent)
    : QDialog(parent),
      initialGame(baseGame)
{
    setWindowTitle("对局研究 / 试下");
    resize(980, 720);
    setupUI();
    setupConnections();

    reviewBoard->setAIEnabled(false);
    reviewBoard->loadGame(initialGame);
    refreshMoveList();
}

void ReviewDialog::setupUI()
{
    setStyleSheet(R"(
        QDialog {
            background: #efeae2;
            color: #2c2a25;
        }
        QGroupBox {
            font-size: 14px;
            font-weight: 600;
            color: #4b4035;
            border: 1px solid #d8d0c4;
            border-radius: 10px;
            margin-top: 12px;
            padding-top: 10px;
            background: #fcfbf8;
        }
        QPushButton {
            background: #ffffff;
            border: 1px solid #cfc6ba;
            border-radius: 8px;
            padding: 8px 12px;
        }
        QPushButton:hover { background: #f3eee6; }
        QPushButton:pressed { background: #e8dfd3; }
        QListWidget {
            background: #ffffff;
            border: 1px solid #e0d8cc;
            border-radius: 8px;
            padding: 4px;
            outline: none;
        }
        QListWidget::item { padding: 7px 6px; border-radius: 6px; }
        QListWidget::item:hover { background: #f5efe6; }
        QListWidget::item:selected {
            background: #8a6a43;
            color: #fffaf2;
        }
    )");

    auto* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(16, 14, 16, 14);
    rootLayout->setSpacing(10);

    auto* title = new QLabel("对局研究：在当前局面上自由试下", this);
    QFont titleFont = title->font();
    titleFont.setPointSize(15);
    titleFont.setBold(true);
    title->setFont(titleFont);

    auto* hint = new QLabel("这里的落子只存在于研究窗口，不会影响主棋盘。适合复盘、变化图、试应手。", this);
    hint->setStyleSheet("color: #6a5b4a;");

    auto* contentLayout = new QHBoxLayout();
    contentLayout->setSpacing(12);

    reviewBoard = new BoardWidget(this);
    reviewBoard->setMinimumSize(620, 620);

    auto* rightBox = new QGroupBox("研究记录", this);
    auto* rightLayout = new QVBoxLayout(rightBox);

    statusLabel = new QLabel("可直接在棋盘上试下", rightBox);
    moveListWidget = new QListWidget(rightBox);

    passButton = new QPushButton("停一手", rightBox);
    undoButton = new QPushButton("撤销研究手", rightBox);
    resetButton = new QPushButton("回到进入时局面", rightBox);
    closeButton = new QPushButton("关闭", rightBox);

    rightLayout->addWidget(statusLabel);
    rightLayout->addWidget(moveListWidget, 1);
    rightLayout->addWidget(passButton);
    rightLayout->addWidget(undoButton);
    rightLayout->addWidget(resetButton);
    rightLayout->addWidget(closeButton);

    contentLayout->addWidget(reviewBoard, 3);
    contentLayout->addWidget(rightBox, 1);

    rootLayout->addWidget(title);
    rootLayout->addWidget(hint);
    rootLayout->addLayout(contentLayout, 1);
}

void ReviewDialog::setupConnections()
{
    connect(passButton, &QPushButton::clicked, reviewBoard, &BoardWidget::passTurn);
    connect(undoButton, &QPushButton::clicked, reviewBoard, &BoardWidget::undoLastMove);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    connect(resetButton, &QPushButton::clicked, this, [this]() {
        reviewBoard->loadGame(initialGame);
        statusLabel->setText("已回到进入研究时的局面");
        refreshMoveList();
    });

    connect(reviewBoard, &BoardWidget::movePlayed, this, [this](int x, int y, Stone color) {
        statusLabel->setText(QString("研究落子：%1 %2").arg(stoneToString(color), moveToString(x, y)));
        refreshMoveList();
    });

    connect(reviewBoard, &BoardWidget::passPlayed, this, [this](Stone color) {
        statusLabel->setText(QString("研究落子：%1 停一手").arg(stoneToString(color)));
        refreshMoveList();
    });

    connect(reviewBoard, &BoardWidget::moveUndone, this, [this]() {
        statusLabel->setText("已撤销一步研究手");
        refreshMoveList();
    });

    connect(reviewBoard, &BoardWidget::illegalAction, this, [this](const QString& message) {
        statusLabel->setText(message);
    });
}

void ReviewDialog::refreshMoveList()
{
    moveListWidget->clear();

    const auto& history = reviewBoard->getGame().getHistory();
    for (int i = 0; i < (int)history.size(); ++i) {
        const RecordMove& record = history[i];
        QString text;
        if (record.isPass) {
            text = QString("%1. %2 停一手").arg(i + 1).arg(stoneToString(record.color));
        } else {
            text = QString("%1. %2 %3").arg(i + 1).arg(stoneToString(record.color), moveToString(record.x, record.y));
        }
        moveListWidget->addItem(text);
    }

    if (moveListWidget->count() > 0) {
        moveListWidget->setCurrentRow(moveListWidget->count() - 1);
    }
}

QString ReviewDialog::stoneToString(Stone color) const
{
    if (color == Stone::BLACK) {
        return "黑";
    }
    if (color == Stone::WHITE) {
        return "白";
    }
    return "空";
}

QString ReviewDialog::moveToString(int x, int y) const
{
    const QString columns = "ABCDEFGHJKLMNOPQRST";
    if (x < 0 || x >= columns.size()) {
        return QString("(%1,%2)").arg(x).arg(y);
    }
    return QString("%1%2").arg(columns[x]).arg(19 - y);
}
