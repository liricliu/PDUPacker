#ifndef CONNDIALOG_H
#define CONNDIALOG_H

#include <QDialog>
#include "PDU/commchannel.h"

namespace Ui {
class ConnDialog;
}

class ConnDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConnDialog(QWidget *parent = nullptr);
    ~ConnDialog();

public slots:
    void setChann(int index);
    void connectClicked();
    void randIP();
    void randMAC();
    void localIP();
    void localMAC();
    void onConnReady();
    void onConnDown();
    void refreshCOM();
    void setCOM(int index);

private:
    Ui::ConnDialog *ui;
    void accept();
};

#endif // CONNDIALOG_H
