#include "widget.h"
#include "ui_widget.h"
#include <QStatusBar>
#include <QDebug>

#define PADDING 5

Widget::Widget(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    setWindowTitle("研煊调试助手");

    // // 定时发送-定时器
    // timSend = new QTimer;
    // timSend->setInterval(1000); // 设置默认定时时长1000ms
    // connect(timSend, &QTimer::timeout, this, [=]()
    //         { on_btnSend_clicked(); });

    mySerialPort = new QSerialPort(this);

    // 串口接收，信号槽关联
    connect(mySerialPort, SIGNAL(readyRead()), this, SLOT(serialPortRead_Slot()));

    addTimestamp = false;

    // this->setStyleSheet("background: #FFF6E9");

    // this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    // isLeftPressed = false;
    // this->setMouseTracking(true);

    model = new QStandardItemModel(this);
    ui->tableview_Sensor->setModel(model);
}

Widget::~Widget()
{
    if (timer) {
        delete timer;
    }
    delete ui;
}

// void Widget::mousePressEvent(QMouseEvent *event)
// {
//     switch ( event->button()){
//     case Qt::RightButton:
//         this->close();
//         break;
//     case Qt::LeftButton:
//         isLeftPressed = true;
//         mouseOffset = event->globalPos() - this->frameGeometry().topLeft();
//     }
// }
// void Widget::mouseMoveEvent(QMouseEvent *event)
// {
//     QPoint globalPos = event->globalPos();
//     if(!isLeftPressed){
//         this->setCursorShape(globalPos);
//         return;
//     }

//     if(location == CENTER){
//         move(globalPos-mouseOffset);
//         event->accept();
//         return;
//     }

// }
// void Widget::mouseReleaseEvent(QMouseEvent *event)
// {
//     if(event->button() == Qt::LeftButton) {
//         isLeftPressed = false;

//     }

// }
// void Widget::setCursorShape(const QPoint& point)
// {
//     QRect rect = this->rect();
//     QPoint topLeft = mapToGlobal(rect.topLeft());
//     QPoint bottomRight = mapToGlobal(rect.bottomRight());

//     int x = point.x();
//     int y = point.y();

//     if(x >= topLeft.x() && x <= topLeft.x() + PADDING && y >= topLeft.y() && y <= topLeft.y() + PADDING)
//     {
//         // 判断是否在左上角区域
//         location = TOP_LEFT;
//         this->setCursor(QCursor(Qt::SizeFDiagCursor));
//     }
//     else if(x <= bottomRight.x() && x >= bottomRight.x() - PADDING && y <= bottomRight.y() && y >= bottomRight.y() - PADDING)
//     {
//         // 判断是否在右下角区域
//         location = BOTTOM_RIGHT;
//         this->setCursor(QCursor(Qt::SizeFDiagCursor));
//     }
//     else if(x >= topLeft.x() && x <= topLeft.x() + PADDING && y <= bottomRight.y() && y >= bottomRight.y() - PADDING)
//     {
//         // 判断是否在左下角区域
//         location = BOTTOM_LEFT;
//         this->setCursor(QCursor(Qt::SizeBDiagCursor));
//     }
//     else if(x <= bottomRight.x() && x >= bottomRight.x() - PADDING && y >= topLeft.y() && y <= topLeft.y() + PADDING)
//     {
//         // 判断是否在右上角区域
//         location = TOP_RIGHT;
//         this->setCursor(QCursor(Qt::SizeBDiagCursor));
//     }
//     else if(x >= topLeft.x() && x <= topLeft.x() + PADDING)
//     {
//         // 判断是否在左边区域
//         location = LEFT;
//         this->setCursor(QCursor(Qt::SizeHorCursor));
//     }
//     else if(x <= bottomRight.x() && x >= bottomRight.x() - PADDING)
//     {
//         // 判断是否在右边区域
//         location = RIGHT;
//         this->setCursor(QCursor(Qt::SizeHorCursor));
//     }
//     else if(y < bottomRight.y() && y >= bottomRight.y() - PADDING)
//     {
//         // 判断是否在下边区域
//         location = BOTTOM;
//         this->setCursor(QCursor(Qt::SizeVerCursor));
//     }
//     else if(y >= topLeft.y() && y <= topLeft.y() + PADDING)
//     {
//         // 判断是否在上边区域
//         location = TOP;
//         this->setCursor(QCursor(Qt::SizeVerCursor));
//     }
//     else
//     {
//         // 如果都不满足，则为中心区域
//         location = CENTER;
//         this->setCursor(QCursor(Qt::ArrowCursor));
//     }
// }

QString Widget::intToHexWithPadding(int value, int padding)
{
    QString hexValue = QString::number(value, 16).toUpper();
    while (hexValue.length() < padding)
    {
        hexValue = "0" + hexValue;
    }
    return hexValue;
}
unsigned short Widget::crc16(unsigned char *CmmBuf, unsigned char Len)
{
    // qDebug() << "data:" << CmmBuf;
    // qDebug() << "length:" << Len;

    unsigned short crc = 0xFFFF;
    unsigned char i;
    unsigned char j;
    // if(Len > 78) return 1;
    if (Len > 200)
        return 1;
    j = 0;
    while (j < Len)
    {
        crc ^= CmmBuf[j];
        for (i = 0; i < 8; i++)
        {
            if (crc & 0x01)
            {
                crc >>= 1;
                crc ^= 0xA001;
            }
            else
            {
                crc >>= 1;
            }
        }
        j++;
    }

    return crc;
}

/*------------------------------
     * 检测crc是否正确
     ------------------------------*/
bool Widget::crc_Checking(QString message)
{
    // 将获取到底报文两个两个以空格分开
    QString strDis, temp_st;
    int data_frame_bit = 0;                       // 数据帧到位数
    for (int i = 0; i < message.length(); i += 2) // 填加空格
    {
        temp_st = message.mid(i, 2);
        strDis += temp_st;
        strDis += " ";
        data_frame_bit++;
    }
    if (data_frame_bit < 5)
    {
        return false;
    }
    // 将获取到底字符串报文，转成QByteArray格式，并用空格分隔获取到有多少个参数i
    QStringList strlist = strDis.split(" ");
    int data_frame_bit_crc = data_frame_bit - 2;
    unsigned char crc_str[data_frame_bit_crc];
    QString crc_str_accept = "";         // 接收到的crc校验码
    QString crc_str_calculated = "";     // 计算出来的crc校验码大写
    QString crc_str_calculated_low = ""; // 计算出来的crc校验码小写
    for (int i = 0; i < data_frame_bit - 2; i++)
    {
        // 将数据帧接收到到crc检验吗去掉
        crc_str[i] = strlist.at(i).toInt(0, 16);
    }

    // 计算出crc16校验码
    int crc_reaout_int = this->crc16(crc_str, data_frame_bit - 2);
    crc_str_calculated = QString("%1").arg(crc_reaout_int, 4, 16, QChar('0')).toUpper();     // 窗口文件都是大写，所以计算出到字符串字母也要大写。
    crc_str_calculated_low = QString("%1").arg(crc_reaout_int, 4, 16, QChar('0')).toLower(); // 小写
    // 由于编号是0，1，2，3...所以重新拼接到crc校验吗应该从len-1开始，循环两次
    for (int i = data_frame_bit - 1; i > data_frame_bit - 3; i--)
    {
        // 重新拼接接收到到crc16校验码
        crc_str_accept += strlist.at(i);
    }
    // qDebug() << crc_str_accept << "-------" << crc_str_calculated;
    // 判断校验码是否正确，正确返回true,
    if (crc_str_calculated == crc_str_accept || crc_str_calculated_low == crc_str_accept)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*------------------------------
     * 计算出crc的直
     * 将crc拼接到message后面
     ------------------------------*/
QString Widget::crcCalculation(QString message)
{

    /*
     * 进来现将空格去掉
     * 进来不管有没有空格，先去掉，再按找下面标准执行。
     *
     */
    message = message.replace(" ", "");
    /*
     * 这边传过来到没有crc校验，所以总长度不要减去2
     */
    // 将获取到底报文两个两个以空格分开
    QString strDis, temp_st;
    int data_frame_bit = 0;                       // 数据帧到位数
    for (int i = 0; i < message.length(); i += 2) // 填加空格
    {
        temp_st = message.mid(i, 2);
        strDis += temp_st;
        strDis += " ";
        data_frame_bit++;
    }
    if (data_frame_bit < 5)
    {
        return "false";
    }
    // 将获取到底字符串报文，转成QByteArray格式，并用空格分隔获取到有多少个参数i
    QStringList strlist = strDis.split(" ");
    int data_frame_bit_crc = data_frame_bit;
    unsigned char crc_str[data_frame_bit_crc];

    QString crc_str_calculated_low = ""; // 计算出来的crc校验码小写
    for (int i = 0; i < data_frame_bit; i++)
    {
        // 将数据帧存放数组
        crc_str[i] = strlist.at(i).toInt(0, 16);
    }

    // 计算出crc16校验码
    int crc_reaout_int = this->crc16(crc_str, data_frame_bit);
    crc_str_calculated_low = QString("%1").arg(crc_reaout_int, 4, 16, QChar('0')).toLower(); // 小写

    QString strDis_crc;
    for (int i = 0; i < crc_str_calculated_low.length(); i += 2) // 填加空格
    {
        temp_st = crc_str_calculated_low.mid(i, 2);
        strDis_crc += temp_st;
        strDis_crc += " ";
    }
    QStringList strlist_crc = strDis_crc.split(" ");
    crc_str_calculated_low = strlist_crc.at(1) + strlist_crc.at(0);
    // qDebug() << "lo hi :" << crc_str_calculated_low ;


    message = message + crc_str_calculated_low;

    return message.toUpper();
}

QString Widget::readRequestWithCRC(QString address, int function, int REG, int quality)
{
    QString request = address.append(intToHexWithPadding(function, 2)).append(intToHexWithPadding(REG, 4)).append(intToHexWithPadding(quality, 4));
    return crcCalculation(request);
}

QString Widget::ushortToHex(unsigned short value)
{
    return QString("%1").arg(value, 4, 16, QChar('0')).toUpper();
}

float Widget::hexStringToFloat(const QString &hexStr)
{
    bool ok;
    quint32 value = hexStr.toUInt(&ok, 16);
    if (!ok)
    {
        qDebug() << "Conversion failed for hex string:" << hexStr;
        return 0.0f;
    }
    return *(reinterpret_cast<float *>(&value));
}

QString Widget::floatToHexString(float f)
{
    union
    {
        float fValue;
        unsigned int uValue;
    } converter;
    converter.fValue = f;
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << converter.uValue;
    return QString::fromStdString(ss.str());
}

float Widget::stringToFloat(const QString &str)
{
    bool ok;
    float value = str.toFloat(&ok);
    if (!ok)
    {
        qDebug() << "Conversion failed for string:" << str;
        return 0.0f;
    }
    return value;
}

unsigned short stringToUShort(const QString &str)
{
    bool ok;
    unsigned short value = str.toUShort(&ok);
    if (!ok)
    {
        qDebug() << "Conversion failed for string:" << str;
        return 0;
    }
    return value;
}

// 打开串口 槽函数
void Widget::on_btnSwitchOn_clicked()
{

    QSerialPort::BaudRate baudRate;
    QSerialPort::DataBits dataBits;
    QSerialPort::StopBits stopBits;
    QSerialPort::Parity checkBits;

    // 获取串口波特率
    // baudRate = ui->cmbBaudRate->currentText().toInt();
    if (ui->comboBox_Baudrate->currentText() == "1200")
    {
        baudRate = QSerialPort::Baud1200;
    }
    else if (ui->comboBox_Baudrate->currentText() == "2400")
    {
        baudRate = QSerialPort::Baud2400;
    }
    else if (ui->comboBox_Baudrate->currentText() == "4800")
    {
        baudRate = QSerialPort::Baud4800;
    }
    else if (ui->comboBox_Baudrate->currentText() == "9600")
    {
        baudRate = QSerialPort::Baud9600;
    }
    else if (ui->comboBox_Baudrate->currentText() == "115200")
    {
        baudRate = QSerialPort::Baud115200;
    }
    else
    {
    }

    // 获取串口数据位
    if (ui->comboBox_Databits->currentText() == "5")
    {
        dataBits = QSerialPort::Data5;
    }
    else if (ui->comboBox_Databits->currentText() == "6")
    {
        dataBits = QSerialPort::Data6;
    }
    else if (ui->comboBox_Databits->currentText() == "7")
    {
        dataBits = QSerialPort::Data7;
    }
    else if (ui->comboBox_Databits->currentText() == "8")
    {
        dataBits = QSerialPort::Data8;
    }
    else
    {
    }

    // 获取串口停止位
    if (ui->comboBox_Stopbits->currentText() == "1")
    {
        stopBits = QSerialPort::OneStop;
    }
    else if (ui->comboBox_Stopbits->currentText() == "1.5")
    {
        stopBits = QSerialPort::OneAndHalfStop;
    }
    else if (ui->comboBox_Stopbits->currentText() == "2")
    {
        stopBits = QSerialPort::TwoStop;
    }
    else
    {
    }

    // 获取串口奇偶校验位
    if (ui->comboBox_Parity->currentText() == "None")
    {
        checkBits = QSerialPort::NoParity;
    }
    else if (ui->comboBox_Parity->currentText() == "Odd")
    {
        checkBits = QSerialPort::OddParity;
    }
    else if (ui->comboBox_Parity->currentText() == "Even")
    {
        checkBits = QSerialPort::EvenParity;
    }
    else
    {
    }

    // 想想用 substr strchr怎么从带有信息的字符串中提前串口号字符串
    // 初始化串口属性，设置 端口号、波特率、数据位、停止位、奇偶校验位数
    mySerialPort->setBaudRate(baudRate);
    mySerialPort->setDataBits(dataBits);
    mySerialPort->setStopBits(stopBits);
    mySerialPort->setParity(checkBits);
    // mySerialPort->setPortName(ui->cmbSerialPort->currentText());// 不匹配带有串口设备信息的文本
    //  匹配带有串口设备信息的文本
    QString spTxt = ui->comboBox_Serialport->currentText();
    spTxt = spTxt.section(':', 0, 0); // spTxt.mid(0, spTxt.indexOf(":"));
    // qDebug() << spTxt;
    mySerialPort->setPortName(spTxt);

    // 根据初始化好的串口属性，打开串口
    // 如果打开成功，反转打开按钮显示和功能。打开失败，无变化，并且弹出错误对话框。
    if (mySerialPort->open(QIODevice::ReadWrite) == true)
    {
        // QMessageBox::
        //  让端口号下拉框不可选，避免误操作（选择功能不可用，控件背景为灰色）
        ui->comboBox_Serialport->setEnabled(false);
        ui->comboBox_Baudrate->setEnabled(false);
        ui->comboBox_Stopbits->setEnabled(false);
        ui->comboBox_Databits->setEnabled(false);
        ui->comboBox_Parity->setEnabled(false);
    }
    else
    {
        QMessageBox::critical(this, "错误提示", "串口打开失败！！！\r\n该串口可能被占用\r\n请选择正确的串口");
    }
    qDebug() << "open";
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

// 串口数据接收，并做处理，显示在界面
void Widget::serialPortRead_Slot()
{
    recBuf = mySerialPort->readAll();
    // qDebug() << "触发接收函数" << recBuf;

    if (ui->checkRec->checkState() == Qt::Unchecked)
    {

        QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        QString newText = QString("%1 RX: %2").arg(timeStamp).arg(QString::fromLocal8Bit(recBuf));
        ui->msg_Rec->appendPlainText(newText);

        qDebug() << "字符" << recBuf;
    }
    else
    {
        // 16进制形式显示
        QString str1 = recBuf.toHex().toUpper();
        // qDebug() << "十六进制处理结果" << str1;

        QString str2;
        for (int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i, 2);
            str2 += " ";
        }

        QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
        QString newText = QString("%1 Rx: %2").arg(timeStamp).arg(str2.trimmed());
        ui->msg_Rec->appendPlainText(newText);


        qDebug() << "接收指令：" << str2.trimmed(); // 调整为输出16进制字符串
    }

    switch (READ_Command)
    {
    case MODBUS_ADD:
        READ_Command = 0;
        readID(recBuf);
        ui->newID->setText("0" + QString::number(readID(recBuf)));
        ui->ID->setText("0" + QString::number(readID(recBuf)));

        break;
    case NH4_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->NH4p_K->setText(stringK);
        ui->NH4p_B->setText(stringB);
        break;
    case NH4_ALL:
        handleNH4_ALL();
        break;
    case K_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->Kp_K->setText(stringK);
        ui->Kp_B->setText(stringB);
        break;
    case ORP_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->ORP_K->setText(stringK);
        ui->ORP_B->setText(stringB);
        break;
    case TOC_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->TOC_K->setText(stringK);
        ui->TOC_B->setText(stringB);
        break;
    case SAC_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->SAC_K->setText(stringK);
        ui->SAC_B->setText(stringB);
        break;
    case BOD_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->BOD_K->setText(stringK);
        ui->BOD_B->setText(stringB);
        break;
    case T_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->T_K->setText(stringK);
        ui->T_B->setText(stringB);
        break;
        break;
    case CHL_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->CHLK->setText(stringK);
        ui->CHLB->setText(stringB);
        break;
    case TUR_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->TUR_K->setText(stringK);
        ui->TUR_B->setText(stringB);
        break;
    case PH_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->pH_K->setText(stringK);
        ui->pH_B->setText(stringB);
        break;
    case SN_REG:
        READ_Command = 0;
        readSN(recBuf);
        ui->SN->setText(asciiSN);

        break;
    case HUMI_REG:
        READ_Command = 0;
        readHumi(recBuf);
        ui->Humi->setText(stringK);

        break;
    case WIPER_REG:
        READ_Command = 0;
        ui->WiperTime->setText(QString::number(readWiperTime(recBuf)));

        break;
    case NH4_T:
        READ_Command = 0;
        readNH4_T(recBuf);
        ui->NH4_T0->setText(DATA1);
        ui->NH4_TA->setText(DATA2);
        ui->NH4_TB->setText(DATA3);
        break;
    case K_T:
        READ_Command = 0;
        readNH4_T(recBuf);
        ui->K_T0->setText(DATA1);
        ui->K_TA->setText(DATA2);
        ui->K_TB->setText(DATA3);
        break;

    case Kp_comp:
        READ_Command = 0;
        readHumi(recBuf);
        ui->Kp_COMP->setText(stringK);
        break;

    case TEMP_REG:
        READ_Command = 0;
        readTEMP(recBuf);
        ui->BoardTemp->setText(stringK);
        break;
    case CALIB_COE_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->COND_K->setText(stringK);
        ui->COND_B->setText(stringB);
        break;
    case TDS_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->TDS_K->setText(stringK);
        ui->TDS_B->setText(stringB);
        break;
    case TUBR_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->TUBR_K->setText(stringK);
        ui->TUBR_B->setText(stringB);
        break;
    case SS_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->SS_K->setText(stringK);
        ui->SS_B->setText(stringB);
        break;
    case TRANS_Kb_REG:
        READ_Command = 0;
        readDataKB(recBuf);
        ui->TR_K->setText(stringK);
        ui->TR_B->setText(stringB);
        break;
    case COND_REG:
        handleCOND_ALL();
        break;
    case CHL_ALL:
        handleCHL_ALL();
        break;
    case TUBR_REG:
        handleTUBR_ALL();
        break;
    case BGA_Kb_REG:
        readDataKB(recBuf);
        ui->BGA_K->setText(stringK);
        break;
    case BGA_ALL:
        handleBGA_ALL();
        break;
    case COD_ALL:
        handleCOD_ALL();
        break;
    case COD_KB_REG:
        readDataKB(recBuf);
        ui->COD_K->setText(stringK);
        ui->COD_B->setText(stringB);

        break;
    default:
        break;
    }

    // 移动光标到文本结尾
    ui->msg_Rec->moveCursor(QTextCursor::End);
}

// void Widget::on_btnSend_clicked()
// {
//     QByteArray sendData;

//     // 判断是否为16进制发送，将发送区全部的asc2转换为16进制字符串显示，发送的时候转换为16进制发送
//     if (ui->checkSend->checkState() == Qt::Unchecked)
//     {
//         // 字符串形式发送
//         sendData = ui->msg_Send->toPlainText().toLocal8Bit();
//         qDebug() << "发送字符串成功" << sendData;
//     }
//     else
//     {
//         // 16进制发送
//         sendData = QByteArray::fromHex(ui->msg_Send->toPlainText().toUtf8());
//         qDebug() << "发送16进制成功" << sendData.toHex(' ');
//     }

//     mySerialPort->write(sendData);
//     qDebug() << sendData;
// }

// void Widget::on_btnClearRec_clicked()
// {
//     ui->msg_Rec->clear();
// }

// void Widget::on_btnClearSend_clicked()
// {
//     ui->msg_Send->clear();
// }

// 先前发送区的部分在多选框状态转换槽函数中进行转换。（最好多选框和发送区组成一个自定义控件，方便以后调用）
// void Widget::on_checkSend_stateChanged(int arg1)
// {
//     // 获取文本字符串
//     QString txtBuf = ui->msg_Send->toPlainText();

//     // 获取多选框状态，未选为0，选中为2
//     // 为0时，多选框未被勾选，将先前的发送区的16进制字符串转换为asc2字符串
//     if (arg1 == 0)
//     {

//         QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
//         // 文本控件清屏，显示新文本
//         ui->msg_Send->clear();
//         ui->msg_Send->insertPlainText(str1);
//         // 移动光标到文本结尾
//         ui->msg_Send->moveCursor(QTextCursor::End);
//     }
//     else
//     { // 多选框被勾选，将先前的发送区的asc2字符串转换为16进制字符串

//         QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
//         // 添加空格
//         QByteArray str2;
//         for (int i = 0; i < str1.length(); i += 2)
//         {
//             str2 += str1.mid(i, 2);
//             str2 += " ";
//         }
//         // 文本控件清屏，显示新文本
//         ui->msg_Send->clear();
//         ui->msg_Send->insertPlainText(str2);
//         // 移动光标到文本结尾
//         ui->msg_Send->moveCursor(QTextCursor::End);
//     }
// }

void Widget::on_checkRec_stateChanged(int arg1)
{
    // 获取文本字符串
    QString txtBuf = ui->msg_Rec->toPlainText();

    // 获取多选框状态，未选为0，选中为2
    if (arg1 == 0)
    {

        QByteArray str1 = QByteArray::fromHex(txtBuf.toUtf8());
        // 文本控件清屏，显示新文本
        ui->msg_Rec->clear();
        ui->msg_Rec->insertPlainText(str1);
        // 移动光标到文本结尾
        ui->msg_Rec->moveCursor(QTextCursor::End);
    }
    else
    { // 不为0时，多选框被勾选，接收区先前接收asc2字符串转换为16进制显示

        QByteArray str1 = txtBuf.toUtf8().toHex().toUpper();
        // 添加空格
        QByteArray str2;
        for (int i = 0; i < str1.length(); i += 2)
        {
            str2 += str1.mid(i, 2);
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
    addTimestamp = (arg1 != 0);
}

void Widget::processIDResponse()
{
    qDebug() << "触发查询ID";
    QByteArray recBuf = mySerialPort->readAll();

    // 判断是否为读取 ID 的响应
    if (recBuf.startsWith(QByteArray("\x01\x03\x02")))
    {
        // 假设响应格式是地址+功能码+字节数+数据，数据为一个字节的 ID
        int id = recBuf[3];
        ui->newID->setText(QString::number(id));
    }
}

/*  MODBUS获取地址*/
void Widget::on_getID_clicked()
{
    READ_Command = 0X000E;
    request = readRequestWithCRC("ff", READ_COMMAND, MODBUS_ADD, 1);
    qDebug() << "发送帧:" << request;
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

/*  K/B值查询 按钮点击函数集*/
// NH4+ 查询K/B
void Widget::on_getNH4p_clicked()
{
    READ_Command = 0x00A4;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, NH4_Kb_REG, 4);
    qDebug() << "发送帧:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// K+ 查询K/B
void Widget::on_getK_clicked()
{
    READ_Command = 0x00A8;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, K_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// ORP 查询K/B
void Widget::on_getORP_clicked()
{
    READ_Command = 0x00AC;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, ORP_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// TOC 查询K/B
void Widget::on_getTOCkb_clicked()
{
    READ_Command = 0x00B0;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TOC_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
// SAC 查询K/B
void Widget::on_getSACkb_clicked()
{
    READ_Command = 0x00B4;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, SAC_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// BOD 查询K/B
void Widget::on_getBODkb_clicked()
{
    READ_Command = 0x00B8;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, BOD_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
// T% 查询K/B
void Widget::on_getTab_clicked()
{
    READ_Command = 0x00BC;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, T_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// TUR 查询K/B
void Widget::on_getTURkb_clicked()
{
    READ_Command = 0x00C0;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TUR_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
// pH 查询K/B
void Widget::on_getPH_clicked()
{
    READ_Command = 0x00C4;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, PH_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// COD 查询K/B
void Widget::on_getCODkb_clicked()
{
    READ_Command = 0X0068;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CALIB_COE_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getSN_clicked()
{
    READ_Command = 0x0009;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, SN_REG, 4);
    qDebug() << "on_getSN_clicked:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getHUMI_clicked()
{
    READ_Command = 0x0056;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, HUMI_REG, 2);
    qDebug() << "on_getHUMI_clicked:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUpper().toUtf8()));
}

void Widget::on_getNH4_ALL_clicked()
{
    // //每次点击先清空timer，防止叠加
    // if (timer) {
    //     timer->stop();
    //     delete timer;
    //     timer = nullptr;
    // }

    READ_Command = 0X0048;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, NH4_ALL, 20);
    qDebug() << "on_getNH4_ALL_clicked:" << request;

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0048;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);
}

void Widget::on_getNH4_T_clicked()
{
    READ_Command = 0X00D0;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, NH4_T, 6);
    qDebug() << "on_getNH4_T_clicked:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
void Widget::on_getK_T_clicked()
{
    READ_Command = 0X00D6;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, K_T, 6);
    qDebug() << "on_getK_T_clicked:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getK_Comp_clicked()
{
    READ_Command = 0X007c;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, Kp_comp, 2);
    qDebug() << "on_getK_T_clicked:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getBoardTemp_clicked()
{
    READ_Command = 0X0030;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TEMP_REG, 2);

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
void Widget::on_getCOND_clicked()
{
    // //每次点击先清空timer，防止叠加
    // if (timer) {
    //     timer->stop();
    //     delete timer;
    //     timer = nullptr;
    // }

    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, COND_REG, 8);

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0036;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);
}

void Widget::on_getCHL_ALL_clicked()
{
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CHL_ALL, 4);

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0038;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);

}
void Widget::on_getCONDkb_clicked()
{
    READ_Command = 0X0060;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CALIB_COE_REG, 4);

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getTDSkb_clicked()
{
    READ_Command = 0X0040;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TDS_REG, 4);

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
void Widget::on_getWaterTemp_clicked()
{
}
void Widget::on_getCHLkb_clicked()
{
    READ_Command = 0X0061;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CALIB_COE_REG, 4);

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


QString Widget::reverseFourOctets(const QString &data)
{
    QString reversed;
    for (int i = data.length() - 2; i >= 0; i -= 2)
    {
        reversed += data.mid(i, 2);
    }
    return reversed;
}



void Widget::readDataKB(const QByteArray &data)
{
    QString data1_smallend = data.toHex().toUpper().mid(6, 8);
    QString data2_smallend = data.toHex().toUpper().mid(14, 8);

    qDebug() << "reversed data1 " << reverseFourOctets(data1_smallend);
    qDebug() << "reversed data2: " << reverseFourOctets(data2_smallend);

    qDebug() << " data1 " << data1_smallend;
    qDebug() << " data2: " << data2_smallend;

    stringK = QString::number(hexStringToFloat(reverseFourOctets(data1_smallend)));
    stringB = QString::number(hexStringToFloat(reverseFourOctets(data2_smallend)));

    qDebug() << "stringK: " << stringK;
    qDebug() << "stringB: " << stringB;
}



void Widget::readHumi(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 8);
    stringK = QString::number(hexStringToFloat(reverseFourOctets(data1)));
}


void Widget::readTEMP(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 8);
    stringK = QString::number(hexStringToFloat(reverseFourOctets(data1)));
}


void Widget::readNH4_T(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 8);
    QString data2 = data.toHex().toUpper().mid(14, 8);
    QString data3 = data.toHex().toUpper().mid(22, 8);

    DATA1 = QString::number(hexStringToFloat(reverseFourOctets(data1)));
    DATA2 = QString::number(hexStringToFloat(reverseFourOctets(data2)));
    DATA3 = QString::number(hexStringToFloat(reverseFourOctets(data3)));
    qDebug() << "data1: " << data1;
    qDebug() << "data2: " << data2;
    qDebug() << "data3: " << data3;
}

void Widget::readNH4_ALL(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 8);
    QString data2 = data.toHex().toUpper().mid(14, 8);
    QString data3 = data.toHex().toUpper().mid(22, 8);
    QString data4 = data.toHex().toUpper().mid(30, 8);
    QString data5 = data.toHex().toUpper().mid(38, 8);
    QString data6 = data.toHex().toUpper().mid(46, 8);
    QString data7 = data.toHex().toUpper().mid(54, 8);

    qDebug() << "data1: " << data1;
    qDebug() << "data2: " << data2;
    qDebug() << "data3: " << data3;
    qDebug() << "data4: " << data4;
    qDebug() << "data5: " << data5;
    qDebug() << "data6: " << data6;
    qDebug() << "data7: " << data7;

    DATA1 = QString::number(hexStringToFloat(reverseFourOctets(data1)));
    DATA2 = QString::number(hexStringToFloat(reverseFourOctets(data2)));
    DATA3 = QString::number(hexStringToFloat(reverseFourOctets(data3)));
    DATA4 = QString::number(hexStringToFloat(reverseFourOctets(data4)));
    DATA5 = QString::number(hexStringToFloat(reverseFourOctets(data5)));
    DATA6 = QString::number(hexStringToFloat(reverseFourOctets(data6)));
    DATA7 = QString::number(hexStringToFloat(reverseFourOctets(data7)));

}

int Widget::readID(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 4);
    qDebug() << "ID: " << data1;
    bool ok;
    int value = data1.toInt(&ok, 16);
    if (ok)
        qDebug() << "Converted integer value: " << value;
    else
        qDebug() << "Conversion failed.";
    return value;
}

int Widget::readWiperTime(const QByteArray &data)
{
    QString data1 = data.toHex().toUpper().mid(6, 4);
    qDebug() << "Wiper time: " << data1;
    bool ok;
    int value = data1.toInt(&ok, 16);
    if (ok)
        qDebug() << "Converted integer value: " << value;
    else
        qDebug() << "Conversion failed.";
    return value;
}

void Widget::readSN(const QByteArray &data)
{
    QString SN = data.toHex().toUpper().mid(6, 16);

    // 将十六进制字符串转换回字节数组
    QByteArray byteArray = QByteArray::fromHex(SN.toUtf8());
    asciiSN.clear();

    // 将字节数组解析为 ASCII 字符
    for (char byte : byteArray)
    {
        asciiSN += QChar(byte);
    }

    qDebug() << "ASCII SN: " << asciiSN;
}

QString Widget::appendKB(QString data1, QString data2)
{
    qDebug() << "reversed data1: " << reverseFourOctets(stringK);
    qDebug() << "reversed data2: " << reverseFourOctets(stringB);

    qDebug() << " data1 " << data1;
    qDebug() << " data2: " << data2;

    stringK = floatToHexString(stringToFloat(data1));
    stringB = floatToHexString(stringToFloat(data2));

    return reverseFourOctets(stringK).append(reverseFourOctets(stringB));
}
QString Widget::writeRequestWithCRC_M(QString address, int function, int REG, int quality, int length, QString data)
{
    QString request = address.append(intToHexWithPadding(function, 2)).append(intToHexWithPadding(REG, 4)).append(intToHexWithPadding(quality, 4)).append(intToHexWithPadding(length, 2)).append(data);
    qDebug() << "write request:" << request;

    return crcCalculation(request);
}
QString Widget::writeRequestWithCRC_S(QString address, int function, int REG, QString data)
{
    QString request = address.append(intToHexWithPadding(function, 2)).append(intToHexWithPadding(REG, 4)).append(data);
    qDebug() << "write request:" << request;

    return crcCalculation(request);
}

// NH4+ K/B值修改
void Widget::on_chgNH4p_clicked()
{
    // Modbus_pre= "FF1000A4000408";
    QString dataToWrite = appendKB(ui->NH4p_K->text(), ui->NH4p_B->text());
    qDebug() << "dataToWrite" << dataToWrite;

    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, NH4_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}
//ID修改
void Widget::on_setID_clicked()
{
    QString inputText = ui->IDtoSet->text();

    QString dataToWrite = ushortToHex(stringToUShort(inputText));

    request = writeRequestWithCRC_S(ui->newID->text(), WRITES_COMMAND, MODBUS_ADD, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// K+ K/B值修改
void Widget::on_chgK_clicked()
{
    // Modbus_pre= "FF1000A8000408";

    QString dataToWrite = appendKB(ui->Kp_K->text(), ui->Kp_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, K_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// orp+ K/B值修改
void Widget::on_chgORP_clicked()
{
    // Modbus_pre= "FF1000AC000408";
    QString dataToWrite = appendKB(ui->ORP_K->text(), ui->ORP_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, ORP_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

// ph K/B值修改
void Widget::on_chgPH_clicked()
{
    // Modbus_pre= "FF1000C4000408";
    QString dataToWrite = appendKB(ui->pH_K->text(), ui->pH_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, PH_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgTOC_clicked()
{
    // Modbus_pre= "FF1000B0000408";
    QString dataToWrite = appendKB(ui->TOC_K->text(), ui->TOC_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, TOC_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgCOD_clicked()
{
    // Modbus_pre= "FF1000C4000408";
    QString dataToWrite = appendKB(ui->COD_K->text(), ui->COD_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, TOC_Kb_REG, 4, 8, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgBOD_clicked()
{
    // Modbus_pre= "FF1000B8000408";
    QString dataToWrite = appendKB(ui->BOD_K->text(), ui->BOD_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, BOD_Kb_REG, 4, 8, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgTUR_clicked()
{
    // Modbus_pre= "FF1000C0000408";
    QString dataToWrite = appendKB(ui->TUR_K->text(), ui->TUR_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, TUR_Kb_REG, 4, 8, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgSAC_clicked()
{
    // Modbus_pre= "FF1000B4000408";
    QString dataToWrite = appendKB(ui->SAC_K->text(), ui->SAC_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, SAC_Kb_REG, 4, 8, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgT_clicked()
{
    // Modbus_pre= "FF1000BC000408";
    QString dataToWrite = appendKB(ui->T_K->text(), ui->T_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, T_Kb_REG, 4, 8, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_getWiperTime_clicked()
{
    READ_Command = 0x001A;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, WIPER_REG, 1);
    qDebug() << "WIPER:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgWiperTime_clicked()
{
    QString inputText = ui->chgWipertime->toPlainText();
    if (inputText.isEmpty())
    {
        // 使用 QMessageBox 显示提醒信息
        QMessageBox::warning(this, "提示", "请先输入要更改的时间。");
        return;
    }

    QString dataToWrite = ushortToHex(stringToUShort(inputText));

    request = writeRequestWithCRC_S(ui->newID->text(), WRITES_COMMAND, WIPER_REG, dataToWrite); // 需要改寄存器
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_startWiper_clicked()
{
    request = writeRequestWithCRC_S(ui->newID->text(), WRITES_COMMAND, WIPER_START_REG, "0001");
    qDebug() << "WIPER:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgNH4_T_clicked()
{
    QString dataToWrite = appendKB(ui->NH4_T0->text(), ui->NH4_TA->text());
    QString dataTB = reverseFourOctets(floatToHexString(stringToFloat(ui->NH4_TB->text())));
    dataToWrite = dataToWrite.append(dataTB);

    qDebug() << "dataToWrite" << dataToWrite;

    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, 0X00D0, 6, 12, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgK_T_clicked()
{
    QString dataToWrite = appendKB(ui->K_T0->text(), ui->K_TA->text());
    QString dataTB = reverseFourOctets(floatToHexString(stringToFloat(ui->K_TB->text())));
    dataToWrite = dataToWrite.append(dataTB);

    qDebug() << "dataToWrite" << dataToWrite;

    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, 0X00D6, 6, 12, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_chgK_Comp_clicked()
{
    QString comp = reverseFourOctets(floatToHexString(stringToFloat(ui->Kp_COMP->text())));

    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, Kp_comp, 2, 4, comp);
    qDebug() << "系数更改k+:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}

void Widget::on_stopNH4_ALL_clicked()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}


void Widget::on_btnClearRec_clicked()
{
    ui->msg_Rec->clear();

}

void Widget::handleNH4_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "pH", "ORP", "NH4+", "K+", "NH4_N"});
    }

    ui->NH4_TEMP->setText(DATA1);
    ui->pH->setText(DATA2);
    ui->ORP->setText(DATA3);
    ui->NH4_PLUS->setText(DATA4);
    ui->K_PLUS->setText(DATA5);
    ui->NH4_N->setText(DATA6);

    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);
    model->setData(model->index(row, 3), DATA3);
    model->setData(model->index(row, 4), DATA4);
    model->setData(model->index(row, 5), DATA5);
    model->setData(model->index(row, 6), DATA6);
}
void Widget::handleCOND_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "电导率", "盐度", "TDS"});
    }

    ui->COND_TEMP->setText(DATA1);
    ui->COND->setText(DATA2);
    ui->SAIL->setText(DATA3);
    ui->TDS->setText(DATA4);

    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);
    model->setData(model->index(row, 3), DATA3);
    model->setData(model->index(row, 4), DATA4);

}

void Widget::handleCHL_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "叶绿素"});
    }

    ui->CHL_value->setText(DATA1);
    ui->CHL_Temp->setText(DATA2);

    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);

}
void Widget::handleTUBR_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "浊度", "悬浮物", "水透明度"});
    }

    ui->TUBR_VALUE->setText(DATA1);
    ui->TUBR_temp->setText(DATA2);
    // ui->CHL_Temp->setText(DATA3);
    // ui->CHL_Temp->setText(DATA4);
    // ui->CHL_Temp->setText(DATA5);


    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);
    model->setData(model->index(row, 3), DATA3);
    model->setData(model->index(row, 4), DATA4);
    model->setData(model->index(row, 5), DATA5);
}
void Widget::handleBGA_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "浊度", "悬浮物", "水透明度"});
    }

    ui->TUBR_VALUE->setText(DATA1);
    ui->TUBR_temp->setText(DATA2);
    // ui->CHL_Temp->setText(DATA3);
    // ui->CHL_Temp->setText(DATA4);
    // ui->CHL_Temp->setText(DATA5);


    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);
    model->setData(model->index(row, 3), DATA3);
    model->setData(model->index(row, 4), DATA4);
    model->setData(model->index(row, 5), DATA5);
}
void Widget::handleCOD_ALL()
{
    READ_Command = 0;
    readNH4_ALL(recBuf);

    if (model->rowCount() == 0) {
        model->setHorizontalHeaderLabels({"时间", "温度", "COD", "TOC", "SAC", "BOD","T%","TUBR"});
    }

    ui->COD_VALUE->setText(DATA1);
    ui->COD_Temp->setText(DATA2);
    // ui->CHL_Temp->setText(DATA3);
    // ui->CHL_Temp->setText(DATA4);
    // ui->CHL_Temp->setText(DATA5);
    // ui->CHL_Temp->setText(DATA6);
    // ui->CHL_Temp->setText(DATA7);


    // 获取当前时间
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 创建一个新行并添加数据
    int row = model->rowCount();
    model->insertRow(row);

    model->setData(model->index(row, 0), timeStamp);
    model->setData(model->index(row, 1), DATA1);
    model->setData(model->index(row, 2), DATA2);
    model->setData(model->index(row, 3), DATA3);
    model->setData(model->index(row, 4), DATA4);
    model->setData(model->index(row, 5), DATA5);
    model->setData(model->index(row, 6), DATA6);
    model->setData(model->index(row, 7), DATA7);

}

void Widget::on_btnClearRec_4_clicked()
{
    model->clear();
}



void Widget::on_exportData_clicked()
{
    // 打开文件对话框，让用户选择保存 CSV 文件的位置
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Export CSV"), "",
                                                    tr("CSV Files (*.csv);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Error"),
                              tr("Cannot open file %1 for writing: %2.")
                                  .arg(fileName)
                                  .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);

    // 写入表头
    QStringList headers;
    for (int i = 0; i < model->columnCount(); ++i) {
        headers << model->headerData(i, Qt::Horizontal).toString();
    }
    out << headers.join(",") << "\n";

    // 写入数据
    for (int row = 0; row < model->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < model->columnCount(); ++col) {
            QVariant data = model->data(model->index(row, col));
            rowData << data.toString();
        }
        out << rowData.join(",") << "\n";
    }

    file.close();
    QMessageBox::information(this, tr("Export Successful"),
                             tr("Data has been exported to %1.").arg(fileName));
}


void Widget::on_stopCOND_clicked()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}

void Widget::on_stopCHL_ALL_clicked()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}



void Widget::on_getTUkb_clicked()
{
    READ_Command = 0X0062;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CALIB_COE_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_chgTUkb_clicked()
{
    QString dataToWrite = appendKB(ui->TUBR_K->text(), ui->TUBR_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, CALIB_COE_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_getSSkb_clicked()
{
    READ_Command = 0x0063;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TOC_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_chgSSkb_clicked()
{
    QString dataToWrite = appendKB(ui->SS_K->text(), ui->SS_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, TOC_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_getTRkb_clicked()
{
    READ_Command = 0x0064;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, SAC_Kb_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_chgTRkb_clicked()
{
    QString dataToWrite = appendKB(ui->TR_K->text(), ui->TR_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, SAC_Kb_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_getTUall_clicked()
{
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TUBR_REG, 8);

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0034;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);
}


void Widget::on_getBGAkb_clicked()
{
    READ_Command = 0x0065;
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CALIB_COE_REG, 4);
    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_chgBGAkb_clicked()
{
    QString dataToWrite = appendKB(ui->BGA_K->text(), ui->BGA_B->text());
    request = writeRequestWithCRC_M(ui->newID->text(), WRITEM_COMMAND, CALIB_COE_REG, 4, 8, dataToWrite);
    qDebug() << "write request with crc:" << request;

    mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
}


void Widget::on_getBGA_ALL_clicked()
{
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, CHL_ALL, 8);

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0066;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);
}


void Widget::on_stopBGA_ALL_clicked()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}


void Widget::on_getCODall_clicked()
{
    request = readRequestWithCRC(ui->newID->text(), READ_COMMAND, TDS_REG, 14);

    // 创建定时器并连接槽函数
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this](){
        READ_Command = 0X0067;

        mySerialPort->write(QByteArray::fromHex(request.toUtf8()));
    });
    timer->start(2000);
}


void Widget::on_stopCODall_clicked()
{
    if (timer) {
        timer->stop();
        delete timer;
        timer = nullptr;
    }
}

