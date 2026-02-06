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
#include <QDir>
#include <QRandomGenerator>
#include <QImage>
#include <QResizeEvent>
#include <QRegularExpression>
#include <QVBoxLayout>
#include <QTime>
#include "stop.h"
#include "up.h"
#include "versionchecker.h"
#include "help.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_progressWindow(nullptr)
    , m_killThread(nullptr)
{
    ui->setupUi(this);
    m_versionChecker = new VersionChecker(this);

    // 连接服务器/版本检查信号（保留）
    connect(m_versionChecker, &VersionChecker::serverOnline, this, [=]() {
        qDebug() << "服务器在线";
    });
    connect(m_versionChecker, &VersionChecker::serverOffline, this, [=]() {
        QMessageBox::critical(this, "服务不可用", "qingfangcomputer.com不在线，无法提供服务<br>或您没有连接到互联网！");
        QApplication::quit();
    });
    connect(m_versionChecker, &VersionChecker::newVersionAvailable, this, &MainWindow::onNewVersionAvailable);
    connect(m_versionChecker, &VersionChecker::noUpdatesAvailable, this, &MainWindow::onNoUpdatesAvailable);

    m_versionChecker->checkServerAvailability();
    m_versionChecker->checkForUpdates("4.2.6");
    setupUI();
}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_versionChecker;
    // 释放子线程和进度窗口
    if (m_killThread) {
        m_killThread->quit();
        m_killThread->wait();
        delete m_killThread;
    }
    if (m_progressWindow) {
        delete m_progressWindow;
    }
}

// 子线程日志更新 → 转发给进度窗口
void MainWindow::onThreadLogUpdated(const QString &log)
{
    if (m_progressWindow) {
        m_progressWindow->appendLog(log);
    }
}

// 子线程进度更新 → 转发给进度窗口
void MainWindow::onThreadProgressUpdated(int current, int total)
{
    if (m_progressWindow) {
        m_progressWindow->updateProgress(current, total);
    }
}

// 子线程执行完成 → 通知进度窗口
void MainWindow::onThreadFinished()
{
    if (m_progressWindow) {
        m_progressWindow->finishProgress();
    }
    // 重置计数器
    m_clickCount = 0;
    // 释放子线程
    m_killThread->deleteLater();
    m_killThread = nullptr;
}

// 强制执行关闭（核心修改：启动子线程）
void MainWindow::forceKillAllClassroomProcesses()
{
    // 创建进度窗口
    m_progressWindow = new ProgressWindow(this);
    m_progressWindow->show();

    // 创建子线程（传入进程列表+3轮）
    m_killThread = new KillProcessThread(m_classroomProcesses, 3, this);
    // 连接子线程信号到主窗口槽函数
    connect(m_killThread, &KillProcessThread::logUpdated, this, &MainWindow::onThreadLogUpdated);
    connect(m_killThread, &KillProcessThread::progressUpdated, this, &MainWindow::onThreadProgressUpdated);
    connect(m_killThread, &KillProcessThread::finishedKill, this, &MainWindow::onThreadFinished);

    // 启动子线程（关键：耗时操作在子线程执行，不卡UI）
    m_killThread->start();
}

// 核心：关闭按钮点击逻辑（保留，仅调用forceKillAllClassroomProcesses）
void MainWindow::on_commandLinkButton_clicked()
{
    QString runningClassroom;
    QString runningProcess;

    // 检测运行的电子教室（保留）
    for (auto it = m_classroomProcesses.constBegin(); it != m_classroomProcesses.constEnd(); ++it) {
        if (isProcessRunning(it.key())) {
            runningProcess = it.key();
            runningClassroom = it.value();
            break;
        }
    }

    if (!runningClassroom.isEmpty()) {
        m_clickCount = 0;
        qDebug() << QString("检测到%1（进程%2），开始关闭").arg(runningClassroom).arg(runningProcess);
        bool killSuccess = killProcessWithRetry(runningProcess, 3);

        if (killSuccess) {
            QMessageBox::information(this, "操作成功", QString("%1 关闭成功！").arg(runningClassroom));
        } else {
            QMessageBox::critical(this, "操作失败",
                                  QString("%1 关闭失败！<br>解决方案：<br>1. 关闭安全软件<br>2. 管理员运行本软件<br>3. 手动结束进程（%2）").arg(runningClassroom).arg(runningProcess));
        }
    } else {
        m_clickCount++;
        qDebug() << QString("未检测到运行中的电子教室，当前点击次数：%1").arg(m_clickCount);

        if (m_clickCount >= 3) {
            QMessageBox::StandardButton result = QMessageBox::question(
                this, "强制执行确认",
                "检测到连续点击三次关闭按钮，可能软件未检测到电子教室正在运行！\n点击确定将强制执行所有电子教室的关闭代码。",
                QMessageBox::Ok | QMessageBox::Cancel,
                QMessageBox::Cancel
                );

            if (result == QMessageBox::Ok) {
                forceKillAllClassroomProcesses();  // 启动子线程+进度窗口
            } else {
                m_clickCount = 0;
            }
        } else {
            QMessageBox::information(this, "检测结果", "未检测到运行中的电子教室客户端！");
        }
    }
}

// 以下保留原有未修改的函数（listResourceImages、setRandomBackground、resizeEvent、setupUI、onNewVersionAvailable、onNoUpdatesAvailable、on_pushButton_2/3/4_clicked、isProcessRunning、killProcessWithRetry）
// ===== 以下是原有函数的完整代码 =====
void MainWindow::listResourceImages(const QString &path, QStringList &outList)
{
    QDir dir(path);
    if (!dir.exists()) {
        qWarning() << "资源路径不存在：" << path;
        return;
    }

    QStringList filters;
    filters << "*.jpg" << "*.png" << "*.bmp" << "*.jpeg" << "*.gif";
    dir.setNameFilters(filters);
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot);

    QFileInfoList fileList = dir.entryInfoList();
    for (const QFileInfo &fileInfo : fileList) {
        outList.append(fileInfo.filePath());
    }
}

void MainWindow::setRandomBackground()
{
    QStringList backgroundImages;
    listResourceImages(":/wallpaper/", backgroundImages);

    if (backgroundImages.isEmpty()) {
        QMessageBox::warning(this, "提示", "未找到背景图片资源，请检查 :/wallpaper/ 路径");
        return;
    }

    int index;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    index = QRandomGenerator::global()->bounded(backgroundImages.size());
#else
    qsrand(QTime::currentTime().msec());
    index = qrand() % backgroundImages.size();
#endif

    QString selectedImagePath = backgroundImages.at(index);

    QImage backgroundImage(selectedImagePath);
    if (backgroundImage.isNull()) {
        QMessageBox::warning(this, "错误", "背景图片加载失败：" + selectedImagePath);
        return;
    }

    QPalette palette = this->palette();
    QBrush brush(backgroundImage.scaled(
        this->size(),
        Qt::KeepAspectRatioByExpanding,
        Qt::SmoothTransformation
        ));
    brush.setStyle(Qt::TexturePattern);
    palette.setBrush(QPalette::Window, brush);
    this->setPalette(palette);

    if (ui->centralwidget) {
        ui->centralwidget->setAutoFillBackground(false);
        ui->centralwidget->raise();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    setRandomBackground();
}

void MainWindow::setupUI()
{
    setRandomBackground();

    QString styleSheet = R"(
        QWidget#centralwidget {
            background: transparent;
            border: none;
        }
        QLabel {
            font-size: 14px;
        }
        QGroupBox {
            border: 1px solid #ccc;
            border-radius: 6px;
            margin-top: 10px;
            padding: 10px;
            background-color: rgba(255, 255, 255, 0.8);
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 10px;
            padding: 0 5px 0 5px;
            background-color: transparent;
        }
        QStatusBar {
            background: transparent;
            border: none;
        }
    )";
    setStyleSheet(styleSheet);
}

void MainWindow::onNewVersionAvailable(QString version)
{
    QMessageBox::information(this, "更新提示", "治于神者，众人不知其功；争于明者，众人知之。——《墨子·50章 公输》<br>有新版本可以升级！ 可更新的版本: v" + version);
    up *upWindow = new up();
    upWindow->setStyleSheet(this->styleSheet());
    upWindow->show();
}

void MainWindow::onNoUpdatesAvailable()
{
    // QMessageBox::information(this, "版本信息", "当前已是最新版本");
}

void MainWindow::on_pushButton_3_clicked()
{
    help *helpWindow = new help();
    helpWindow->setStyleSheet(this->styleSheet());
    helpWindow->show();
}

void MainWindow::on_pushButton_4_clicked()
{
    QApplication::quit();
}

bool MainWindow::isProcessRunning(const QString &processName)
{
    QProcess tasklist;
    tasklist.start("tasklist", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(processName) << "/NH" << "/FO" << "CSV");
    tasklist.waitForFinished(3000);
    QString output = tasklist.readAllStandardOutput().toLower();

    QProcess wmic;
    wmic.start("wmic", QStringList() << "process" << "where" << QString("name='%1'").arg(processName) << "get" << "name");
    wmic.waitForFinished(3000);
    QString wmicOutput = wmic.readAllStandardOutput().toLower();

    return output.contains(processName.toLower()) || wmicOutput.contains(processName.toLower());
}

bool MainWindow::killProcessWithRetry(const QString &processName, int maxRetry)
{
    int retryCount = 0;
    while (retryCount < maxRetry) {
        if (!isProcessRunning(processName)) {
            return true;
        }
        // 直接调用子线程的killSingleProcess逻辑（简化）
        QProcess taskkill;
        taskkill.start("taskkill", QStringList() << "/F" << "/T" << "/IM" << processName);
        taskkill.waitForFinished(2000);
        retryCount++;
        QThread::msleep(800);
    }
    return !isProcessRunning(processName);
}
