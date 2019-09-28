#include <QTextCodec>
#include <QSettings>
#include <QProcess>

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "About.h"
#include "widgets/WidgetTransactions.h"
#include "widgets/WidgetSettingsServer.h"


const int WIDGET_TRANSACTION = 0;


QStringList getStringListByIntList(QList<int> src)
{
    QStringList res;
    foreach (int value, src) {
        res.append(QString::number(value));
    }
    return res;
}

QList<int> getIntListByStringList(QStringList src)
{
    QList<int> res;
    foreach (QString value, src) {
        res.append(value.toInt());
    }
    return res;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    sql(this)
{
    ui->setupUi(this);
    loadCodec();
    loadSettings();
    connect(&sql, SIGNAL(SuccessConnect()), this, SLOT(SuccessConnect()));
    connect(&sql, SIGNAL(ErrorConnect()), this, SLOT(ErrorConnect()));

    ui->tabWidget->addTab(new WidgetTransactions(&sql, this), "Transactions");
    ui->tabWidget->addTab(new WidgetSettingsServer(&sql, this), "Server settings");
    ui->tabWidget->setCurrentIndex(currentTab);

    loadSettingsWidgets();

    reconnect();
    update();
    startTimer(1000);
}

MainWindow::~MainWindow()
{
    saveSettings();
    saveSettingsWidgets();
    delete ui;
}

void MainWindow::timerEvent(QTimerEvent *)
{
    if (ui->actionAutoUpdate->isChecked()) {
        update();
    }
}


void MainWindow::loadCodec()
{
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
#if QT_VERSION_MAJOR < 5
    QTextCodec::setCodecForCStrings(codec);
    QTextCodec::setCodecForTr(codec);
#endif
}


void MainWindow::loadSettings()
{
    QSettings ini;

    this->setGeometry(ini.value("WINDOW/X", 0).toInt(),
                      ini.value("WINDOW/Y", 0).toInt(),
                      ini.value("WINDOW/WIDTH", 0).toInt(),
                      ini.value("WINDOW/HEIGHT", 0).toInt());
    ui->splitter->setSizes(getIntListByStringList(ini.value("WINDOW/SPLITTER", "100/200").toString().split("/")));
    currentTab = ini.value("WINDOW/TAB", 0).toInt();
    if (ini.value("WINDOW/MAXIMIZED", false).toBool()) {
        this->setWindowState(Qt::WindowMaximized);
    }

    ui->actionAutoUpdate->setChecked(ini.value("CHECK/AUTO_UPDATE", false).toBool());

    ui->editHost->setText(ini.value("CONNECTION/HOST", ui->editHost->text()).toString());
    ui->editPort->setText(ini.value("CONNECTION/PORT", ui->editPort->text()).toString());
    ui->editUser->setText(ini.value("CONNECTION/USER", ui->editUser->text()).toString());
    ui->editPassword->setText(ini.value("CONNECTION/PASSWORD", ui->editPassword->text()).toString());
}

void MainWindow::saveSettings()
{
    QSettings ini;

    ini.setValue("WINDOW/X", this->x());
    ini.setValue("WINDOW/Y", this->y());
    ini.setValue("WINDOW/WIDTH", this->width());
    ini.setValue("WINDOW/HEIGHT", this->height());
    ini.setValue("WINDOW/SPLITTER", getStringListByIntList(ui->splitter->sizes()).join("/"));
    ini.setValue("WINDOW/TAB", ui->tabWidget->currentIndex());
    ini.setValue("WINDOW/MAXIMIZED", this->isMaximized());

    ini.setValue("CHECK/AUTO_UPDATE", ui->actionAutoUpdate->isChecked());

    ini.setValue("CONNECTION/HOST", ui->editHost->text());
    ini.setValue("CONNECTION/PORT", ui->editPort->text());
    ini.setValue("CONNECTION/USER", ui->editUser->text());
    ini.setValue("CONNECTION/PASSWORD", ui->editPassword->text());
}

void MainWindow::loadSettingsWidgets()
{
    QSettings ini;
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        QWidget *widget = ui->tabWidget->widget(i);
        if (widget) {
            static_cast<Widget*>(widget)->loadSettings(ini);
        }
    }
}

void MainWindow::saveSettingsWidgets()
{
    QSettings ini;
    for (int i = 0; i < ui->tabWidget->count(); ++i) {
        QWidget *widget = ui->tabWidget->widget(i);
        if (widget) {
            static_cast<Widget*>(widget)->saveSettings(ini);
        }
    }
}


void MainWindow::reconnect()
{
    sql.connect(ui->editHost->text(), ui->editPort->text().toInt(),
                ui->editUser->text(), ui->editPassword->text());
}

void MainWindow::update()
{
    QWidget *widget = ui->tabWidget->currentWidget();
    if (widget) {
        static_cast<Widget*>(widget)->update();
    }
}


void MainWindow::on_actionConnect_triggered()
{
    reconnect();
}

void MainWindow::on_actionUpdate_triggered()
{
    update();
}

void MainWindow::on_actionPsql_triggered()
{
    //TODO
//    QProcess::execute(QString("startPsl %1 %2 %3")
//                      .arg(ui->editHost->text())
//                      .arg(ui->editPort->text())
//                      .arg(ui->editUser->text()));
}

void MainWindow::on_actionAbout_triggered()
{
    (new About())->show();
}

void MainWindow::SuccessConnect()
{
    ui->statusBar->showMessage("Connected");
    ui->statusBar->setStyleSheet("QStatusBar {color: #008f00}");
}

void MainWindow::ErrorConnect()
{
    ui->statusBar->showMessage("Not connection");
    ui->statusBar->setStyleSheet("QStatusBar {color: #8f0000}");
}

void MainWindow::on_tabWidget_currentChanged(int)
{
    update();
}
