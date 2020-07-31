#include "ConnectSetDialog.h"
#include <QtCore/QString>

ConnectSetDialog::ConnectSetDialog(Connector* _conn)
{
    this->conn = _conn;
    this->mainLayout = new QVBoxLayout();
    this->mainWidget = new QWidget(this);//this makes our widget parent to this widget
    this->mainWidget->setLayout(this->mainLayout);
    this->mainWidget->resize(200, 100);

    QLabel *ip_text = new QLabel("IP Address:");
    QLabel *port_text = new QLabel("Port:");
    this->ip_input = new QLineEdit();
    this->ip_input->setText(QString::fromStdString(this->conn->ip));
    this->port_input = new QLineEdit();
    this->port_input->setText(QString::number(this->conn->port));
    QGridLayout *setting_lay = new QGridLayout();
    setting_lay->addWidget(ip_text, 0, 0);
    setting_lay->addWidget(ip_input, 0, 1);
    setting_lay->addWidget(port_text, 1, 0);
    setting_lay->addWidget(port_input, 1, 1);
    this->mainLayout->addLayout(setting_lay);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    this->mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, [this] { accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [this] { cancel(); });
    connect(this, &ConnectSetDialog::rejected, [this] { cancel(); });
}

ConnectSetDialog::~ConnectSetDialog()
{

}

void ConnectSetDialog::accept(){
    this->conn->SetIp(this->ip_input->text().toStdString(), this->port_input->text().toInt());
    this->close();
}
void ConnectSetDialog::cancel(){
    this->ip_input->setText(QString::fromStdString(this->conn->ip));
    this->port_input->setText(QString::number(this->conn->port));
    this->close();
}
