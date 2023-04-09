#include "commchannel.h"
#include <QMessageBox>

CommChannel* CommChannel::_instance;

CommChannel::CommChannel(QObject *parent) : QObject(parent)
{
    listen_port=0;
    type=ChannelType::unset;
    mServer = new QTcpServer(this);
    serial = new QSerialPort;
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
        {
            serial->setPort(info);                      // 在对象中设置串口
            if(serial->open(QIODevice::ReadWrite))      // 以读写方式打开串口
            {
                comnames.append(info.portName());  // 添加计算机中的端口
                serial->close();                        // 关闭
            } else
            {
                qDebug() << "串口打开失败，请重试";
            }
        }
}

bool CommChannel::connect_chan(){
    switch (type) {
    case ChannelType::unset:
        return false;
    case ChannelType::tcp_c:
        mSocket = new QTcpSocket(this);
        mSocket->connectToHost(conn_ip,conn_port);
        QObject::connect(mSocket,&QTcpSocket::connected,this,[&](){
            peer_ip=conn_ip;
            connect(mSocket,&QTcpSocket::readyRead,this,&CommChannel::tcp_rx);
            QByteArray totx=QByteArray();
            totx.append('\x00');
            totx.append(ip);
            totx.append('\x00');
            totx.append(mac);
            totx.append('\x00');
            tx(totx.data(),totx.size());
            QMessageBox::information(nullptr,"提示","连接成功");
        });
        connect(mSocket,&QTcpSocket::disconnected,this,&CommChannel::onDisconn);
        break;
    case ChannelType::tcp_s:
        if(mServer->isListening()){
            QMessageBox::warning(nullptr,"警告","已经在监听状态");
            return false;
        }
        if(!mServer->listen(QHostAddress::Any,listen_port)){
            QMessageBox::warning(nullptr,"警告","该端口已被占用");
            return false;
        }
        connect(mServer,&QTcpServer::newConnection,this,[&](){
            mSocket = mServer->nextPendingConnection();
            QHostAddress c_addr = mSocket->peerAddress();
            peer_ip=c_addr.toString();
            connect(mSocket,&QTcpSocket::readyRead,this,&CommChannel::tcp_rx);
            QByteArray totx=QByteArray();
            totx.append('\x00');
            totx.append(ip);
            totx.append('\x00');
            totx.append(mac);
            totx.append('\x00');
            tx(totx.data(),totx.size());
            connect(mSocket,&QTcpSocket::disconnected,this,&CommChannel::onDisconn);
            QMessageBox::information(nullptr,"提示","有新的连接");
        });
        break;
    case ChannelType::com:
            serial->setBaudRate(QSerialPort::Baud115200);
            serial->setParity(QSerialPort::NoParity);
            serial->setDataBits(QSerialPort::Data8);
            serial->setStopBits(QSerialPort::OneStop);
            serial->setFlowControl(QSerialPort::NoFlowControl);
            serial->setPortName(com_name);
            if(!serial->open(QIODevice::ReadWrite)){
                QByteArray totx=QByteArray();
                totx.append('\x00');
                totx.append(ip);
                totx.append('\x00');
                totx.append(mac);
                totx.append('\x00');
                tx(totx.data(),totx.size());
                QMessageBox::warning(nullptr,"警告","COM口打开失败或已经被打开");
                return false;
            }else{
                connect(serial,&QSerialPort::readyRead,this,&CommChannel::tcp_rx);
                QByteArray totx=QByteArray();
                totx.append('\x00');
                totx.append(ip);
                totx.append('\x00');
                totx.append(mac);
                totx.append('\x00');
                tx(totx.data(),totx.size());
            }
        break;
    }
    return true;
}

void CommChannel::tx(const char* cc,int length){
    switch (type) {
    case ChannelType::tcp_c:
    case ChannelType::tcp_s:
        if(!mSocket->isValid()){
            QMessageBox::warning(nullptr,"警告","信道尚未连接");
            return;
        }
        mSocket->write(cc,length);
        break;
    case ChannelType::com:
        if(!serial->isWritable()){
            QMessageBox::warning(nullptr,"警告","信道尚未连接");
            return;
        }
        serial->write(cc,length);
        break;
    case ChannelType::unset:
        QMessageBox::warning(nullptr,"警告","信道尚未连接");
        break;
    }

}

void CommChannel::tcp_rx(){
    if(type==ChannelType::com){
        buffer=serial->readAll();
    }else{
        buffer = mSocket->readAll();
    }
    //给包分类，如果是以太网帧则emit rx，否则emit connection_ready
    if((buffer.data())[0]=='\xAA'){
        emit rx();
    }else {
        if((buffer.data())[0]=='\x00'){
            o_ip=QString();
            o_mac=QString();
            int i;
            for(i=1;((buffer.data())[i])!='\x00';i++){
                o_ip.append((buffer.data())[i]);
            }
            i++;
            for(;((buffer.data())[i])!='\x00';i++){
                o_mac.append((buffer.data())[i]);
            }
            emit connection_ready();
        }
    }
}

void CommChannel::onDisconn(){
    emit disconnected();
}

void CommChannel::refreshCOM(){
    comnames.clear();
    foreach(const QSerialPortInfo &info,QSerialPortInfo::availablePorts())
        {
            serial->setPort(info);                      // 在对象中设置串口
            if(serial->open(QIODevice::ReadWrite))      // 以读写方式打开串口
            {
                comnames.append(info.portName());  // 添加计算机中的端口
                serial->close();                        // 关闭
            } else
            {
                qDebug() << "串口打开失败，请重试";
            }
        }
}
