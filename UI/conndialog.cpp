#include "conndialog.h"
#include "ui_conndialog.h"

ConnDialog::ConnDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnDialog)
{
    ui->setupUi(this);
    connect(ui->comboBox,SIGNAL(currentIndexChanged(int)),this,SLOT(setChann(int)));
}

ConnDialog::~ConnDialog()
{
    delete ui;
}

void ConnDialog::accept(){

}

void ConnDialog::setChann(int index){
    ui->stackedWidget->setCurrentIndex(index);
}
