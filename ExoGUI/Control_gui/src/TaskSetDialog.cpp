#include "TaskSetDialog.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>
TaskSetDialog::TaskSetDialog(Connector *_conn)
{
    this->conn = _conn;
    this->mainLayout = new QVBoxLayout();
    this->mainWidget = new QWidget(this);
    this->mainWidget->setLayout(this->mainLayout);
    this->mainWidget->resize(400, 600);

    this->supPreUI();
    this->initPreUI();
    this->recPreUI();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    this->mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, [this] { ok(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [this] { cancel(); });
    connect(this, &TaskSetDialog::rejected, [this] { cancel(); });
}

TaskSetDialog::~TaskSetDialog()
{
}
void TaskSetDialog::SendSetting(){
    std::stringstream ss_pre;
    ss_pre << std::setfill('0') << std::setw(4) << std::to_string(this->left_sup.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_sup.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->left_sup.knePre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_sup.knePre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->left_init.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_init.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->left_init.knePre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_init.knePre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->left_rec.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_rec.ankPre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->left_rec.knePre)
           << std::setfill('0') << std::setw(4) << std::to_string(this->right_rec.knePre);
    conn->Send("setpre" + ss_pre.str());
}

void TaskSetDialog::supPreUI(){
    QGroupBox *supPreBox = new QGroupBox("Support Pressure settings");
    QGridLayout *supPreLay = new QGridLayout();
    QLabel *lab1_1 = new QLabel("Left Ankle: ");
    this->lAnkSup_in = new QLineEdit();
    this->lAnkSup_in->setText(QString::number(this->left_sup.ankPre));
    supPreLay->addWidget(lab1_1, 0, 0);
    supPreLay->addWidget(this->lAnkSup_in, 0, 1);
    
    QLabel *lab1_2 = new QLabel("Right Ankle: ");
    this->rAnkSup_in = new QLineEdit();
    this->rAnkSup_in->setText(QString::number(this->right_sup.ankPre));
    supPreLay->addWidget(lab1_2, 1, 0);
    supPreLay->addWidget(this->rAnkSup_in, 1, 1);

    QLabel *lab1_3 = new QLabel("Left Knee: ");
    this->lKneSup_in = new QLineEdit();
    this->lKneSup_in->setText(QString::number(this->left_sup.knePre));
    supPreLay->addWidget(lab1_3, 2, 0);
    supPreLay->addWidget(this->lKneSup_in, 2, 1);

    QLabel *lab1_4 = new QLabel("Right Knee: ");
    this->rKneSup_in = new QLineEdit();
    this->rKneSup_in->setText(QString::number(this->right_sup.ankPre));
    supPreLay->addWidget(lab1_4, 3, 0);
    supPreLay->addWidget(this->rKneSup_in, 3, 1);
    supPreBox->setLayout(supPreLay);
    this->mainLayout->addWidget(supPreBox);
}

void TaskSetDialog::initPreUI(){
    QGroupBox *initPreBox = new QGroupBox("Initialized Pressure settings");
    QGridLayout *initPreLay = new QGridLayout();
    QLabel *lab1_1 = new QLabel("Left Ankle: ");
    this->lAnkInit_in = new QLineEdit();
    this->lAnkInit_in->setText(QString::number(this->left_init.ankPre));
    initPreLay->addWidget(lab1_1, 0, 0);
    initPreLay->addWidget(this->lAnkInit_in, 0, 1);
    
    QLabel *lab1_2 = new QLabel("Right Ankle: ");
    this->rAnkInit_in = new QLineEdit();
    this->rAnkInit_in->setText(QString::number(this->right_init.ankPre));
    initPreLay->addWidget(lab1_2, 1, 0);
    initPreLay->addWidget(this->rAnkInit_in, 1, 1);

    QLabel *lab1_3 = new QLabel("Left Knee: ");
    this->lKneInit_in = new QLineEdit();
    this->lKneInit_in->setText(QString::number(this->left_init.knePre));
    initPreLay->addWidget(lab1_3, 2, 0);
    initPreLay->addWidget(this->lKneInit_in, 2, 1);

    QLabel *lab1_4 = new QLabel("Right Knee: ");
    this->rKneInit_in = new QLineEdit();
    this->rKneInit_in->setText(QString::number(this->right_init.ankPre));
    initPreLay->addWidget(lab1_4, 3, 0);
    initPreLay->addWidget(this->rKneInit_in, 3, 1);
    initPreBox->setLayout(initPreLay);
    this->mainLayout->addWidget(initPreBox);

}
void TaskSetDialog::recPreUI(){
    QGroupBox *recPreBox = new QGroupBox("Recycle Pressure settings");
    QGridLayout *recPreLay = new QGridLayout();
    QLabel *lab1_1 = new QLabel("Left Ankle: ");
    this->lAnkRec_in = new QLineEdit();
    this->lAnkRec_in->setText(QString::number(this->left_rec.ankPre));
    recPreLay->addWidget(lab1_1, 0, 0);
    recPreLay->addWidget(this->lAnkRec_in, 0, 1);
    
    QLabel *lab1_2 = new QLabel("Right Ankle: ");
    this->rAnkRec_in = new QLineEdit();
    this->rAnkRec_in->setText(QString::number(this->right_rec.ankPre));
    recPreLay->addWidget(lab1_2, 1, 0);
    recPreLay->addWidget(this->rAnkRec_in, 1, 1);

    QLabel *lab1_3 = new QLabel("Left Knee: ");
    this->lKneRec_in = new QLineEdit();
    this->lKneRec_in->setText(QString::number(this->left_rec.knePre));
    recPreLay->addWidget(lab1_3, 2, 0);
    recPreLay->addWidget(this->lKneRec_in, 2, 1);

    QLabel *lab1_4 = new QLabel("Right Knee: ");
    this->rKneRec_in = new QLineEdit();
    this->rKneRec_in->setText(QString::number(this->right_rec.ankPre));
    recPreLay->addWidget(lab1_4, 3, 0);
    recPreLay->addWidget(this->rKneRec_in, 3, 1);
    recPreBox->setLayout(recPreLay);
    this->mainLayout->addWidget(recPreBox);

}


void TaskSetDialog::ok(){
    this->left_sup.setAnkPre(this->lAnkSup_in->text().toInt());
    this->left_sup.setKnePre(this->lKneSup_in->text().toInt());
    this->right_sup.setAnkPre( this->rAnkSup_in->text().toInt());
    this->right_sup.setKnePre(this->rKneSup_in->text().toInt());

    this->left_init.setAnkPre(this->lAnkInit_in->text().toInt());
    this->left_init.setKnePre(this->lKneInit_in->text().toInt());
    this->right_init.setAnkPre(this->rAnkInit_in->text().toInt());
    this->right_init.setKnePre(this->rKneInit_in->text().toInt());

    this->left_rec.setAnkPre(this->lAnkRec_in->text().toInt());
    this->left_rec.setKnePre(this->lKneRec_in->text().toInt());
    this->right_rec.setAnkPre(this->rAnkRec_in->text().toInt());
    this->right_rec.setKnePre(this->rKneRec_in->text().toInt());
    this->SendSetting();
    this->close();
}
void TaskSetDialog::cancel(){
    this->lAnkSup_in->setText(QString::number(this->left_sup.ankPre));
    this->rAnkSup_in->setText(QString::number(this->right_sup.ankPre));
    this->lKneSup_in->setText(QString::number(this->left_sup.knePre));
    this->rKneSup_in->setText(QString::number(this->right_sup.knePre));

    this->lAnkInit_in->setText(QString::number(this->left_init.ankPre));
    this->rAnkInit_in->setText(QString::number(this->right_init.ankPre));
    this->lKneInit_in->setText(QString::number(this->left_init.knePre));
    this->rKneInit_in->setText(QString::number(this->right_init.knePre));

    this->lAnkRec_in->setText(QString::number(this->left_rec.ankPre));
    this->rAnkRec_in->setText(QString::number(this->right_rec.ankPre));
    this->lKneRec_in->setText(QString::number(this->left_rec.knePre));
    this->rKneRec_in->setText(QString::number(this->right_rec.knePre));
    this->close();
}
