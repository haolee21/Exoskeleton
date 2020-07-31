// For the gui part I just defined in header since I use forward declaration often (e.g. auto) 
// since sometimes I am not sure which widget/layout to use, yet they are all similar

#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QAction>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenuBar>
#include "Connector.h"
#include <iostream>
#include "ConnectSetDialog.h"
#include<QtWidgets/QVBoxLayout>
#include<QtWidgets/QGroupBox>
//define index for tasks
#define INIT_LANK 0
#define INIT_RANK 1
#define INIT_LKNE 2
#define INIT_RKNE 3
#define REC_LANK 4
#define REC_RANK 5
#define REC_LKNE 6
#define REC_RKNE 7
#define NUM_TASKS 8
#define END_TASK 9
#define TEST_VAL 10
#define TEST_PWM 11
class MainWindow:public QMainWindow
{
   Q_OBJECT
private:
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;
    Connector *conn;
    void genMenuBar();
    void genConnect_lay();
    void genTask_lay();
    
    //actions
    
    QCheckBox* checkBoxList[NUM_TASKS];
    void actTask(char task); //we won't have more than 2^8 tasks
    

    //MenuBar
    
    //Current ip/status
    QLineEdit *curIP;
    QLineEdit *curPort;
    QLabel *status;
    void connectExo();
private slots:
    void sendTask();
public:
    MainWindow(Connector *conn);
    ~MainWindow();
    void UpdateCon();
};


#endif