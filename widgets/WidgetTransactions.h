#ifndef WIDGETTRANSACTIONS_H
#define WIDGETTRANSACTIONS_H

#include <QMenu>
#include <QTableWidgetItem>

#include "Widget.h"

namespace Ui {
class WidgetTransactions;
}

class WidgetTransactions : public Widget
{
    Q_OBJECT

public:
    explicit WidgetTransactions(Sql *sql, QWidget *parent = 0);
    ~WidgetTransactions();
    void resizeEvent(QResizeEvent *);
    bool eventFilter(QObject *target, QEvent *event);

    void update();
    void loadSettings(QSettings &ini);
    void saveSettings(QSettings &ini);

private:
    Ui::WidgetTransactions *ui;
    QMenu contextMenu;
    QTableWidgetItem *currentItem;

    void clearTable();
    void createContextMenu();

    void showContextMenu(QTableWidgetItem *item);

private slots:
    void on_actionKill_triggered();
    void on_actionKillWhere_triggered();
    void on_actionKillAll_triggered();
    void on_actionFree_triggered();
};

#endif // WIDGETTRANSACTIONS_H
