#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QSettings>

#include "sql.h"

class Widget : public QWidget
{
public:
    explicit Widget(Sql *sql, QWidget *parent = 0);

    virtual void update();
    virtual void loadSettings(QSettings &);
    virtual void saveSettings(QSettings &);

protected:
    Sql *sql;
};

#endif // WIDGET_H
