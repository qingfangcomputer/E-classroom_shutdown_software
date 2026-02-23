#include "KillProcessThread.h"
#include <QProcess>
#include <QThread>
#include <QDebug>

KillProcessThread::KillProcessThread(const QMap<QString, QString> &processMap, int totalRounds, QObject *parent)
    : QThread(parent)
    , m_processMap(processMap)
    , m_totalRounds(totalRounds)
{
}

KillProcessThread::~KillProcessThread()
{
    // 确保线程退出
    quit();
    wait();
}

// 线程核心逻辑：执行多轮进程关闭（修复进度计算）
void KillProcessThread::run()
{
    int totalProgress = m_totalRounds * m_processMap.size() * 100;  // 总进度（轮次×进程数×100）
    int currentProgress = 0;

    for (int round = 1; round <= m_totalRounds; round++) {
        emit logUpdated(QString("===== 执行第%1轮全量进程关闭 =====").arg(round));

        // 遍历所有进程执行关闭
        for (auto it = m_processMap.constBegin(); it != m_processMap.constEnd(); ++it) {
            QString processName = it.key();
            QString className = it.value();
            killSingleProcess(processName, className);

            currentProgress += 100;
            emit progressUpdated(currentProgress, totalProgress);  // 每关闭一个进程更新进度
            QThread::msleep(200);  // 短延迟，避免操作过快（非阻塞主线程）
        }

        emit logUpdated(QString("第%1轮关闭完成，等待1秒...").arg(round));
        QThread::msleep(1000);  // 轮次间隔（子线程内sleep，不影响UI）
    }

    emit logUpdated("所有轮次执行完成！");
    emit progressUpdated(totalProgress, totalProgress);
    emit finishedKill();  // 通知主线程执行完成
}

// 关闭单个进程（添加失败日志反馈）
void KillProcessThread::killSingleProcess(const QString &processName, const QString &className)
{
    emit logUpdated(QString("正在关闭 %1（进程：%2）").arg(className).arg(processName));

    // 方式1：常规taskkill
    QProcess taskkill;
    taskkill.start("taskkill", QStringList() << "/F" << "/T" << "/IM" << processName);
    taskkill.waitForFinished(2000);
    QString taskkillOutput = taskkill.readAllStandardOutput() + taskkill.readAllStandardError();

    // 方式2：wmic强制终止
    QProcess wmicKill;
    wmicKill.start("wmic", QStringList() << "process" << "where" << QString("name='%1'").arg(processName) << "delete");
    wmicKill.waitForFinished(2000);
    QString wmicOutput = wmicKill.readAllStandardOutput() + wmicKill.readAllStandardError();

    // 方式3：管理员权限执行
    runCommandAsAdmin(QString("taskkill /F /T /IM %1").arg(processName));

    // 日志反馈执行结果
    if (taskkillOutput.contains("成功") || wmicOutput.contains("删除")) {
        emit logUpdated(QString("✅ 成功关闭 %1（进程：%2）").arg(className).arg(processName));
    } else {
        emit logUpdated(QString("❌ 关闭 %1失败：taskkill=%2 | wmic=%3").arg(className).arg(taskkillOutput.trimmed()).arg(wmicOutput.trimmed()));
    }

    qDebug() << QString("关闭进程%1：taskkill输出=%2, wmic输出=%3")
                    .arg(processName)
                    .arg(taskkillOutput)
                    .arg(wmicOutput);
}

// 管理员权限执行命令（无UI操作）
void KillProcessThread::runCommandAsAdmin(const QString &command)
{
    QString psCommand = QString("Start-Process cmd -ArgumentList '/c %1' -Verb RunAs").arg(command);
    QProcess::startDetached("powershell", QStringList() << "-Command" << psCommand);
}
