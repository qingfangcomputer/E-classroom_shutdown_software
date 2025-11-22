#ifndef UP_H
#define UP_H

#include <QMainWindow>

namespace Ui {
class up;
}

class up : public QMainWindow
{
    Q_OBJECT

public:
    explicit up(QWidget *parent = nullptr);
    ~up();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::up *ui;
};

#endif // UP_H
