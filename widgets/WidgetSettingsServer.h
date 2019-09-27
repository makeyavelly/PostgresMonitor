#ifndef WIDGETSETTINGSSERVER_H
#define WIDGETSETTINGSSERVER_H

#include "Widget.h"

namespace Ui {
class WidgetSettingsServer;
}

class WidgetSettingsServer : public Widget
{
    Q_OBJECT

public:
    explicit WidgetSettingsServer(Sql *sql, QWidget *parent = 0);
    ~WidgetSettingsServer();

    void update();
    void loadSettings(QSettings &ini);
    void saveSettings(QSettings &ini);

private:
    Ui::WidgetSettingsServer *ui;

    void clearTable();
    void updateFilters();

private slots:
    void on_editName_textEdited(const QString &);
    void on_comboContext_currentIndexChanged(const QString &);
    void on_btSave_clicked();
    void on_btRestart_clicked();
    void on_btResetAll_clicked();
    void on_btExport_clicked();
    void on_btImport_clicked();

    void slotReset();
};

#endif // WIDGETSETTINGSSERVER_H
