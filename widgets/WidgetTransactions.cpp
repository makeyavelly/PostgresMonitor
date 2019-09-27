#include <QDateTime>
#include <QMessageBox>
#include <QMouseEvent>

#include "WidgetTransactions.h"
#include "ui_WidgetTransactions.h"

#include "log.h"


const int COL_QUERY         =  0;
const int COL_DATABASE      =  1;
const int COL_APPLICATION   =  2;
const int COL_USER          =  3;
const int COL_IP            =  4;
const int COL_STATE         =  5;
const int COL_WAITING       =  6;
const int COL_T_START       =  7;
const int COL_ELAPSED       =  8;


WidgetTransactions::WidgetTransactions(Sql *sql, QWidget *parent) :
    Widget(sql, parent),
    ui(new Ui::WidgetTransactions)
{
    ui->setupUi(this);
    clearTable();
    createContextMenu();
    currentItem = 0x0;
    ui->table->installEventFilter(this);
}

WidgetTransactions::~WidgetTransactions()
{
    delete ui;
}

void WidgetTransactions::resizeEvent(QResizeEvent *)
{
    int w = 0;
    for (int col = 0; col < ui->table->columnCount(); ++col) {
        if (col != COL_QUERY) {
            w += ui->table->columnWidth(col);
        }
    }
    ui->table->setColumnWidth(COL_QUERY, ui->table->width() - w - 26);
}

bool WidgetTransactions::eventFilter(QObject *target, QEvent *event)
{
    bool res = QWidget::eventFilter(target, event);
    if (target == ui->table && event->type() == QEvent::ContextMenu) {
        QTableWidgetItem *item = ui->table->currentItem();
        if (item) {
            showContextMenu(item);
        }
    }
    return res;
}

void WidgetTransactions::update()
{
    clearTable();
    if (sql->isOpen()) {
        int row = 0;
        foreach (Sql::Record record, sql->select("select * from pg_stat_activity;")) {
//                                                 "where wait_event_type='Client';")) {
            ui->table->setRowCount(++row);
            QString pid = record.get("pid").value().toString();

            QTableWidgetItem *item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_QUERY, item);
            item->setData(Qt::UserRole, pid);
            item->setText(record.get("query").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_DATABASE, item);
            item->setText(record.get("datname").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_APPLICATION, item);
            item->setText(record.get("application_name").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_USER, item);
            item->setText(record.get("usename").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_IP, item);
            item->setText(record.get("client_addr").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_T_START, item);
            QDateTime tStart = record.get("backend_start").value().toDateTime();
            item->setData(Qt::DisplayRole, tStart);
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_STATE, item);
            item->setText(record.get("state").value().toString());
            item->setToolTip(item->text());

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_WAITING, item);
            bool isBlock = record.get("waiting").value().toBool();
            item->setData(Qt::DisplayRole, isBlock);
            if (isBlock) {
                item->setToolTip(sql->getBlock(pid).join("\n"));
                item->setBackgroundColor(QColor("#ffafaf"));
            }

            item = new QTableWidgetItem();
            ui->table->setItem(row - 1, COL_ELAPSED, item);
            item->setData(Qt::DisplayRole, tStart.secsTo(QDateTime::currentDateTime()));
            item->setToolTip(item->text());
        }
    }
    ui->table->resizeColumnsToContents();
    resizeEvent(0x0);
}

void WidgetTransactions::loadSettings(QSettings &ini)
{
    int columnSort = ini.value("WIDGET_TRANSACTIONS/SORT_COLUMN", COL_ELAPSED).toInt();
    Qt::SortOrder orderSort = Qt::SortOrder(ini.value("WIDGET_TRANSACTIONS/SORT_ORDER",
                                                      Qt::DescendingOrder).toInt());
    ui->table->sortByColumn(columnSort, orderSort);
}

void WidgetTransactions::saveSettings(QSettings &ini)
{
    ini.setValue("WIDGET_TRANSACTIONS/SORT_COLUMN", ui->table->horizontalHeader()->sortIndicatorSection());
    ini.setValue("WIDGET_TRANSACTIONS/SORT_ORDER", int(ui->table->horizontalHeader()->sortIndicatorOrder()));
}


void WidgetTransactions::clearTable()
{
    ui->table->clear();
    ui->table->setRowCount(0);
    QStringList labels;
    labels.append(tr("Query"));
    labels.append(tr("Database"));
    labels.append(tr("Application"));
    labels.append(tr("User"));
    labels.append(tr("IP"));
    labels.append(tr("State"));
    labels.append(tr("W"));
    labels.append(tr("Time start"));
    labels.append(tr("Elapsed, sec"));
    ui->table->setColumnCount(labels.count());
    ui->table->setHorizontalHeaderLabels(labels);
}

void WidgetTransactions::createContextMenu()
{
    contextMenu.addAction(ui->actionFree);
    contextMenu.addAction(ui->actionKill);
    contextMenu.addAction(ui->actionKillWhere);
    contextMenu.addAction(ui->actionKillAll);
}


void WidgetTransactions::showContextMenu(QTableWidgetItem *item)
{
    int column = item->column();
    QString header = ui->table->horizontalHeaderItem(column)->text();
    currentItem = item;
    if (column == COL_QUERY || column == COL_T_START || column == COL_ELAPSED) {
        ui->actionKillWhere->setVisible(false);
    } else {
        ui->actionKillWhere->setVisible(true);
        ui->actionKillWhere->setText(QString("Kill %1='%2'")
                                     .arg(header.toLower())
                                     .arg(currentItem->text()));
    }
    if (column == COL_WAITING && item->data(Qt::DisplayRole).toBool()) {
        ui->actionFree->setVisible(true);
    } else {
        ui->actionFree->setVisible(false);
    }
    contextMenu.move(QCursor::pos());
    contextMenu.show();
}

void WidgetTransactions::on_actionKill_triggered()
{
    if (currentItem) {
        QString id = ui->table->item(currentItem->row(), COL_QUERY)->data(Qt::UserRole).toString();
        if (sql->rollback(id)) {
            ui->table->removeRow(currentItem->row());
        } else {
            QMessageBox::warning(this, "Предупреждение", "Транзакция не прервана!");
        }
    }
}

void WidgetTransactions::on_actionKillWhere_triggered()
{
    int row = 0;
    bool isError = false;
    QString currentText = currentItem->text();
    int column = currentItem->column();
    while (row < ui->table->rowCount()) {
        if (currentText == ui->table->item(row, column)->text()) {
            QString id = ui->table->item(row, COL_QUERY)->data(Qt::UserRole).toString();
            if (sql->rollback(id)) {
                ui->table->removeRow(row);
            } else {
                isError = true;
                row++;
            }
        } else {
            row++;
        }
    }
    if (isError) {
        QMessageBox::warning(this, "Предупреждение", "Не все транзакции прерваны!");
    }
}

void WidgetTransactions::on_actionKillAll_triggered()
{
    int row = 0;
    bool isError = false;
    while (row < ui->table->rowCount()) {
        QString id = ui->table->item(row, COL_QUERY)->data(Qt::UserRole).toString();
        if (sql->rollback(id)) {
            ui->table->removeRow(row);
        } else {
            isError = true;
            row++;
        }
    }
    if (isError) {
        QMessageBox::warning(this, "Предупреждение", "Не все транзакции прерваны!");
    }
}

void WidgetTransactions::on_actionFree_triggered()
{
    QStringList blocks = currentItem->toolTip().split("\n");
    logInfo(blocks.join(";"));
    foreach (QString pid, blocks) {
        logInfo(pid);
        sql->rollback(pid);
    }
    update();
}
