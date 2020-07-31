#ifndef TASKSETDIALOG_H
#define TASKSETDIALOG_H
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QDialogButtonBox>
#include "Connector.h"
struct TaskParam
{
    int knePre=0;
    int ankPre=0;
    void setKnePre(int _val){
        if(_val>1024)
            knePre = 1024;
        else
            knePre = _val;

    }
    void setAnkPre(int _val){
        if(_val>1024)
            ankPre = 1024;
        else
            ankPre = _val;
    }
};

class TaskSetDialog:public QDialog
{
private:
    
    Connector *conn;
    TaskParam left_init;
    TaskParam right_init;
    TaskParam left_sup;
    TaskParam right_sup;
    TaskParam left_rec;
    TaskParam right_rec;

    QVBoxLayout *mainLayout;
    QWidget *mainWidget;

    void supPreUI();
    void initPreUI();
    void recPreUI();
    void ok();
    void cancel();

    QLineEdit *lAnkSup_in;
    QLineEdit *rAnkSup_in;
    QLineEdit *lKneSup_in;
    QLineEdit *rKneSup_in;

    QLineEdit *lAnkInit_in;
    QLineEdit *rAnkInit_in;
    QLineEdit *lKneInit_in;
    QLineEdit *rKneInit_in;

    QLineEdit *lAnkRec_in;
    QLineEdit *rAnkRec_in;
    QLineEdit *lKneRec_in;
    QLineEdit *rKneRec_in;

public:
    TaskSetDialog(Connector *conn);
    ~TaskSetDialog();
    void SendSetting();
};
#endif

