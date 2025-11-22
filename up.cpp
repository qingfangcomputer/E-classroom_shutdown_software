#include "up.h"
#include "ui_up.h"
#include "QUrl"
#include "QDesktopServices"

up::up(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::up)
{
    ui->setupUi(this);
    setWindowTitle("软件更新");
}

up::~up()
{
    delete ui;
}

void up::on_pushButton_clicked()
{
    QDesktopServices::openUrl(QUrl("https://github.com/qingfangcomputer/E-classroom_shutdown_software", QUrl::TolerantMode));
}

void up::on_pushButton_2_clicked()
{
    this->close();
}
