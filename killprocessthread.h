#ifndef KILLPROCESSTHREAD_H
#define KILLPROCESSTHREAD_H

#include <QThread>
#include <QString>
#include <QMap>

// 子线程：执行耗时的进程关闭操作，通过信号通知主线程进度/日志
class KillProcessThread : public QThread
{
    Q_OBJECT

public:
    explicit KillProcessThread(const QMap<QString, QString> &processMap, int totalRounds = 3, QObject *parent = nullptr);
    ~KillProcessThread() override;

signals:
    // 发送实时日志（供进度窗口显示）
    void logUpdated(const QString &log);
    // 发送进度更新（当前进度/总进度）
    void progressUpdated(int current, int total);
    // 线程执行完成
    void finishedKill();

protected:
    void run() override;  // 线程核心执行函数

private:
    QMap<QString, QString> m_processMap;  // 待关闭的进程列表
    int m_totalRounds;                    // 总执行轮次
    // 单个进程关闭（纯函数，无UI操作）
    void killSingleProcess(const QString &processName, const QString &className);
    // 管理员权限执行命令
    void runCommandAsAdmin(const QString &command);
};

#endif // KILLPROCESSTHREAD_H
