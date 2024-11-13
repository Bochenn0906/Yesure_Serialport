#include "widget.h"
#include "ui_widget.h"
#include <QStatusBar>
#include <QDebug>

Widget::Widget(QWidget *parent): QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("研煊调试助手");


    // 定时发送-定时器
    timSend = new QTimer;
    timSend->setInterval(1000);// 设置默认定时时长1000ms
    connect(timSend, &QTimer::timeout, this, [=](){
        on_btnSend_clicked();
    });

    // 新建一串口对象
    mySerialPort = new QSerialPort(this);

    // 串口接收，信号槽关联
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(serialPortRead_Slot()));

    // 初始时不添加时间戳标志为 false
    addTimestamp = false;

}

Widget::~Widget()
{
    delete ui;
}




// 打开串口 槽函数
void Widget::on_btnSwitchOn_clicked()
{

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity   checkBits;

    // 获取串口波特率
    //baudRate = ui->cmbBaudRate->currentText().toInt();
    if(ui->comboBox_Baudrate->currentText() == "1200"){
        baudRate = QSerialPort::Baud1200;
    }else if(ui->comboBox_Baudrate->currentText() == "2400"){
        baudRate = QSerialPort::Baud2400;
    }else if(ui->comboBox_Baudrate->currentText() == "4800"){
        baudRate = QSerialPort::Baud4800;
    }else if(ui->comboBox_Baudrate->currentText() == "9600"){
        baudRate = QSerialPort::Baud9600;
    }else if(ui->comboBox_Baudrate->currentText() == "115200"){
        baudRate = QSerialPort::Baud115200;
    }else{

    }

    // 获取串口数据位
    if(ui->comboBox_Databits->currentText() == "5"){
        dataBits = QSerialPort::Data5;
    }else if(ui->comboBox_Databits->currentText() == "6"){
        dataBits = QSerialPort::Data6;
    }else if(ui->comboBox_Databits->currentText() == "7"){
        dataBits = QSerialPort::Data7;
    }else if(ui->comboBox_Databits->currentText() == "8"){
        dataBits = QSerialPort::Data8;
    }else{

    }

    // 获取串口停止位
    if(ui->comboBox_Stopbits->currentText() == "1"){
        stopBits = QSerialPort::OneStop;
    }else if(ui->comboBox_Stopbits->currentText() == "1.5"){
        stopBits = QSerialPort::OneAndHalfStop;
    }else if(ui->comboBox_Stopbits->currentText() == "2"){
        stopBits = QSerialPort::TwoStop;
    }else{

    }

    // 获取串口奇偶校验位
    if(ui->comboBox_Parity->currentText() == "None"){
        checkBits = QSerialPort::NoParity;
    }else if(ui->comboBox_Parity->currentText() == "Odd"){
        checkBits = QSerialPort::OddParity;
    }else if(ui->comboBox_Parity->currentText() == "Even"){
        checkBits = QSerialPort::EvenParity;
    }else{

    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    //mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    // 匹配带有串口设备信息的文本
    QString spTxt = ui->comboBox_Serialport->currentText();
    spTxt = spTxt.section(':', 0, 0);//spTxt.mid(0, spTxt.indexOf(":"));
    //qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if(mySerialPort->open(QIODevice::ReadWrite) == true){
        //QMessageBox::
        // 让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
        ui->comboBox_Serialport->setEnabled(false);
        ui->comboBox_Baudrate->setEnabled(false);
        ui->comboBox_Stopbits->setEnabled(false);
        ui->comboBox_Databits->setEnabled(false);
        ui->comboBox_Parity->setEnabled(false);
    }else{
        QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n该串口可能被占用\r\n请选择正确的串口");
    }
    qDebug() << "open" ;
}

// 关闭串口 槽函数
void Widget::on_btnSwitchOff_clicked()
{
    mySerialPort->close();
    // 端口号下拉框恢复可选，避免误操作
    ui->comboBox_Serialport->setEnabled(true);
    ui->comboBox_Baudrate->setEnabled(true);
    ui->comboBox_Stopbits->setEnabled(true);
    ui->comboBox_Databits->setEnabled(true);
    ui->comboBox_Parity->setEnabled(true);

}

// // 状态栏标签显示计数值
// void Widget::setNumOnLabel(QLabel *lbl, QString strS, long num)
// {
//     // 标签显示
//     QString strN;
//     strN.sprintf("%ld", num);
//     QString str = strS + strN;
//     lbl->setText(str);
// }

void Widget::serialPortRead_Slot()
{
    qDebug() << "触发接收函数" ;
    QByteArray recBuf;
    recBuf = mySerialPort->readAll();

    if (ui->checkRec->checkState() == Qt::Unchecked) {
        // 字符串形式显示
        if (addTimestamp) {
            QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
            QString newText = QString("%1 RX: %2").arg(timeStamp).arg(QString::fromLocal8Bit(recBuf));
            ui->msg_Rec->appendPlainText(newText);
        } else {
            ui->msg_Rec->appendPlainText(QString::fromLocal8Bit(recBuf));
        }
        qDebug() << "字符" << recBuf ;
    } else {
        // 16进制形式显示
        QString str1 = recBuf.toHex().toUpper();
        QString str2;
        for (int i = 0; i < str1.length(); i += 2) {
            str2 += str1.mid(i, 2);
            str2 += " ";
        }
        if (addTimestamp) {
            QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
            QString newText = QString("%1 Rx: %2").arg(timeStamp).arg(str2.trimmed());
            ui->msg_Rec->appendPlainText(newText);
        } else {
            ui->msg_Rec->appendPlainText(str2.trimmed());
        }
        qDebug() << "16 进制" << recBuf;
    }

    // 移动光标到文本结尾
    ui->msg_Rec->moveCursor(QTextCursor::End);
}
// 发送按键槽函数
// 如果勾选16进制发送，按照asc2的16进制发送
void Widget::on_btnSend_clicked()
{
    QByteArray sendData;

    // 判断是否为16进制发送，将发送区全部的asc2转换为16进制字符串显示，发送的时候转换为16进制发送
    if (ui->checkSend->checkState() == Qt::Unchecked) {
        // 字符串形式发送
        sendData = ui->msg_Send->toPlainText().toLocal8Bit();
        qDebug() << "发送字符串成功" << sendData;

    } else {
        // 16进制发送
        sendData = QByteArray::fromHex(ui->msg_Send->toPlainText().toUtf8());
        qDebug() << "发送16进制成功" << sendData.toHex(' ');

    }

    // 如发送成功，会返回发送的字节长度。失败，返回-1。
    int a = mySerialPort->write(sendData);
    // // 发送字节计数并显示
    // if (a > 0) {
    //     // 发送字节计数
    //     sendNum += a;
    //     // 状态栏显示计数值
    //     setNumOnLabel(lblSendNum, "S: ", sendNum);
    // } else {
    //     qDebug() << "发送失败";
    // }

    qDebug() << sendData;
}

void Widget::on_btnClearRec_clicked()
{
    ui->msg_Rec->clear();
    // // 清除发送、接收字节计数
    // sendNum = 0;
    // recvNum = 0;
    // // 状态栏显示计数值
    // setNumOnLabel(lblSendNum, "S: ", sendNum);
    // setNumOnLabel(lblRecvNum, "R: ", recvNum);
}


void Widget::on_btnClearSend_clicked()
{
    ui->msg_Send->clear();
    // // 清除发送字节计数
    // sendNum = 0;
    // // 状态栏显示计数值
    // setNumOnLabel(lblSendNum, "S: ", sendNum);
}



// 先前发送区的部分在多选框状态转换槽函数中进行转换。（最好多选框和发送区组成一个自定义控件，方便以后调用）
void Widget::on_checkSend_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->msg_Send->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，将先前的发送区的16进制字符串转换为asc2字符串
    if(arg1 == 0){

        QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // 文本控件清屏，显示新文本
        ui->msg_Send->clear();
        ui->msg_Send->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->msg_Send->moveCursor(QTextCursor::End);

    }else{// 多选框被勾选，将先前的发送区的asc2字符串转换为16进制字符串

        QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
        // 添加空格
        QByteArray str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->msg_Send->clear();
        ui->msg_Send->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->msg_Send->moveCursor(QTextCursor::End);

    }
}

void Widget::on_checkRec_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->msg_Rec->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    // 为0时，多选框未被勾选，接收区先前接收的16进制数据转换为asc2字符串格式
    if(arg1 == 0){

        QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // 文本控件清屏，显示新文本
        ui->msg_Rec->clear();
        ui->msg_Rec->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->msg_Rec->moveCursor(QTextCursor::End);

    }else{// 不为0时，多选框被勾选，接收区先前接收asc2字符串转换为16进制显示

        QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
        // 添加空格
        QByteArray str2;
        for(int i = 0; i<str1.length (); i+=2)
        {
            str2 += str1.mid (i,2);
            str2 += " ";
        }
        // 文本控件清屏，显示新文本
        ui->msg_Rec->clear();
        ui->msg_Rec->insertPlainText(str2);
        // 移动光标到文本结尾
        ui->msg_Rec->moveCursor(QTextCursor::End);

    }
}

void Widget::on_checkTime_stateChanged(int arg1)
{
    // 设置标志变量
    addTimestamp = (arg1!= 0);
}

void Widget::on_fetch_ID_clicked()
{
    QByteArray fetchid_Command = QByteArray("\x01\x03\x00\x00\x00\x05");
    mySerialPort->write(fetchid_Command);

    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(processIDResponse()));
}

void Widget::processIDResponse()
{
    qDebug() << "触发查询ID" ;
    QByteArray recBuf = mySerialPort->readAll();

    // 判断是否为读取 ID 的响应
    if (recBuf.startsWith(QByteArray("\x01\x03\x02")))
    {
        // 假设响应格式是地址+功能码+字节数+数据，数据为一个字节的 ID
        int id = recBuf[3];
        ui->newID->setPlainText(QString::number(id));
    }
}

void Widget::on_change_ID_clicked()
{
}

