#include "mycombobox.h"

myComboBox::myComboBox(QWidget *parent) : QComboBox{parent}
{
    scanActivePort();
}


void myComboBox::scanActivePort()
{
    clear();

    QStringList serialPortName;

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){

        QString serialPortInfo = info.portName() + ": " + info.description();// 串口设备信息，芯片/驱动名称

        serialPortName << serialPortInfo;
    }
    this->addItems(serialPortName);
}

void myComboBox::mousePressEvent(QMouseEvent *event)
{

    if(event->button() == Qt::LeftButton)
    {
        // 扫描可用串口
        scanActivePort();

        // 弹出下拉框
        showPopup();
    }
}
