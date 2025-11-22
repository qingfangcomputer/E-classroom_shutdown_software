#include "versionchecker.h"
#include <QNetworkReply>
#include <QVersionNumber>

VersionChecker::VersionChecker(QObject *parent) :
    QObject(parent),
    m_networkManager(new QNetworkAccessManager(this)),
    m_currentVersion()
{
}

VersionChecker::~VersionChecker()
{
    delete m_networkManager;
}

void VersionChecker::checkForUpdates(const QString &currentVersion)
{
    m_currentVersion = QVersionNumber::fromString(currentVersion);
    QNetworkRequest request(QUrl("https://qingfangcomputer.top/cloud/up file/jiyu.txt"));
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &VersionChecker::handleNetworkReply);
}

// 新增：检查服务器可用性
void VersionChecker::checkServerAvailability()
{
    QNetworkRequest request(QUrl("https://qingfangcomputer.com"));
    // 设置超时时间
    request.setTransferTimeout(5000);
    QNetworkReply *reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &VersionChecker::handleServerCheckReply);
}

void VersionChecker::handleNetworkReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply && reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString version = data.data();
        QVersionNumber serverVersion = QVersionNumber::fromString(version);
        if (serverVersion > m_currentVersion) {
            emit newVersionAvailable(version);
        } else {
            emit noUpdatesAvailable();
        }
    } else if (reply) {
        emit noUpdatesAvailable();
    }
    reply->deleteLater();
}

// 新增：处理服务器检查响应
void VersionChecker::handleServerCheckReply()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (reply) {
        if (reply->error() == QNetworkReply::NoError) {
            emit serverOnline();
        } else {
            emit serverOffline();
        }
        reply->deleteLater();
    }
}
