#ifndef CONNDIALOG_H
#define CONNDIALOG_H

#include <QDialog>

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
private:
    Ui::ConnDialog *ui;
    void accept();
};

#endif // CONNDIALOG_H
