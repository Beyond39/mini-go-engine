#pragma once

#include <QWidget>

class QPushButton ; 
class QListWidget ;
class QLabel ;
class QComboBox ;
class HomePage : public QWidget
{
    Q_OBJECT

public:
    explicit HomePage(QWidget *parent = nullptr);

signals:
    void startAIGameRequested() ;
    void sgfSelected(QString path) ;

private:
    QPushButton *startButton ;
    QPushButton *continueButton;
    QPushButton *openSgfButton;
    QPushButton *rulesButton;

    QListWidget *popularListWidget ;
    QListWidget *archiveListWidget ;
    QListWidget *sgfListWidget ;

    QLabel *statusTagLabel ;
    QComboBox *modeComboBox ;
    QComboBox *colorComboBox ;

    void setupUI() ;
    void setupConnections() ;
    void loadSGFList() ;
};