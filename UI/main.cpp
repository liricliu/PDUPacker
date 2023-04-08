#include "mainwindow.h"
#include "PDU/commchannel.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    CommChannel::init();
    MainWindow w;
    w.show();

    return a.exec();
}
