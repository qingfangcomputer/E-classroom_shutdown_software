#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QRandomGenerator>
#include <QImage>
#include <QPalette>
#include <QMessageBox>
#include <QProcess>
#include <QThread>
#include <QRegularExpression>
#include <QMap>
#include <QTimer>
#include "versionchecker.h"
#include "progresswindow.h"
#include "KillProcessThread.h"  // 引入子线程

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
    void on_pushButton_3_clicked();
    void on_pushButton_4_clicked();
    void on_commandLinkButton_clicked();
    // 新增：接收子线程的日志/进度/完成信号
    void onThreadLogUpdated(const QString &log);
    void onThreadProgressUpdated(int current, int total);
    void onThreadFinished();

private:
    Ui::MainWindow *ui;
    VersionChecker *m_versionChecker;
    int m_clickCount = 0;                // 点击计数器
    ProgressWindow *m_progressWindow;   // 进度窗口
    KillProcessThread *m_killThread;    // 进程关闭子线程

    // 电子教室进程列表（全局）
    const QMap<QString, QString> m_classroomProcesses = {
        {"StudentMain.exe", "极域电子教室"},
        {"Student.exe", "红蜘蛛电子教室"},
        {"RedSpiderStudent.exe", "红蜘蛛电子教室"},
        {"LanStarStudent.exe", "蓝星电子教室"},
        {"NetOpStudent.exe", "NetOp电子教室"},
        {"ClassInStudent.exe", "ClassIn电子教室"},
        {"SmartClassroomStudent.exe", "智慧教室学生端"},
        {"e-LearningStudent.exe", "易乐学电子教室"},
        {"MultimediaClassroom.exe", "多媒体电子教室"}
    };

    void setupUI();
    void setRandomBackground();
    void listResourceImages(const QString &path, QStringList &outList);
    void resizeEvent(QResizeEvent *event) override;

    // 进程检测函数（保留）
    bool isProcessRunning(const QString &processName);
    bool killProcessWithRetry(const QString &processName, int maxRetry = 3);
    // 强制执行关闭（启动子线程）
    void forceKillAllClassroomProcesses();
};
#endif // MAINWINDOW_H
