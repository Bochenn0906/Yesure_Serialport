#ifndef myComboBox_H
#define myComboBox_H

#include <QComboBox>
#include <QMouseEvent>
#include <QtSerialPort>
#include <QSerialPortInfo>


class myComboBox : public QComboBox
{
    Q_OBJECT
public:
    explicit myComboBox(QWidget *parent = nullptr);


protected:

    void mousePressEvent(QMouseEvent *event);


signals:


private:
    void scanActivePort();
};

#endif // myComboBox_H
