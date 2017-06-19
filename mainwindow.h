#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore/QtGlobal>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QIntValidator>
#include <QLineEdit>
#include <QLabel>
#include "crctools.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public:
    struct Settings {
        QString name;
        qint32 baudRate;
        QString stringBaudRate;
        QSerialPort::DataBits dataBits;
        QString stringDataBits;
        QSerialPort::Parity parity;
        QString stringParity;
        QSerialPort::StopBits stopBits;
        QString stringStopBits;
        QSerialPort::FlowControl flowControl;
        QString stringFlowControl;
        bool localEchoEnabled;
        bool SerialStatus;      //0关闭 ，1打开
    };

private slots:

//    void checkCustomBaudRatePolicy(int idx);
//    void checkCustomDevicePathPolicy(int idx);


    void on_serialOpenBut_clicked();

    void on_sendBut_clicked();

    void on_saveBut_clicked();

    void on_clearBut_clicked();

    void on_lineEdit_editingFinished();

    void on_lineEdit_returnPressed();

    void readData();

    void on_refSerial_triggered();

    void on_HexSend_clicked();

private:
    Ui::MainWindow *ui;
    Settings currentSettings;
    QSerialPort *serial;
    QLabel *status;
    qint32 SendCounter,ReceiveCounter;
    crcTools *crc;
    void fillPortsInfo();
    void fillSerialPortParamBox();
    void showStatusMessage(const QString &message);

public:
    void StringToHex(QString str, QByteArray &senddata); //字符串转换为十六进制数据0-F
    char ConvertHexChar(char ch);
};

#endif // MAINWINDOW_H
