#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>

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

    void switchPage();

private slots:
    void on_SingleIns_clicked();
    void on_BrushCheck_clicked();
    void on_ChannelCheck_clicked();
    void on_CodCali_clicked();
    void on_MixedSolu_clicked();
    void on_Robust_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
