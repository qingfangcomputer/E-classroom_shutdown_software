#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QApplication>
#include <QPushButton>
#include <QProcess>
#include <QPalette>
#include <QStyle>
#include <QDesktopServices>
#include <QUrl>
#include "stop.h"
#include "up.h"
#include "versionchecker.h"
#include "help.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_versionChecker = new VersionChecker(this);

    // 连接服务器检查信号
    connect(m_versionChecker, &VersionChecker::serverOnline, this, [=]() {
        qDebug() << "服务器在线";
    });
    connect(m_versionChecker, &VersionChecker::serverOffline, this, [=]() {
        // 显示错误消息，并在用户点击OK后退出应用
        QMessageBox::critical(this, "服务不可用", "qingfangcomputer.com不在线，无法提供服务");
        // 退出应用
        QApplication::quit();
    });

    // 连接版本检查信号
    connect(m_versionChecker, &VersionChecker::newVersionAvailable, this, &MainWindow::onNewVersionAvailable);
    connect(m_versionChecker, &VersionChecker::noUpdatesAvailable, this, &MainWindow::onNoUpdatesAvailable);

    // 先检查服务器可用性，再检查版本
    m_versionChecker->checkServerAvailability();
    m_versionChecker->checkForUpdates("3.0.5"); // 软件当前版本号

    // 界面美化
    setupUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_versionChecker;
}

// 新增：UI美化设置
void MainWindow::setupUI()
{
    // 设置窗口标题和大小
    setWindowTitle("极域电子教室控制工具");
    setMinimumSize(600, 400);

    // 设置样式表 - 美化按钮和窗口
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f0f0f0;
        }
        QPushButton, QCommandLinkButton {
            background-color: #4CAF50;
            color: white;
            border-radius: 6px;
            padding: 8px 16px;
            font-size: 14px;
            border: none;
        }
        QPushButton:hover, QCommandLinkButton:hover {
            background-color: #45a049;
        }
        QPushButton:pressed, QCommandLinkButton:pressed {
            background-color: #3d8b40;
        }
        QLabel {
            font-size: 14px;
        }
        QGroupBox {
            border: 1px solid #ccc;
            border-radius: 6px;
            margin-top: 10px;
            padding: 10px;
            background-color: #ffffff;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: transparent;
        }
    )";
    setStyleSheet(styleSheet);

    // 如果有状态栏，可以设置状态栏样式
    statusBar()->setStyleSheet("QStatusBar { background-color: #e0e0e0; }");
}

void MainWindow::onNewVersionAvailable(QString version)
{
    QMessageBox::information(this, "更新提示", "治于神者，众人不知其功；争于明者，众人知之。——《墨子·50章 公输》<br>有新版本可以升级！ 可更新的版本: v" + version);
    up *upWindow = new up();
    upWindow->setStyleSheet(this->styleSheet()); // 应用相同样式
    upWindow->show();
}

void MainWindow::onNoUpdatesAvailable()
{
    // 可以显示当前是最新版本的提示
    // QMessageBox::information(this, "版本信息", "当前已是最新版本");
}


void MainWindow::on_pushButton_3_clicked()
{
    stop *stopWindow = new stop();
    stopWindow->setStyleSheet(this->styleSheet()); // 应用相同样式
    stopWindow->show();
}

void MainWindow::on_commandLinkButton_clicked()
{
    // 创建一个QProcess实例
    QProcess process;
    // 设置要执行的命令
    QString program = "cmd";
    QStringList arguments;
    arguments << "/c" << "taskkill /f /t /im StudentMain.exe";
    //杀死一个名为StudentMain.exe的进程（彻底杀死）

    // 启动进程
    process.start(program, arguments);

    // 等待进程完成
    process.waitForFinished();

    // 读取进程的输出
    QString output = process.readAllStandardOutput();
    qDebug() << "CMD Output:" << output;

    // 使用新的QMessageBox重载形式，指定按钮
    QMessageBox::information(this,
                             "极域电子教室关闭软件.exe",
                             "执行成功<br>若极域电子教室学生端浮动工具栏消失，则极域电子教室学生端关闭成功。",
                             QMessageBox::Ok); // 使用StandardButtons枚举
}

