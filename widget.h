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

#define NH4_Kb_REG   0x00A4
#define K_Kb_REG     0x00A8
#define ORP_Kb_REG   0x00AC
#define TOC_Kb_REG   0x00B0
#define SAC_Kb_REG   0x00B4
#define BOD_Kb_REG   0x00B8
#define T_Kb_REG     0x00BC
#define TUR_Kb_REG   0x00C0
#define PH_Kb_REG    0x00C4
#define SN_REG       0x0009
#define HUMI_REG     0x0056
#define WIPER_REG    0x001A
#define WIPER_START_REG    0x0019
#define MODBUS_ADD   0X000E
#define NH4_ALL      0X0048
#define NH4_T        0X00D0
#define K_T          0X00D6
#define Kp_comp      0X007C
#define TEMP_REG     0X0030
#define COND_REG     0X0036
#define TDS_REG      0X0040
#define CALIB_COE_REG     0X0060


#define READ_COMMAND    03
#define WRITEM_COMMAND  16
#define WRITES_COMMAND  06

#define WIRITE_KB    1
#define WIRITE_COE    2

QT_BEGIN_NAMESPACE
namespace Ui {
class Widget;
}

enum Location{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    CENTER
};


QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

    QString request;

    unsigned int READ_Command;
    unsigned int SensorKB_change;

    QString Device_id;
    QString Modbus_pre;
    QString value_K;
    QString value_B;

    QString stringK;
    QString stringB;

    QString asciiSN;
    QString DATA1;
    QString DATA2;
    QString DATA3;
    QString DATA4;
    QString DATA5;
    QString DATA6;
    QString DATA7;
    QString DATA8;
    QString DATA9;
    QString DATA10;
    void readDataKB(const QByteArray &data);
    void readSN(const QByteArray &data);
    void readHumi(const QByteArray &data);
    int readWiperTime(const QByteArray &data);
    int readID(const QByteArray &data);
    void readNH4_ALL(const QByteArray &data);
    void readNH4_T(const QByteArray &data);
    void readTEMP(const QByteArray &data);
    QString appendKB( QString data1, QString data2);
    float hexStringToFloat(const QString &hexStr);
    QString floatToHexString(float f);
    QString intToHexWithPadding(int value, int padding);
    float stringToFloat(const QString &str);
    QString ushortToHex(unsigned short value);
    unsigned short crc16(unsigned char *CmmBuf, unsigned char Len);
    bool        crc_Checking(QString);
    QString     crcCalculation(QString message);
    QString readRequestWithCRC(QString address, int function, int REG, int quality);
    QString writeRequestWithCRC_M(QString address, int function, int REG, int quality, int length,QString data);
    QString writeRequestWithCRC_S(QString address, int function, int REG,QString data);
    QString reverseFourOctets(const QString &data);
private:
    Ui::Widget *ui;    
    QSerialPort *mySerialPort;
    // 定时发送-定时器
    QTimer *timSend;
    bool addTimestamp;
    QByteArray recBuf;
    bool isLeftPressed;
    QPoint mouseOffset;
    Location location;
    void setCursorShape(const QPoint& point);

protected:
    // void mousePressEvent(QMouseEvent *event);
    // void mouseMoveEvent(QMouseEvent *event);
    // void mouseReleaseEvent(QMouseEvent *event);
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
    void on_chgK_clicked();
    void on_chgPH_clicked();
    void on_chgORP_clicked();

    void on_chgCOD_clicked();
    void on_chgBOD_clicked();
    void on_chgTOC_clicked();
    void on_chgTUR_clicked();
    void on_chgSAC_clicked();
    void on_chgT_clicked();
    void on_getSN_clicked();
    void on_getHUMI_clicked();
    void on_getBoardTemp_clicked();
    void on_getWaterTemp_clicked();

    void on_getWiperTime_clicked();
    void on_chgWiperTime_clicked();
    void on_startWiper_clicked();
    void on_getID_clicked();
    void on_getNH4_ALL_clicked();
    void on_getNH4_T_clicked();
    void on_getK_T_clicked();
    void on_getK_Comp_clicked();
    void on_chgNH4_T_clicked();
    void on_chgK_T_clicked();
    void on_chgK_Comp_clicked();
    void on_getCOND_clicked();
    void on_getTDSkb_clicked();
    void on_getCONDkb_clicked();
};
#endif // WIDGET_H
