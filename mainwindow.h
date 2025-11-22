#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "versionchecker.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewVersionAvailable(QString version);
    void onNoUpdatesAvailable();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();

    void on_commandLinkButton_clicked();

    void on_commandLinkButton_2_clicked();

    void on_commandLinkButton_3_clicked();

private:
    Ui::MainWindow *ui;
    VersionChecker *m_versionChecker;
    void setupUI();
};
#endif // MAINWINDOW_H
