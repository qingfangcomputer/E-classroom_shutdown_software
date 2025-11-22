#ifndef STOP_H
#define STOP_H

#include <QMainWindow>

namespace Ui {
class stop;
}

class stop : public QMainWindow
{
    Q_OBJECT

public:
    explicit stop(QWidget *parent = nullptr);
    ~stop();

private:
    Ui::stop *ui;
};

#endif // STOP_H
