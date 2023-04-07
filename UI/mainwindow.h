#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void onIPV4ShouldGen();
    void onIPV6ShouldGen();

private:
    Ui::MainWindow *ui;

    constexpr static unsigned char DSCP[22]={
        0b00000000,//CS0
        0b00100000,//CS1
        0b01000000,//CS2
        0b01100000,//CS3
        0b10000000,//CS4
        0b10100000,//CS5
        0b11000000,//CS6
        0b11100000,//CS7
        0b00101000,//AF11
        0b00110000,//AF12
        0b00111000,//AF13
        0b01001000,//AF21
        0b01010000,//AF22
        0b01011000,//AF23
        0b01101000,//AF31
        0b01110000,//AF32
        0b01111000,//AF33
        0b10001000,//AF41
        0b10010000,//AF42
        0b10011000,//AF43
        0b10111000,//EF PHB
        0b10110000 //VOICE-ADMIT
    };
    constexpr static unsigned char ECN[4]={
        0b10,0b01,0b11,0b00
    };
};

#endif // MAINWINDOW_H
