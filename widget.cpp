#include "widget.h"
#include "ui_widget.h"
#include <QGraphicsDropShadowEffect>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    //创建阴影
    QGraphicsDropShadowEffect * effect = new QGraphicsDropShadowEffect(this);
    //设置偏移
    effect->setOffset(10);
    //设置模糊
    effect->setBlurRadius(50);
    //设置颜色
    effect->setColor(QColor(63, 63, 63,20));
    ui->verticalWidget_3->setGraphicsEffect(effect);
}

Widget::~Widget()
{
    delete ui;
}
