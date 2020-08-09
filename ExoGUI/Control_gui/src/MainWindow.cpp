#include "MainWindow.h"
#include "TaskSetDialog.h"
#include "EncoderSetDialog.h"
MainWindow::MainWindow(Connector *_conn)
{
    this->conn = _conn;
    this->mainLayout = new QVBoxLayout();
    this->mainWidget = new QWidget(this);
    this->mainWidget->setLayout(this->mainLayout);
    this->setCentralWidget(this->mainWidget);
    this->genMenuBar();
    this->genConnect_lay();
    this->genTask_lay();

   
}

MainWindow::~MainWindow()
{
}
void MainWindow::genMenuBar(){
    QMenu* sys_setMenu = this->menuBar()->addMenu(tr("&System Setting"));
    
    QAction *act_setConnect = new QAction("&Connection Setting");
    sys_setMenu->addAction(act_setConnect);
    QMenu* con_setMenu = this->menuBar()->addMenu(tr("&Exo Settings"));
    QAction *act_taskSet = new QAction("&Tasks Settings");
    con_setMenu->addAction(act_taskSet);
    QAction *act_encoderSet = new QAction("&Encoder Settings");
    con_setMenu->addAction(act_encoderSet);

    ConnectSetDialog *connSetDiag = new ConnectSetDialog(this->conn);
    connect(act_setConnect, &QAction::triggered, [connSetDiag] { connSetDiag->show(); });
    connect(connSetDiag, &ConnectSetDialog::finished, [this] { this->UpdateCon(); });

    TaskSetDialog *taskSetDiag = new TaskSetDialog(this->conn);
    connect(act_taskSet, &QAction::triggered, [taskSetDiag] { taskSetDiag->show(); });
 
    EncoderSetDialog *encoderSetDiag = new EncoderSetDialog(
        [&conn = this->conn](const std::string &msg) { conn->Send(msg); }); 
    //    instead of passing the whole conn or even connector, I just pass a function to run, [&conn=this->conn] is the way c++14 catch member in object
    connect(act_encoderSet, &QAction::triggered, [encoderSetDiag] { encoderSetDiag->show(); });
}
void MainWindow::genConnect_lay(){
    QHBoxLayout *curCon_lay = new QHBoxLayout();
    QLabel *curIP_lab = new QLabel("IP: ");
    QLabel *curPort_lab = new QLabel("Port: ");
    this->curIP = new QLineEdit();
    this->curPort = new QLineEdit();
    QLabel *cur_stat = new QLabel("Status: ");
    this->status = new QLabel();
    status->setText("offline");
    curIP->setReadOnly(true);
    curPort->setReadOnly(true);
    curIP->setText(QString::fromStdString(this->conn->ip));
    curPort->setText(QString::number(this->conn->port));
    curCon_lay->addWidget(curIP_lab);
    curCon_lay->addWidget(curIP);
    curCon_lay->addWidget(curPort_lab);
    curCon_lay->addWidget(curPort);
    curCon_lay->addWidget(cur_stat);
    curCon_lay->addWidget(status);
    this->mainLayout->addLayout(curCon_lay);

    QBoxLayout *con_lay = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
    QPushButton *but_connect = new QPushButton("Connect");
    QPushButton *but_reboot=new QPushButton("Reboot");
    QPushButton *but_shutdown = new QPushButton("Shut Down");
    con_lay->addWidget(but_connect);
    con_lay->addWidget(but_reboot);
    con_lay->addWidget(but_shutdown);
    this->mainLayout->addLayout(con_lay);
    connect(but_connect, &QPushButton::clicked, [this] { this->connectExo(); });
}
void MainWindow::genTask_lay(){
    auto *task_lay = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
    auto *jobs_lay = new QGridLayout();
    
    this->checkBoxList[INIT_LANK] = new QCheckBox("Init LAnk");
    this->checkBoxList[INIT_RANK] = new QCheckBox("Init RAnk");
    this->checkBoxList[INIT_LKNE] = new QCheckBox("Init LKne");
    this->checkBoxList[INIT_RKNE] = new QCheckBox("Init RKne");
    this->checkBoxList[REC_LANK]  = new QCheckBox("Rec LAnk");
    this->checkBoxList[REC_RANK]  = new QCheckBox("Rec RAnk");
    this->checkBoxList[REC_LKNE]  = new QCheckBox("Rec LKne");
    this->checkBoxList[REC_RKNE]  = new QCheckBox("Rec RKne");

    jobs_lay->addWidget(this->checkBoxList[INIT_LANK], 0, 0);
    jobs_lay->addWidget(this->checkBoxList[INIT_RANK], 1, 0);
    jobs_lay->addWidget(this->checkBoxList[INIT_LKNE], 0, 1);
    jobs_lay->addWidget(this->checkBoxList[INIT_RKNE], 1, 1);
    jobs_lay->addWidget(this->checkBoxList[REC_LANK], 0, 2);
    jobs_lay->addWidget(this->checkBoxList[REC_RANK], 1, 2);
    jobs_lay->addWidget(this->checkBoxList[REC_LKNE], 0, 3);
    jobs_lay->addWidget(this->checkBoxList[REC_RKNE], 1, 3);
    auto *but_lay = new QGridLayout();

    QPushButton *but_send_task = new QPushButton("&Send");
    QPushButton *but_end_task = new QPushButton("&End All");
    QPushButton *but_test_val = new QPushButton("Test Valve");
    QPushButton *but_test_pwm = new QPushButton("Test PWM");
    but_lay->addWidget(but_send_task, 0, 0);
    but_lay->addWidget(but_end_task,0,1);
    but_lay->addWidget(but_test_val, 1, 0);
    but_lay->addWidget(but_test_pwm, 1, 1);
    
    task_lay->addLayout(jobs_lay);
    task_lay->addLayout(but_lay);

    QGroupBox *taskBox = new QGroupBox("Exoskeleton Tasks");
    taskBox->setLayout(task_lay);
    this->mainLayout->addWidget(taskBox);

    connect(but_send_task, &QPushButton::clicked,[this]{sendTask();});
    connect(but_end_task, &QPushButton::clicked, [this] {actTask(END_TASK); });
    connect(but_test_val, &QPushButton::clicked, [this] { actTask(TEST_VAL); });
    connect(but_test_pwm, &QPushButton::clicked, [this] { actTask(TEST_PWM); });
}
void MainWindow::sendTask(){
    for (int i = 0; i < NUM_TASKS;i++){
        if(this->checkBoxList[i]->checkState()==Qt::Checked){
            this->actTask((char)i);
            this->checkBoxList[i]->setCheckState(Qt::Unchecked);
        }
    }
}
void MainWindow::actTask(char _task){
    switch (_task)
    {
    case INIT_LANK:
        this->conn->Send("actinitlank");
        break;
    case INIT_RANK:
        this->conn->Send("actinitrank");
        break;
    case INIT_LKNE:
        this->conn->Send("actinitlkne");
        break;
    case INIT_RKNE:
        this->conn->Send("actinitrkne");
        break;
    case REC_LANK:
        this->conn->Send("actreclank");
        break;
    case REC_RANK:
        this->conn->Send("actrecrank");
        break;
    case REC_LKNE:
        this->conn->Send("actreclkne");
        break;
    case REC_RKNE:
        this->conn->Send("actrecrkne");
        break;

    default:
        break;
    }
    


}
void MainWindow::UpdateCon(){
    this->curIP->setText(QString::fromStdString(this->conn->ip));
    this->curPort->setText(QString::number(this->conn->port));

}
void MainWindow::connectExo(){
    this->conn->Connect();
    if (this->conn->isConnect)
    {
        this->status->setText("connected");
    }
}