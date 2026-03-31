#pragma once

#include <QWidget>

class QPushButton;

class GamePage : public QWidget
{
    Q_OBJECT

public:
    explicit GamePage(QWidget *parent = nullptr);

signals:
    void backToHomeRequested();

private:
    QPushButton *backButton;

    void setupUI();
    void setupConnections();

};