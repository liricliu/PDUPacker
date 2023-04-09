#ifndef COMMCHANNEL_H
#define COMMCHANNEL_H

#include <QObject>
#include <QtNetwork>
#include <QtSerialPort>

enum class ChannelType{
    unset,
    tcp_s,
    tcp_c,
    com
};

class CommChannel : public QObject
{
    Q_OBJECT
public:
    QByteArray buffer;
    QList<QString> comnames;
    static void init(){
        _instance=new CommChannel;
        if(_instance==nullptr){
            exit(-1);
        }
    }
    static CommChannel* instance(){
        return _instance;
    }
    void tx(const char*,int);
    bool connect_chan();
    void set_mac(QString m){
        if(is_mac(m))
            mac=m;
        return;
    }
    void set_ip(QString i){
        if(is_ip(i))
            ip=i;
        return;
    }
    QString get_ip(){
        if(is_ip(ip))
            return ip;
        else
            return QString();
    }
    QString get_mac(){
        if(is_mac(mac))
            return mac;
        else
            return QString();
    }
    QString get_o_ip(){
        if(is_ip(o_ip))
            return o_ip;
        else
            return QString();
    }
    QString get_o_mac(){
        if(is_mac(o_mac))
            return o_mac;
        else
            return QString();
    }
    static bool is_ip(QString s){
        QStringList sl=s.split(".");
        if(sl.count()!=4)
            return false;

        for(int i=0;i<4;i++){
            bool isOk;
            sl[i].toInt(&isOk);
            if(!isOk){
                return false;
            }
        }
        return true;
    }
    static bool is_mac(QString s){
        if(s.size()!=17){
            return false;
        }
        QStringList sl=s.split(":");
        if(sl.count()!=6)
            return false;

        for(int i=0;i<6;i++){
            bool isOk;
            sl[i].toInt(&isOk,16);
            if(!isOk){
                return false;
            }
        }
        return true;
    }
    void set_type(ChannelType t){
        type=t;
    }
    void set_conn_ip(QString s){
        if(is_ip(s)){
            conn_ip=s;
        }
    }
    void set_conn_port(unsigned short i){
        conn_port=i;
    }
    void set_listen_port(unsigned short i){
        listen_port=i;
    }
    void set_com_name(QString s){
        com_name=s;
    }
    QString get_peer_ip(){
        return peer_ip;
    }
    ChannelType get_type(){
        return type;
    }
private:
    explicit CommChannel(QObject *parent = nullptr);
    static CommChannel* _instance;
    QString ip;
    QString o_ip;
    QString mac;
    QString o_mac;
    ChannelType type;
    unsigned short listen_port;
    unsigned short conn_port;
    QString conn_ip;
    QString com_name;
    QTcpServer* mServer;
    QTcpSocket* mSocket;
    QSerialPort* serial;
    QString peer_ip;
signals:
    void connection_ready();
    void disconnected();
    void rx();
public slots:
    void tcp_rx();
    void onDisconn();
    void refreshCOM();


};

#endif // COMMCHANNEL_H
