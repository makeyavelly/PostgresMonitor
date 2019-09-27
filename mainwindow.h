#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "sql.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void timerEvent(QTimerEvent *);

private:
    Ui::MainWindow *ui;
    Sql sql;
    int currentTab;

    void loadCodec();

    void loadSettings();
    void saveSettings();

    void loadSettingsWidgets();
    void saveSettingsWidgets();

    void reconnect();
    void update();

private slots:
    void on_actionConnect_triggered();
    void on_actionUpdate_triggered();
    void on_actionPsql_triggered();
    void on_actionAbout_triggered();

    void SuccessConnect();
    void ErrorConnect();
    void on_tabWidget_currentChanged(int);
};

#endif // MAINWINDOW_H
