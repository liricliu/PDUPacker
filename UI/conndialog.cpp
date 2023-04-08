#include "conndialog.h"
#include "ui_conndialog.h"
#include <QMessageBox>
#include <QTime>
#include <QtNetwork>

ConnDialog::ConnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnDialog)
{
    ui->setupUi(this);
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setChann(int)));
    connect(ui->pushButton_5,&QPushButton::clicked,this,&ConnDialog::connectClicked);
    connect(ui->pushButton,&QPushButton::clicked,this,&ConnDialog::localMAC);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&ConnDialog::localIP);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&ConnDialog::randIP);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&ConnDialog::randMAC);
    connect(CommChannel::instance(),&CommChannel::connection_ready,this,&ConnDialog::onConnReady);
    connect(CommChannel::instance(),&CommChannel::disconnected,this,&ConnDialog::onConnDown);
}

ConnDialog::~ConnDialog()
{
    delete ui;
}

void ConnDialog::accept(){

}

void ConnDialog::setChann(int index){
    ui->stackedWidget->setCurrentIndex(index);
    switch (index) {
    case 0:
    case 2:
        ui->pushButton_5->setText("连接");
        break;
    case 1:
        ui->pushButton_5->setText("监听");
        break;
    }
}
void ConnDialog::connectClicked(){
    //检查信道设置是否正确
    if(!CommChannel::is_ip(ui->lineEdit_2->text())){
        QMessageBox::warning(this,"警告","IP格式不正确");
        return;
    }else{
        CommChannel::instance()->set_ip(ui->lineEdit_2->text());
    }
    if(!CommChannel::is_mac(ui->lineEdit->text())){
        QMessageBox::warning(this,"警告","MAC格式不正确");
        return;
    }else{
        CommChannel::instance()->set_mac(ui->lineEdit->text());
    }

    switch(ui->stackedWidget->currentIndex()){
    case 0://处理TCP连接
        CommChannel::instance()->set_type(ChannelType::tcp_c);
        if(!CommChannel::is_ip(ui->lineEdit_3->text())){
            QMessageBox::warning(this,"警告","IP格式不正确");
            return;
        }
        CommChannel::instance()->set_conn_ip(ui->lineEdit_3->text());
        bool is_ok;
        ui->lineEdit_4->text().toInt(&is_ok);
        if((!is_ok)||(!(ui->lineEdit_4->text().toInt()<65536))){
            QMessageBox::warning(this,"警告","端口号不正确");
            return;
        }
        CommChannel::instance()->set_conn_port(ui->lineEdit_4->text().toInt());

        break;
    case 2://处理COM连接
        CommChannel::instance()->set_type(ChannelType::com);
        CommChannel::instance()->set_com_name(ui->comboBox_2->currentText());
        break;
    case 1://处理TCP监听
        CommChannel::instance()->set_type(ChannelType::tcp_s);
        //CommChannel::_instance->type=ChannelType::tcp_s;
        ui->lineEdit_5->text().toInt(&is_ok);
        if((!is_ok)||(!(ui->lineEdit_5->text().toInt()<65536))){
            QMessageBox::warning(this,"警告","端口号不正确");
            return;
        }
        CommChannel::instance()->set_listen_port(ui->lineEdit_5->text().toInt());
        ui->label_5->setText("正在监听"+ui->lineEdit_5->text());
        break;
    }
    CommChannel::instance()->connect_chan();
}
void ConnDialog::randIP(){
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QString ip=QString();
    ip.append(QString::number(qrand()%256));
    ip.append('.');
    ip.append(QString::number(qrand()%256));
    ip.append('.');
    ip.append(QString::number(qrand()%256));
    ip.append('.');
    ip.append(QString::number(qrand()%256));
    ui->lineEdit_2->setText(ip);
}
void ConnDialog::randMAC(){
    qsrand(QTime(0,0,0).secsTo(QTime::currentTime()));
    QString mac=QString();
    mac.append(QString::number(qrand()%256,16).toUpper());
    mac.append(':');
    mac.append(QString::number(qrand()%256,16).toUpper());
    mac.append(':');
    mac.append(QString::number(qrand()%256,16).toUpper());
    mac.append(':');
    mac.append(QString::number(qrand()%256,16).toUpper());
    mac.append(':');
    mac.append(QString::number(qrand()%256,16).toUpper());
    mac.append(':');
    mac.append(QString::number(qrand()%256,16).toUpper());
    ui->lineEdit->setText(mac);
}
void ConnDialog::localIP(){
    ui->lineEdit_2->setText(QNetworkInterface().allAddresses().at(0).toString());
}
void ConnDialog::localMAC(){
    ui->lineEdit->setText(QNetworkInterface().allInterfaces().at(0).hardwareAddress());
}
void ConnDialog::onConnReady(){
    ui->label_4->setText(CommChannel::instance()->get_o_mac());
    ui->label_8->setText(CommChannel::instance()->get_o_ip());
    QString cons="已连接：";
    switch (CommChannel::instance()->get_type()) {
    case ChannelType::tcp_c:
    case ChannelType::tcp_s:
        cons.append("TCP虚拟信道[");
        cons.append(CommChannel::instance()->get_peer_ip());
        cons.append("]");
        break;
    case ChannelType::com:
        cons.append("COM信道");
        break;
    case ChannelType::unset:
        cons="未连接";
        break;
    }
    ui->label_5->setText(cons);
    return;
}
void ConnDialog::onConnDown(){
    ui->label_5->setText("未连接");
    ui->label_4->setText("未知");
    ui->label_8->setText("未知");
}
