#include <QtWidgets/QDialog>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QGridLayout>
#include<QtWidgets/QDialogButtonBox>
#include <functional>
#include <Connector.h>
class EncoderSetDialog : public QDialog
{
private:
    QVBoxLayout *mainLayout;
    QWidget *mainWidget;
   

public:
    EncoderSetDialog(SendMsgHandler msgHandler);
    ~EncoderSetDialog();
    void SetZeroPos(char joint);
    

};

EncoderSetDialog::EncoderSetDialog(SendMsgHandler msgHandler)
{
   
    this->mainLayout = new QVBoxLayout();
    this->mainWidget = new QWidget(this);
    this->mainWidget->setLayout(this->mainLayout);
    this->mainWidget->resize(600, 200);

    QGridLayout *resetButton_lay = new QGridLayout();
    QPushButton *LHipPos_frontal = new QPushButton("Left Hip Frontal");
    QPushButton *LHipPos_sagittal = new QPushButton("Left Hip Sagittal");
    QPushButton *RHipPos_frontal = new QPushButton("Right Hip Frontal");
    QPushButton *RHipPos_sagittal = new QPushButton("Right Hip Sagittal");
    QPushButton *LKnePos_sagittal = new QPushButton("Left Knee Sagittal");
    QPushButton *RKnePos_sagittal = new QPushButton("Right Knee Sagittal");
    QPushButton *LAnkPos_sagittal = new QPushButton("Left Ankle Sagittal");
    QPushButton *RAnkPos_sagittal = new QPushButton("Right Ankle Sagittal");

    resetButton_lay->addWidget(LHipPos_frontal, 0, 0);
    resetButton_lay->addWidget(LHipPos_sagittal, 1, 0);
    resetButton_lay->addWidget(LKnePos_sagittal, 2, 0);
    resetButton_lay->addWidget(LAnkPos_sagittal, 3, 0);

    resetButton_lay->addWidget(RHipPos_frontal, 0, 1);
    resetButton_lay->addWidget(RHipPos_sagittal, 1, 1);
    resetButton_lay->addWidget(RKnePos_sagittal, 2, 1);
    resetButton_lay->addWidget(RAnkPos_sagittal, 3, 1);

    this->mainLayout->addLayout(resetButton_lay);
    connect(LHipPos_frontal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerolhipf"); });
    connect(RHipPos_frontal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerorhipf"); });
    connect(LHipPos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerolhips"); });
    connect(RHipPos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerorhips"); });
    connect(LKnePos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerolknes"); });
    connect(RKnePos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerorknes"); });
    connect(LAnkPos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzerolanks"); });
    connect(RAnkPos_sagittal, &QPushButton::clicked, [msgHandler] { msgHandler("setzeroranks"); });

}

EncoderSetDialog::~EncoderSetDialog()
{
}
