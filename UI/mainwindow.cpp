#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{

    ui->setupUi(this);
    cdialog=new ConnDialog(this);
    connect(ui->actionTCP,&QAction::triggered,this,&MainWindow::onConnShouldOpen);
    connect(ui->pushButton_3,&QPushButton::clicked,this,&MainWindow::onIPV4ShouldGen);
    connect(ui->pushButton_3_v6,&QPushButton::clicked,this,&MainWindow::onIPV6ShouldGen);
    connect(ui->pushButton_5,&QPushButton::clicked,this,&MainWindow::onUseChanAddr);
    connect(ui->pushButton_4,&QPushButton::clicked,this,&MainWindow::onUseChanAddrIP);
    connect(ui->pushButton_2,&QPushButton::clicked,this,&MainWindow::onTX);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::onShouldParse);
    connect(CommChannel::instance(),&CommChannel::rx,this,&MainWindow::onRX);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onIPV4ShouldGen(){
    unsigned char DSCP_ECN=DSCP[ui->comboBox->currentIndex()]+ECN[ui->comboBox->currentIndex()];
    QString payload=ui->plainTextEdit_4->toPlainText();
    unsigned short length;
    unsigned char length_array[2];
    if(payload.size()<65536-20){
        length=payload.size()+20;
    }else{
        QMessageBox::warning(this,"警告","负载长度超出ipv4报文长度上限，已自动忽略64K后面的内容。");
        payload=payload.mid(0,65535-20);
        length=65535;
    }
    length_array[0]=length&0x00ff;
    length_array[1]=length>>8;
    bool is_ok;
    unsigned short biaoshi=ui->lineEdit_10->text().toInt(&is_ok);
    if(!(is_ok&&ui->lineEdit_10->text().toInt()<65536)){
        QMessageBox::warning(this,"警告","标识不允许大于65535");
        return;
    }
    unsigned char biaoshi_a[2];
    biaoshi_a[0]=biaoshi&0x00ff;
    biaoshi_a[1]=biaoshi>>8;
    QString dst_ip=ui->lineEdit_8->text();
    QString src_ip=ui->lineEdit_3->text();
    QStringList dst_ip_sl=dst_ip.split(".");
    QStringList src_ip_sl=src_ip.split(".");
    if((dst_ip_sl.count()!=4)||(src_ip_sl.count()!=4)){
        QMessageBox::warning(this,"警告","IPV4地址格式不正确。");
        return;
    }
    unsigned char dst_ip_array[4];
    unsigned char src_ip_array[4];
    for(int i=0;i<4;i++){
        bool isOk;
        dst_ip_sl[i].toInt(&isOk);
        if(isOk&&dst_ip_sl[i].toInt()<256){
            dst_ip_array[i]=dst_ip_sl[i].toInt();
        }else {
            QMessageBox::warning(this,"警告","IPV4地址格式不正确。");
            return;
        }
    }
    for(int i=0;i<4;i++){
        bool isOk;
        src_ip_sl[i].toInt(&isOk);
        if(isOk&&dst_ip_sl[i].toInt()<256){
            src_ip_array[i]=src_ip_sl[i].toInt();
        }else {
            QMessageBox::warning(this,"警告","IPV4地址格式不正确。");
            return;
        }
    }
    bool isOk;
    unsigned char ttl=ui->lineEdit_9->text().toInt(&isOk);
    if(!(ui->lineEdit_9->text().toInt(&isOk)<256&&isOk)){
        QMessageBox::warning(this,"警告","TTL不得大于255。");
        return;
    }
    unsigned char ip_head[20];
    memcpy(ip_head,"\x45",1);
    memcpy(ip_head+1,&DSCP_ECN,1);
    memcpy(ip_head+2,length_array,2);
    memcpy(ip_head+4,biaoshi_a,2);
    memcpy(ip_head+6,"\x00\x00",2);
    memcpy(ip_head+8,&ttl,1);
    memcpy(ip_head+9,"\x00",1);//上层协议
    memcpy(ip_head+10,"\x00\x00",2);//先将校验和置0
    memcpy(ip_head+12,src_ip_array,4);
    memcpy(ip_head+16,dst_ip_array,4);
    //计算校验和
    unsigned short* ip_head_sp=(unsigned short*)ip_head;
    unsigned long checksum=0;
    unsigned short checksum_s=0;
    for(int i=0;i<10;i++){
        checksum+=ip_head_sp[i];
    }
    checksum=(checksum>>16)+(checksum&0x0000ffff);
    checksum+=checksum>>16;
    checksum_s=(unsigned short)(~checksum);
    ip_head[10]=checksum_s&0x00ff;
    ip_head[11]=checksum_s>>8;

    //检验校验和
    checksum=0;
    for(int i=0;i<10;i++){
        checksum+=ip_head_sp[i];
    }
    checksum=(checksum>>16)+(checksum&0x0000ffff);
    checksum+=checksum>>16;
    checksum_s=(unsigned short)(~checksum);
    if(checksum_s!=0){
        QMessageBox::warning(this,"警告","你的电脑出故障了，请重试。");
        return;
    }

    QString fpath=QFileDialog::getSaveFileName(this,"选择ipv4文件保存路径",QDir::homePath(),"IPV4文件 (*.ipv4)");
    if(fpath.isEmpty()){
        return;
    }
    FILE* f=fopen(fpath.toUtf8(),"w");
    if(f==nullptr){
        QMessageBox::warning(this,"警告","无法创建文件，请检查权限。");
        return;
    }
    fwrite(ip_head,1,20,f);
    fwrite(payload.toUtf8(),1,length-20,f);
    fclose(f);
}

void MainWindow::onIPV6ShouldGen(){

}

void MainWindow::onConnShouldOpen(){
    if(cdialog!=nullptr){
        cdialog->exec();
    }else {
        cdialog=new ConnDialog(this);
    }
}

void MainWindow::onUseChanAddr(){
    if(!CommChannel::instance()->get_o_mac().isEmpty()){
        ui->lineEdit_7->setText(CommChannel::instance()->get_o_mac());
    }
    if(!CommChannel::instance()->get_mac().isEmpty()){
        ui->lineEdit_6->setText(CommChannel::instance()->get_mac());
    }
}

void MainWindow::onUseChanAddrIP(){
    if(!CommChannel::instance()->get_o_ip().isEmpty()){
        ui->lineEdit_8->setText(CommChannel::instance()->get_o_ip());
    }
    if(!CommChannel::instance()->get_ip().isEmpty()){
        ui->lineEdit_3->setText(CommChannel::instance()->get_ip());
    }
}

void MainWindow::onRX(){
    PDU* pdu=new_PDU(type_ethernet_v2);
    pdu_list.append(pdu);
    QDateTime curDateTime=QDateTime::currentDateTime();
    ui->listWidget->addItem("接收到的包："+curDateTime.toString("yyyy-MM-dd-hh:mm:ss"));
    memcpy(pdu->buffer,CommChannel::instance()->buffer,CommChannel::instance()->buffer.size());


}

void MainWindow::onTX(){
    QString fpath=QFileDialog::getOpenFileName(this,"选择待封帧文件",QDir::homePath(),"上层协议 (*.ipv4 *.ipv6 *.arp *.rarp *.raw)");
    if(fpath.isEmpty()){
        return;
    }
    PDU* pdu=new_PDU(type_ethernet_v2);
    if(!CommChannel::is_mac(ui->lineEdit_6->text())){
        QMessageBox::warning(this,"警告","MAC格式错误");
        return;
    }
    if(!CommChannel::is_mac(ui->lineEdit_7->text())){
        QMessageBox::warning(this,"警告","MAC格式错误");
        return;
    }
    PDU_Load_Type loadtype=load_is_any;
    QStringList sl=fpath.split('.');
    if(sl[sl.count()-1]=="ipv4"){
        loadtype=load_is_ipv4;
    }
    if(sl[sl.count()-1]=="ipv6"){
        loadtype=load_is_ipv6;
    }
    if(sl[sl.count()-1]=="arp"){
        loadtype=load_is_arp;
    }
    if(sl[sl.count()-1]=="rarp"){
        loadtype=load_is_rarp;
    }
    if(sl[sl.count()-1]=="raw"){
        loadtype=load_is_any;
    }
    if(pack_PDU(pdu,fpath.toStdString().c_str(),ui->lineEdit_6->text().toStdString().c_str(),ui->lineEdit_7->text().toStdString().c_str(),loadtype)!=0){
        QMessageBox::warning(this,"警告","PDU创建失败");
        return;
    }
    if(pdu->load_length<PDU_MIN_LOAD_SIZE){
        CommChannel::instance()->tx((const char*)pdu->buffer,PDU_MIN_LOAD_SIZE+26);
    }else{
        CommChannel::instance()->tx((const char*)pdu->buffer,pdu->load_length+26);
    }
    free(pdu);
}

void MainWindow::onShouldParse(){
    int index=ui->listWidget->currentRow();
    if(index<0) return;
    PDU* pdu=pdu_list[index];
    parse_PDU(pdu);
    int tocount=0;
    if(pdu->load_length<PDU_MIN_LOAD_SIZE){
        tocount=PDU_MIN_LOAD_SIZE+26;
    }else{
        tocount=pdu->load_length+26;
    }
    QByteArray ba((const char*)pdu->buffer,tocount);
    ui->plainTextEdit_3->setPlainText(ba.toHex());
    QByteArray bb(((const char*)pdu->buffer)+22,pdu->load_length);
    ui->plainTextEdit_2->setPlainText(bb.toHex());
    ui->lineEdit->setText(pdu->src_mac);
    ui->lineEdit_4->setText(pdu->dst_mac);
    ui->plainTextEdit->clear();
    ui->plainTextEdit->appendPlainText("载荷长度：");
    ui->plainTextEdit->appendPlainText(QString::number(pdu->load_length));
    ui->plainTextEdit->appendPlainText("上层协议：");
    switch (pdu->load_type) {
    case load_is_ipv4:
        ui->plainTextEdit->appendPlainText("ipv4");
        char s[16];
        memset(s,0,16);
        sprintf(s,"%u.%u.%u.%u",pdu->buffer[34],pdu->buffer[35],pdu->buffer[36],pdu->buffer[37]);
        ui->plainTextEdit->appendPlainText("源IP：");
        ui->plainTextEdit->appendPlainText(s);
        sprintf(s,"%u.%u.%u.%u",pdu->buffer[38],pdu->buffer[39],pdu->buffer[40],pdu->buffer[41]);
        ui->plainTextEdit->appendPlainText("矢IP：");
        ui->plainTextEdit->appendPlainText(s);
        break;
    case load_is_ipv6:
        ui->plainTextEdit->appendPlainText("ipv6");
        break;
    case load_is_arp:
        ui->plainTextEdit->appendPlainText("arp");
        break;
    case load_is_rarp:
        ui->plainTextEdit->appendPlainText("rarp");
        break;
    case load_is_any:
        ui->plainTextEdit->appendPlainText("无");
        break;
    }
}
