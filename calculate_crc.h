#ifndef CALCULAT_CRC_H
#define CALCULAT_CRC_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <QtMath>
#include <QDebug>

class CalculatCRC
{
public:
    unsigned short calculateCRC16(const std::vector<unsigned char>& data);
    void AppendCRC16(QByteArray &datas);
    QByteArray UInt2ByteArray(uint num);
    void UInt2ByteArrayR(uint num, QByteArray &byteArray,
                                    int startIndex, int nBytes);
    QByteArray String2ByteArray(const QString &strInput);

};

#endif // CALCULAT_CRC_H
#include <QByteArray>
