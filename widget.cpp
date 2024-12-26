#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
}

Widget::~Widget()
{
    delete ui;
}

//页面切换
void Widget::switchPage(){
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if(button==ui->SingleIns)
        ui->stackedWidget->setCurrentIndex(0);
    else if(button==ui->BrushCheck)
        ui->stackedWidget->setCurrentIndex(1);
    else if(button==ui->ChannelCheck)
        ui->stackedWidget->setCurrentIndex(2);
    else if(button==ui->CodCali)
        ui->stackedWidget->setCurrentIndex(3);
    else if(button==ui->MixedSolu)
        ui->stackedWidget->setCurrentIndex(4);
    else if(button==ui->Robust)
        ui->stackedWidget->setCurrentIndex(5);
    int i = 0;
    ui->stackedWidget->widget(i);

}

//各页面按钮
void Widget::on_SingleIns_clicked()
{
    switchPage();
}

void Widget::on_BrushCheck_clicked()
{
    switchPage();
}
void Widget::on_ChannelCheck_clicked()
{
    switchPage();
}
void Widget::on_CodCali_clicked()
{
    switchPage();
}
void Widget::on_MixedSolu_clicked()
{
    switchPage();
}
void Widget::on_Robust_clicked()
{
    switchPage();
}



