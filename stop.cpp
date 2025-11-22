#include "stop.h"
#include "ui_stop.h"

stop::stop(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::stop)
{
    ui->setupUi(this);
}

stop::~stop()
{
    delete ui;
}
