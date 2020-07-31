#ifndef CONNECTSETDIALOG_H
#define CONNECTSETDIALOG_H
#include<QtWidgets/QDialog>
#include<QtWidgets/QWidget>
#include<QtWidgets/QVBoxLayout>
#include "Connector.h"
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
class ConnectSetDialog:public QDialog
{
private:
    QWidget *mainWidget;
    QVBoxLayout *mainLayout;
    Connector *conn;
    void accept();
    void cancel();
    QLineEdit *ip_input;
    QLineEdit *port_input;

public:
    ConnectSetDialog(Connector*);
    ~ConnectSetDialog();
};
#endif
