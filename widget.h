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
#include <iostream>
#include <iomanip>
#include <sstream>
#include <bitset>

#define getNH4_Kb_Btn   0x00A4
#define getK_Kb_Btn     0x00A8
#define getORP_Kb_Btn   0x00AC
#define getTOC_Kb_Btn   0x00B0
#define getSAC_Kb_Btn   0x00B4
#define getBOD_Kb_Btn   0x00B8
#define getT_Kb_Btn     0x00BC
#define getTUR_Kb_Btn   0x00C0
#define getPH_Kb_Btn    0x00C4

#define chgNH4_Kb_Btn   1
#define chgK_Kb_Btn     2
#define chgORP_Kb_Btn   3
#define chgTOC_Kb_Btn   4
#define chgSAC_Kb_Btn   5
#define chgBOD_Kb_Btn   6
#define chgT_Kb_Btn     7
#define chgTUR_Kb_Btn   8
#define chgPH_Kb_Btn    9
#define chgCOD_Kb_Btn



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
    unsigned int SensorKB_type;
    unsigned int SensorKB_change;
    float Sensor_k;
    float Sensor_B;
    QString valueHex2String;
    QString stringK;
    QString stringB;

    void readDataKB(const QByteArray &data);
    void toChangeKB( QString data1, QString data2);

    float hexStringToFloat(const QString &hexStr);
    QString floatToHexString(float f);
private:
    Ui::Widget *ui;    
    QSerialPort *mySerialPort;
    // 定时发送-定时器
    QTimer *timSend;
    bool addTimestamp;
    QByteArray recBuf;


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
    void processIDResponse();
    void on_getNH4p_clicked();
    void on_getK_clicked();
    void on_getORP_clicked();
    void on_getPH_clicked();
    void on_getCODkb_clicked();
    void on_getBODkb_clicked();
    void on_getTOCkb_clicked();
    void on_getTURkb_clicked();
    void on_getSACkb_clicked();
    void on_getTab_clicked();
    void on_chgNH4p_clicked();
};
#endif // WIDGET_H
