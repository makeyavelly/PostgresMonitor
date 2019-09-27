#include <QProcess>
#include <QMessageBox>
#include <QFileDialog>

#include "WidgetSettingsServer.h"
#include "ui_WidgetSettingsServer.h"
#include "log.h"


const int COL_NAME      =  0;
const int COL_VALUE     =  1;
const int COL_UNIT      =  2;
const int COL_DEFAULT   =  3;
const int COL_MIN       =  4;
const int COL_MAX       =  5;
const int COL_CONTEXT   =  6;
const int COL_REMOVE    =  7;


class ButtonRemove : public QPushButton
{
public:
    ButtonRemove(QString name) :
        QPushButton(QIcon(":/icons/remove"), QString()),
        m_name(name)
    {
        setFlat(true);
        setToolTip("Reset");
    }

    QString name() const
    {
        return m_name;
    }

private:
    QString m_name;
};


WidgetSettingsServer::WidgetSettingsServer(Sql *sql, QWidget *parent) :
    Widget(sql, parent),
    ui(new Ui::WidgetSettingsServer)
{
    ui->setupUi(this);
    clearTable();
}

WidgetSettingsServer::~WidgetSettingsServer()
{
    delete ui;
}

void WidgetSettingsServer::update()
{
    clearTable();
    int row = 0;
    foreach (Sql::Record record, sql->select("select * from pg_settings;")) {
        ui->table->setRowCount(++row);
        QString name = record.get("name").value().toString();
        QString value = record.get("setting").value().toString();
        QString defaultValue = record.get("boot_val").value().toString();
        if (defaultValue.isEmpty()) {
            defaultValue = record.get("reset_val").value().toString();
        }
        bool isNotDefault = record.get("sourcefile").value().toString().contains("postgresql.auto.conf");

        QTableWidgetItem *item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_NAME, item);
        item->setText(name);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_VALUE, item);
        item->setText(value);
        item->setData(Qt::UserRole, value);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_UNIT, item);
        item->setText(record.get("unit").value().toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_DEFAULT, item);
        item->setText(defaultValue);
        item->setData(Qt::UserRole, isNotDefault);
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_MIN, item);
        item->setText(record.get("min_val").value().toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_MAX, item);
        item->setText(record.get("max_val").value().toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        item = new QTableWidgetItem();
        ui->table->setItem(row - 1, COL_CONTEXT, item);
        item->setText(record.get("context").value().toString());
        item->setFlags(item->flags() ^ Qt::ItemIsEditable);

        if (isNotDefault) {
            ButtonRemove *bt = new ButtonRemove(name);
            ui->table->setCellWidget(row - 1, COL_REMOVE, bt);
            connect(bt, SIGNAL(clicked()), this, SLOT(slotReset()));
        }
    }
    updateFilters();
    ui->table->resizeColumnsToContents();
}

void WidgetSettingsServer::loadSettings(QSettings &ini)
{
    int columnSort = ini.value("WIDGET_SETTINGS_SERVER/SORT_COLUMN", COL_NAME).toInt();
    Qt::SortOrder orderSort = Qt::SortOrder(ini.value("WIDGET_SETTINGS_SERVER/SORT_ORDER",
                                                      Qt::AscendingOrder).toInt());
    ui->table->sortByColumn(columnSort, orderSort);
    ui->editName->setText(ini.value("WIDGET_SETTINGS_SERVER/FILTER_NAME", QString()).toString());
    ui->comboContext->setCurrentIndex(ui->comboContext->findText(
                    ini.value("WIDGET_SETTINGS_SERVER/FILTER_CONTEXT", QString("--- All ---")).toString()));
}

void WidgetSettingsServer::saveSettings(QSettings &ini)
{
    ini.setValue("WIDGET_SETTINGS_SERVER/SORT_COLUMN", ui->table->horizontalHeader()->sortIndicatorSection());
    ini.setValue("WIDGET_SETTINGS_SERVER/SORT_ORDER", int(ui->table->horizontalHeader()->sortIndicatorOrder()));
    ini.setValue("WIDGET_SETTINGS_SERVER/FILTER_NAME", ui->editName->text());
    ini.setValue("WIDGET_SETTINGS_SERVER/FILTER_CONTEXT", ui->comboContext->currentText());
}

void WidgetSettingsServer::clearTable()
{
    ui->table->clear();
    ui->table->setRowCount(0);
    QStringList labels;
    labels.append(tr("Name"));
    labels.append(tr("Value"));
    labels.append(tr("Unit"));
    labels.append(tr("Default"));
    labels.append(tr("Min value"));
    labels.append(tr("Max value"));
    labels.append(tr("Context"));
    labels.append(QString());
    ui->table->setColumnCount(labels.count());
    ui->table->setHorizontalHeaderLabels(labels);
}

void WidgetSettingsServer::updateFilters()
{
    QString name = ui->editName->text();
    QString context = ui->comboContext->currentText();

    bool isFilterName = !name.isEmpty();
    bool isFilterContext = ui->comboContext->currentIndex() > 0;

    for (int row = 0; row < ui->table->rowCount(); ++row) {
        if (isFilterName && !ui->table->item(row, COL_NAME)->text().contains(name)) {
            ui->table->hideRow(row);
        } else if (isFilterContext && (ui->table->item(row, COL_CONTEXT)->text() != context)) {
            ui->table->hideRow(row);
        } else {
            ui->table->showRow(row);
        }
    }
}


void WidgetSettingsServer::on_editName_textEdited(const QString &)
{
    updateFilters();
    ui->table->resizeColumnsToContents();
}

void WidgetSettingsServer::on_comboContext_currentIndexChanged(const QString &)
{
    updateFilters();
    ui->table->resizeColumnsToContents();
}

void WidgetSettingsServer::on_btSave_clicked()
{
    QStringList errors;
    for (int row = 0; row < ui->table->rowCount(); ++row) {
        QString value = ui->table->item(row, COL_VALUE)->text();
        if (value != ui->table->item(row, COL_VALUE)->data(Qt::UserRole)) {
            QString name = ui->table->item(row, COL_NAME)->text();
            if (!sql->setParam(name, value)) {
                errors.append(QString("Параметр '%1' не удалось изменить.").arg(name));
            }
        }
    }
    if (!errors.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", errors.join("\n"));
    }
    sql->reloadConf();
    update();
}

void WidgetSettingsServer::on_btRestart_clicked()
{
//    if (QProcess::execute("pg_ctl restart" != 0)) {
//        QProcess::execute("sudo service postgresql restart");
//    }
}

void WidgetSettingsServer::on_btResetAll_clicked()
{
    if (QMessageBox::question(this, "Предупреждение", "Удалить все пользовательские настройки?",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        sql->resetAllParam();
        sql->reloadConf();
        update();
    }
}

void WidgetSettingsServer::on_btExport_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export", QString(), "*.conf");
    if (!fileName.isEmpty()) {
        if (!fileName.contains(".conf")) {
            fileName.append(".conf");
        }
        QSettings file(fileName, QSettings::IniFormat);
        file.beginGroup("SETTING");
        for (int row = 0; row < ui->table->rowCount(); ++row) {
            bool isNotDefault = ui->table->item(row, COL_DEFAULT)->data(Qt::UserRole).toBool();
            QString value = ui->table->item(row, COL_VALUE)->text();
            QString defaultValue = ui->table->item(row, COL_VALUE)->data(Qt::UserRole).toString();
            QString name = ui->table->item(row, COL_NAME)->text();
            if (isNotDefault || value != defaultValue) {
                file.setValue(name, value);
                logInfo(QString("%1=%2").arg(name).arg(value), "EXPORT");
            }
        }
        file.endGroup();
    }
}

void WidgetSettingsServer::on_btImport_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Export", QString(), "*.conf");
    if (!fileName.isEmpty()) {
        sql->resetAllParam();
        QSettings file(fileName, QSettings::IniFormat);
        file.beginGroup("SETTING");
        foreach (QString key, file.allKeys()) {
            QString value = file.value(key).toString();
            sql->setParam(key, value);
            logInfo(QString("%1=%2").arg(key).arg(value), "IMPORT");
        }
        file.endGroup();
        sql->reloadConf();
        update();
    }
}

void WidgetSettingsServer::slotReset()
{
    ButtonRemove *bt = static_cast<ButtonRemove*>(sender());
    sql->resetParam(bt->name());
    sql->reloadConf();
    update();
}
