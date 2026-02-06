#include "progresswindow.h"

ProgressWindow::ProgressWindow(QWidget *parent)
    : QDialog(parent)
{
    // 窗口基础设置
    setWindowTitle("执行进度");
    setFixedSize(600, 200);
    setModal(true);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 1. 进度条初始化
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #cccccc;
            border-radius: 4px;
            text-align: center;
            font-size: 12px;
            color: #333333;
            background-color: #f0f0f0;
        }
        QProgressBar::chunk {
            border-radius: 3px;
            background-color: #00cc00;
        }
    )");

    // 2. 日志显示框（修复QTextEdit兼容问题）
    m_logTextEdit = new QTextEdit(this);
    m_logTextEdit->setReadOnly(true);
    m_logTextEdit->setFont(QFont("Consolas", 10));
    m_logTextEdit->setStyleSheet(R"(
        QTextEdit {
            border: 1px solid #cccccc;
            border-radius: 4px;
            background-color: #ffffff;
            color: #000000;
        }
    )");
    // 移除setMaximumBlockCount（QTextEdit无此函数），改用手动清理日志的兼容方案
    m_logTextEdit->document()->setMaximumBlockCount(1000);  // 正确的写法：通过document()设置

    // 3. 布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->addWidget(m_logTextEdit);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(m_progressBar);
    setLayout(m_mainLayout);

    // 初始日志
    appendLog("开始执行强制执行操作...");
}

ProgressWindow::~ProgressWindow() = default;

// 更新进度条+强制刷新UI
void ProgressWindow::updateProgress(int value, int maxValue)
{
    m_progressBar->setRange(0, maxValue);
    m_progressBar->setValue(value);
    m_progressBar->setFormat(QString("进度：%1/%2 (%3%)")
                                 .arg(value)
                                 .arg(maxValue)
                                 .arg(maxValue > 0 ? (value * 100 / maxValue) : 0));
    // 修复qApp调用（已引入QApplication头文件）
    QApplication::processEvents();  // 等价于qApp->processEvents()，更兼容
}

// 添加日志+自动滚动+强制刷新
void ProgressWindow::appendLog(const QString &log)
{
    m_logTextEdit->append(QString("[%1] %2")
                              .arg(QTime::currentTime().toString("HH:mm:ss"))
                              .arg(log));
    // 自动滚动到最新日志
    m_logTextEdit->verticalScrollBar()->setValue(m_logTextEdit->verticalScrollBar()->maximum());
    // 强制刷新UI（修复qApp调用）
    QApplication::processEvents();
}

// 执行完成处理
void ProgressWindow::finishProgress()
{
    appendLog("强制执行操作完成！");
    QMessageBox::information(this, "执行完成",
                             "已完成3次全量电子教室进程关闭！\n若电子教室窗口仍然存在，请将此问题报告给软件开发者。");
    close();
}
