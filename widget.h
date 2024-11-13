#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMessageBox>
#include <QString>
#include <QTimer>
#include <QPainter>
#include <QtSerialBus>
#include <QModbusDataUnit>
#include <QModbusClient>

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    // 绘图事件
    // void paintEvent(QPaintEvent *);

private:
    Ui::Widget *ui;    
    QSerialPort *mySerialPort;

    // 定时发送-定时器
    QTimer *timSend;
    bool addTimestamp;


private slots:
    void on_btnSwitchOn_clicked();
    void on_btnSwitchOff_clicked();
    void serialPortRead_Slot();
    void on_btnSend_clicked();
    void on_btnClearRec_clicked();
    void on_btnClearSend_clicked();
    void on_checkSend_stateChanged(int arg1);
    void on_checkRec_stateChanged(int arg1);
    void on_checkTime_stateChanged(int arg1);
    void on_fetch_ID_clicked();
    void on_change_ID_clicked();
    void processIDResponse();
};
#endif // WIDGET_H
