#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QTextStream>
#include "crctools.h"
#define user_debug
#ifdef user_debug
#include <QDebug>
#endif
QT_USE_NAMESPACE
static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDialog", "N/A");
QString str2HexStr(QString str);

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fillPortsInfo();
    fillSerialPortParamBox();
    crc=new crcTools;
    SendCounter         = 0;
    ReceiveCounter      = 0;
    ui->SendNum->setText(QString::number(SendCounter));
    ui->RecNum->setText(QString::number(ReceiveCounter));
    currentSettings.SerialStatus=0;
//!     [1]

    serial = new QSerialPort(this);
    status = new QLabel;
    ui->statusBar->addWidget(status);
    ui->serialRecText->setReadOnly(true);
    ui->CrcBox->setDisabled(true);
    status->setText(tr("Closed"));


//! [2]
//    ui->serialRecText->insertPlainText(tr("hello world --insert by init"));

//! [3]
    connect(serial, &QSerialPort::readyRead, this, &MainWindow::readData);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::fillPortsInfo()
{
    ui->serialPortInfoBox->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);
        ui->serialPortInfoBox->addItem(list.first(), list);
    }

    ui->serialPortInfoBox->addItem(tr("Custom"));
}

void MainWindow::fillSerialPortParamBox()
{
    ui->baudRateBox->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox->addItem(tr("Custom"));

    ui->dataBitBox->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitBox->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitBox->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitBox->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitBox->setCurrentIndex(3);

    ui->parityBox->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox->addItem(tr("Space"), QSerialPort::SpaceParity);

    ui->stopBitBox->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitBox->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitBox->addItem(QStringLiteral("2"), QSerialPort::TwoStop);

    ui->flowCtrlBox->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowCtrlBox->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowCtrlBox->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
}

///*
// * 自定义波特率
// *
// *
// */
//void checkCustomBaudRatePolicy(int idx)
//{


//}

///*
// * 自定义设备
// *
// *
// *
// */
//void checkCustomDevicePathPolicy(int idx)
//{


//}

/*
 * 打开 or 关闭  串口
 *
 *
 *
 */
void MainWindow::on_serialOpenBut_clicked()
{
    //1、获取串口信息
    currentSettings.name = ui->serialPortInfoBox->currentText();

    if(ui->baudRateBox->currentIndex()==4){
        currentSettings.baudRate = ui->baudRateBox->currentText().toInt();
    }else{
        currentSettings.baudRate = static_cast<QSerialPort::BaudRate>(ui->baudRateBox->itemData(ui->baudRateBox->currentIndex()).toInt());
    }

    currentSettings.stringBaudRate = QString::number(currentSettings.baudRate);

    currentSettings.dataBits = static_cast<QSerialPort::DataBits>(
                ui->dataBitBox->itemData(ui->dataBitBox->currentIndex()).toInt());
    currentSettings.stringDataBits = ui->dataBitBox->currentText();

    currentSettings.parity = static_cast<QSerialPort::Parity>(
                ui->parityBox->itemData(ui->parityBox->currentIndex()).toInt());
    currentSettings.stringParity = ui->parityBox->currentText();

    currentSettings.stopBits = static_cast<QSerialPort::StopBits>(
                ui->stopBitBox->itemData(ui->stopBitBox->currentIndex()).toInt());
    currentSettings.stringStopBits = ui->stopBitBox->currentText();

    currentSettings.flowControl = static_cast<QSerialPort::FlowControl>(
                ui->flowCtrlBox->itemData(ui->flowCtrlBox->currentIndex()).toInt());
    currentSettings.stringFlowControl = ui->flowCtrlBox->currentText();

    if(!currentSettings.SerialStatus){
        //打开设备
        serial->setPortName(currentSettings.name);
        serial->setBaudRate(currentSettings.baudRate);
        serial->setDataBits(currentSettings.dataBits);
        serial->setParity(currentSettings.parity);
        serial->setStopBits(currentSettings.stopBits);
        serial->setFlowControl(currentSettings.flowControl);
        if (serial->open(QIODevice::ReadWrite)) {
            //打开串口成功
            currentSettings.SerialStatus=1;
            ui->serialOpenBut->setText(tr("关闭"));
 //           ui->serialStatusLabel->setText(tr("open"));
            status->setText(tr("Opened"));
        } else {
            QMessageBox::critical(this, tr("Error"), serial->errorString());

            showStatusMessage(tr("Open error"));
        }
    }else{
        //关闭设备
        if (serial->isOpen())
            serial->close();

        ui->serialOpenBut->setText(tr("打开"));
//        ui->serialStatusLabel->setText(tr("open"));
        status->setText(tr("Closed"));
        currentSettings.SerialStatus=0;
    }

}

void MainWindow::showStatusMessage(const QString &message)
{
    status->setText(message);
}


/*
 * 发送字符
 *
 *
 */
char ConverCharHex(char hex);
void MainWindow::on_sendBut_clicked()
{
    if(!serial->isOpen()){
        //发出警告
        QMessageBox::critical(this, tr("Error"),tr("串口未打开"));
        return ;
    }
    if(ui->HexSend->isChecked()){
        //hex发送方式

        QString SendStr;
        //1、去掉空格分割符
        SendStr=ui->lineEdit->text().remove(QChar(' '),Qt::CaseInsensitive);
        //2、识别hex码
        if (SendStr.length()%2){
            SendStr.insert(SendStr.length()-1,"0");
        }
        QByteArray sendData;
        StringToHex(SendStr,sendData);
#ifdef user_debug
        qDebug()<<sendData;
#endif
        QByteArray crcArry;
        if(ui->CrcBox->isChecked()){
            //向数据末尾添加CRC校验值,且crc模块仅在hex下生效,用于modbus等需要crc校验的场合.
            uint16_t crcval=crc->ucMBCRC16(sendData.data(),sendData.length());
            //将crc值写入
            uint8_t valH=(crcval>>8)&0xFF;
            uint8_t valL=(crcval)&0xFF;
            crcArry[0]=ConverCharHex((valL>>4)&0x0F);
            crcArry[1]=ConverCharHex(valL&0x0F);
            crcArry[2]=' ';
            crcArry[3]=ConverCharHex((valH>>4)&0x0F);
            crcArry[4]=ConverCharHex(valH&0x0F);
            qDebug("0x%x,0x%x",valH,valL);
            ui->crcDisplay->setText(QString(crcArry));
            qDebug()<<crcArry;
        }
        SendCounter+=sendData.length();
        if(ui->lrAdd->isChecked())
        {
            serial->write(sendData.append("\n"));
        }else{
            serial->write(sendData);
        }
        if(ui->echoBox->isChecked()){
            //将发送数据回显到显示窗口;
            //分离字符串
           QString str=QString(sendData.toHex());
           uint strlen=str.length();
#ifdef user_debug
           qDebug()<<strlen;
#endif
           for(int i=strlen;i>1;)
           {
              str.insert(i,' ');
              i-=2;
           }
           //ui->serialRecText->insertPlainText(QString(data.toHex()));
           ui->serialRecText->insertPlainText(str.append(crcArry));
        }
    }else{
        //非Hex发送
        SendCounter+=ui->lineEdit->text().length();
        if(ui->lrAdd->isChecked())
        {
            serial->write((ui->lineEdit->text().append("\n")).toLocal8Bit());
        }else{
            serial->write(ui->lineEdit->text().toLocal8Bit());
        }
        if(ui->echoBox->isChecked()){
            //将发送数据回显到显示窗口;
            ui->serialRecText->insertPlainText(ui->lineEdit->text().append("\n"));
        }
    }

    // ui->serialRecText->insertPlainText("\n");
    ui->SendNum->setText(QString::number(SendCounter));
}


/*
 *保存显示框的内容
 *
 *
 */
void MainWindow::on_saveBut_clicked()
{
    /* 保存内容*/
    QString fileName;
    fileName=QFileDialog::getOpenFileName(this,
                                          tr("选择文件"),
                                          "",
                                          tr("textFile(*.txt *.log)"));
    if(fileName.isEmpty()){
        return ;    //用户取消保存
    }else{
        QFile file(fileName);
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text|QIODevice::Unbuffered)){
#ifdef user_debug
            qDebug()<<"open file failed!!";
#endif
            return ;
        }
#ifdef user_debug
        QFileInfo info(file);
        qDebug() << info.isDir();
        qDebug() << info.isExecutable();
        qDebug() << info.baseName();
        qDebug() << info.completeBaseName();
        qDebug() << info.suffix();
        qDebug() << info.completeSuffix();
#endif
        //向文件写入内容
        QTextStream  out(&file);
//        out<<ui->serialRecText->toPlainText().remove(QChar('\0'),Qt::CaseInsensitive);
        out<<ui->serialRecText->toPlainText();
        file.flush();
        file.close();
#ifdef user_debug
        qDebug()<<"file save complete";
#endif
        //弹出串口
        QMessageBox msgBox;
        msgBox.setText("The document has been saved.");
        msgBox.exec();
    }



}


/*
 *清空显示框
 *
 *
 *
 */
void MainWindow::on_clearBut_clicked()
{
    ui->serialRecText->document()->setPlainText("");
    this->ReceiveCounter=0;
    this->SendCounter=0;
    ui->SendNum->setText(QString::number(SendCounter));
    ui->RecNum->setText(QString::number(ReceiveCounter));
}


/*
 *发送信息编辑完成
 *
 *
 *
 */
void MainWindow::on_lineEdit_editingFinished()
{

}

/*
 * 回车按下
 *
 *
 *
 */
void MainWindow::on_lineEdit_returnPressed()
{
    on_sendBut_clicked();
}


/*
 *读取串口数据的信号槽
 *
 *
 *
 */
void MainWindow::readData()
{
    QByteArray data = serial->readAll();
    ReceiveCounter+=data.length();
    if(ui->HexDisplay->isChecked())
    {
        //分离字符串
       QString str=QString(data.toHex());
       uint strlen=str.length();
       for(uint i=strlen;i>1;)
       {
          str.insert(i,' ');
          i-=2;
       }
       //ui->serialRecText->insertPlainText(QString(data.toHex()));
       ui->serialRecText->insertPlainText(str);
    }else{
       ui->serialRecText->insertPlainText(QString(data));
    }
    //行数统计
    {
        int lineCount=ui->serialRecText->document()->blockCount();          //获取行数
    #define MaximumLineNum      500
        if(lineCount>MaximumLineNum){
 qDebug()<<"500 line clean start"<<endl;
            //删除前边的多余行
//              QTextCursor txCur=ui->serialRecText->textCursor();
//              for(int i=lineCount-MaximumLineNum;i>0;i++){
//                  txCur.select(QTextCursor::BlockUnderCursor);
//                  txCur.removeSelectedText();
//              }
            QTextCursor txCur=ui->serialRecText->textCursor();
            txCur.setPosition(0);
            for(int det=lineCount-MaximumLineNum;det;det--){
                txCur.movePosition(QTextCursor::NextBlock,QTextCursor::KeepAnchor);
            }
            txCur.removeSelectedText();
        }
    }
     QTextCursor txCur=ui->serialRecText->textCursor();
     txCur.movePosition(QTextCursor::End,QTextCursor::MoveAnchor);
     ui->serialRecText->setTextCursor(txCur);
     ui->RecNum->setText(QString::number(ReceiveCounter));
}



void MainWindow::on_refSerial_triggered()
{
    fillPortsInfo();
}



void MainWindow::StringToHex(QString str, QByteArray &senddata) //字符串转换为十六进制数据0-F
{
    int hexdata,lowhexdata;
    int hexdatalen = 0;
    int len = str.length();
    senddata.resize(len / 2);
    char lstr,hstr;
    for (int i = 0; i < len; ) {
        hstr = str[i].toLatin1();
        if (hstr == ' ') {
            ++i;
            continue;
        }
        ++i;
        if (i  >= len) break;
        lstr = str[i].toLatin1();
        hexdata = ConvertHexChar(hstr);
        lowhexdata = ConvertHexChar(lstr);
        if ((hexdata == 16) || (lowhexdata == 16))
            break;
        else
            hexdata = hexdata*16 + lowhexdata;
        ++i;
        senddata[hexdatalen] = (char)hexdata;
        ++hexdatalen;
    }
    senddata.resize(hexdatalen);
}

char MainWindow::ConvertHexChar(char ch)
{
    if ((ch >= '0') && (ch <= '9'))
        return ch - 0x30;
    else if ((ch >= 'A') && (ch <= 'F'))
        return ch - 'A' + 10;
    else if ((ch >= 'a') && (ch <= 'f'))
        return ch - 'a' + 10;
    else return ch -  ch;
}

char ConverCharHex(char hex)
{
    if((hex>=0)&&(hex<=9)){
        return hex+'0';
    }else{
        return hex-10+'a';
    }


}

void MainWindow::on_HexSend_clicked()
{
    if(ui->HexSend->isChecked()){
        ui->CrcBox->setDisabled(false);
    }else{
        ui->CrcBox->setChecked(false);
        ui->CrcBox->setDisabled(true);
    }
}
