#include "Widget.h"

Widget::Widget(Sql *sql, QWidget *parent) :
    QWidget(parent),
    sql(sql)
{}

void Widget::update()
{}

void Widget::loadSettings(QSettings &)
{}

void Widget::saveSettings(QSettings &)
{}
