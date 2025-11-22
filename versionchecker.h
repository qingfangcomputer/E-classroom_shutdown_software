#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>
#include <QVersionNumber>

class VersionChecker : public QObject
{
    Q_OBJECT

public:
    explicit VersionChecker(QObject *parent = nullptr);
    ~VersionChecker();

    void checkForUpdates(const QString &currentVersion);
    void checkServerAvailability(); // 新增：检查服务器可用性

signals:
    void newVersionAvailable(QString version);
    void noUpdatesAvailable();
    void serverOnline(); // 新增：服务器在线信号
    void serverOffline(); // 新增：服务器离线信号

public slots:
    void handleNetworkReply();
    void handleServerCheckReply(); // 新增：处理服务器检查响应

private:
    QNetworkAccessManager *m_networkManager;
    QVersionNumber m_currentVersion;
};

#endif // VERSIONCHECKER_H
