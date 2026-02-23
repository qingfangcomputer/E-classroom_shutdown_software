#ifndef PROGRESSWINDOW_H
#define PROGRESSWINDOW_H

#include <QDialog>
#include <QProgressBar>
#include <QTextEdit>       // 保留QTextEdit（无需替换）
#include <QVBoxLayout>
#include <QString>
#include <QTime>
#include <QScrollBar>
#include <QFont>
#include <QMessageBox>
#include <QApplication>    // 新增：引入qApp所需头文件
#include <QtWidgets>       // 兜底：确保所有QtWidgets组件被识别

class ProgressWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressWindow(QWidget *parent = nullptr);
    ~ProgressWindow() override;

public slots:
    void updateProgress(int value, int maxValue);
    // 添加日志+自动滚动+强制刷新
    void appendLog(const QString &log);
    void finishProgress();

private:
    QProgressBar *m_progressBar;
    QTextEdit *m_logTextEdit;
    QVBoxLayout *m_mainLayout;
};

#endif // PROGRESSWINDOW_H
