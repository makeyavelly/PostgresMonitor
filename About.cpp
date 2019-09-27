#include "About.h"
#include "ui_About.h"

About::About(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::About)
{
    ui->setupUi(this);
    this->setAttribute(Qt::WA_DeleteOnClose);
    ui->labelAbout->setText(ui->labelAbout->text()
                            .replace("%applicationName%", QApplication::applicationName())
                            .replace("%applicationVersion%", QApplication::applicationVersion()));
}

About::~About()
{
    delete ui;
}

void About::on_btOk_clicked()
{
    this->window()->close();
}
