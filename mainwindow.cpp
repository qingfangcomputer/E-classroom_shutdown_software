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
#include "progresswindow.h"  // 确保包含进度窗口头文件
#include "killprocessthread.h"  // 确保包含线程头文件

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_progressWindow(nullptr)
    , m_killThread(nullptr)
{
    ui->setupUi(this);
    m_versionChecker = new VersionChecker(this);

    // 保存原始窗口标题（用于追加“有限的体验”）
    m_originalWindowTitle = this->windowTitle();

    // 连接服务器/版本检查信号（核心：修复服务器离线逻辑+标题追加）
    connect(m_versionChecker, &VersionChecker::serverOnline, this, [=]() {
        qDebug() << "服务器在线";
    });
    connect(m_versionChecker, &VersionChecker::serverOffline, this, [=]() {
        // 弹出带确定/取消的提示框
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setWindowTitle("服务提示");
        msgBox.setText("检测到qingfangcomputer.com不在线，将提供有限的服务<br>或您没有连接到互联网！");
        // 设置按钮文本：确定、取消
        msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
        msgBox.setButtonText(QMessageBox::Ok, "确定");
        msgBox.setButtonText(QMessageBox::Cancel, "取消");
        msgBox.setDefaultButton(QMessageBox::Ok);

        // 处理按钮点击
        int ret = msgBox.exec();
        if (ret == QMessageBox::Cancel) {
            // 点击取消：退出应用
            QApplication::quit();
        } else if (ret == QMessageBox::Ok) {
            // 点击确定：追加“有限的体验”到标题
            QString newTitle = QString("%1 ❌有限的体验").arg(m_originalWindowTitle);
            this->setWindowTitle(newTitle);
        }
    });
    connect(m_versionChecker, &VersionChecker::newVersionAvailable, this, &MainWindow::onNewVersionAvailable);
    connect(m_versionChecker, &VersionChecker::noUpdatesAvailable, this, &MainWindow::onNoUpdatesAvailable);

    m_versionChecker->checkServerAvailability();
    m_versionChecker->checkForUpdates("4.3.1");
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
        m_progressWindow->deleteLater();  // 延迟释放，避免UI卡顿
        m_progressWindow = nullptr;
    }
    // 重置计数器
    m_clickCount = 0;
    // 释放子线程
    if (m_killThread) {
        m_killThread->quit();
        m_killThread->wait();
        m_killThread->deleteLater();
        m_killThread = nullptr;
    }
}

// 强制执行关闭（核心修改：传入QMap类型的进程列表+防止重复点击）
void MainWindow::forceKillAllClassroomProcesses()
{
    // 防止重复点击创建多个线程/窗口
    if (m_killThread && m_killThread->isRunning()) {
        QMessageBox::warning(this, "提示", "正在执行进程关闭操作，请等待完成！");
        return;
    }

    // 创建进度窗口
    if (m_progressWindow) {
        m_progressWindow->deleteLater();
    }
    m_progressWindow = new ProgressWindow(this);
    m_progressWindow->show();

    // 修正：传入QMap类型的m_classroomProcesses（匹配线程构造函数）
    m_killThread = new KillProcessThread(m_classroomProcesses, 3, this);
    // 连接子线程信号到主窗口槽函数
    connect(m_killThread, &KillProcessThread::logUpdated, this, &MainWindow::onThreadLogUpdated);
    connect(m_killThread, &KillProcessThread::progressUpdated, this, &MainWindow::onThreadProgressUpdated);
    connect(m_killThread, &KillProcessThread::finishedKill, this, &MainWindow::onThreadFinished);

    // 启动子线程
    m_killThread->start();
}

// 核心：关闭按钮点击逻辑（修复QMap迭代器调用）
void MainWindow::on_commandLinkButton_clicked()
{
    QString runningClassroom;
    QString runningProcess;

    // 修正：QMap迭代器调用key()/value()（原错误是用了QStringList）
    for (auto it = m_classroomProcesses.constBegin(); it != m_classroomProcesses.constEnd(); ++it) {
        if (isProcessRunning(it.key())) {  // QMap迭代器可以调用key()
            runningProcess = it.key();
            runningClassroom = it.value();  // QMap迭代器可以调用value()
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

// 以下函数保持不变，仅确保编译通过
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
    // 静默处理，不弹窗
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
        QProcess taskkill;
        taskkill.start("taskkill", QStringList() << "/F" << "/T" << "/IM" << processName);
        taskkill.waitForFinished(2000);
        retryCount++;
        QThread::msleep(800);
    }
    return !isProcessRunning(processName);
}
