#include "mainwindow.h"

#include <QTextEdit>
#include <QTableWidget>
#include <QHeaderView>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QMessageBox>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHostAddress>
#include <QAbstractSocket>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QTime>
#include <QInputDialog>

// 构造函数：初始化 UI 和网络组件
MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    m_textSource(nullptr),
    m_textEncoded(nullptr),
    m_textReceivedLocal(nullptr),
    m_textReceivedServer(nullptr),
    m_textDecoded(nullptr),
    m_tableCodes(nullptr),
    m_treeWidget(nullptr),
    m_listSteps(nullptr),
    m_statusLabel(nullptr),
    m_lblCompressionRatio(nullptr),
    m_server(nullptr),
    m_client(nullptr)
{
    setupUi();
    setupStyle();
    setupToolbar();
    setupDock();

    // 状态栏
    m_statusLabel = new QLabel(this);
    statusBar()->addPermanentWidget(m_statusLabel);
    showStatus(tr("就绪"));

    // Socket 初始化
    m_server = new NetworkServer(this);
    connect(m_server, &NetworkServer::dataReceived, this, &MainWindow::onServerDataReceived);
    connect(m_server, &NetworkServer::clientConnected, this, &MainWindow::onServerClientConnected);
    connect(m_server, &NetworkServer::errorOccurred, this, &MainWindow::onServerError);

    m_client = new NetworkClient(this);
    connect(m_client, &NetworkClient::connected, this, &MainWindow::onClientConnectedToServer);
    connect(m_client, &NetworkClient::dataReceived, this, &MainWindow::onClientDataReceived);
    connect(m_client, &NetworkClient::errorOccurred, this, &MainWindow::onClientError);

    // 一些默认文字，方便测试
    m_textSource->setPlainText(tr("你好专业基础实训！"));
}

MainWindow::~MainWindow()
{
}

// 初始化 UI 界面布局和控件
void MainWindow::setupUi()
{
    resize(1200, 800);
    setWindowTitle(tr("哈夫曼编/译码系统 - C++/Qt 实现"));

    QWidget* central = new QWidget(this);
    setCentralWidget(central);

    // 1. Top Section: Input & Encoded Output
    QGroupBox* groupInput = new QGroupBox(tr("原文输入"), this);
    QVBoxLayout* layoutInput = new QVBoxLayout(groupInput);
    m_textSource = new QTextEdit(this);
    m_textSource->setPlaceholderText(tr("在此输入待编码的文字..."));
    layoutInput->addWidget(m_textSource);

    QGroupBox* groupEncoded = new QGroupBox(tr("编码结果 (0/1)"), this);
    QVBoxLayout* layoutEncoded = new QVBoxLayout(groupEncoded);
    m_textEncoded = new QTextEdit(this);
    m_textEncoded->setPlaceholderText(tr("显示编码后的 0/1 序列..."));
    m_textEncoded->setReadOnly(true);
    layoutEncoded->addWidget(m_textEncoded);
    
    // 压缩率显示
    m_lblCompressionRatio = new QLabel(tr("压缩率: N/A"), this);
    m_lblCompressionRatio->setAlignment(Qt::AlignRight);
    m_lblCompressionRatio->setStyleSheet("color: #409eff; font-weight: bold;");
    layoutEncoded->addWidget(m_lblCompressionRatio);

    QSplitter* splitterTop = new QSplitter(Qt::Horizontal, this);
    splitterTop->addWidget(groupInput);
    splitterTop->addWidget(groupEncoded);
    splitterTop->setStretchFactor(0, 1);
    splitterTop->setStretchFactor(1, 1);

    // 2. Middle Section: Frequency Table & Tree Visualization
    QGroupBox* groupTable = new QGroupBox(tr("字符频率表"), this);
    QVBoxLayout* layoutTable = new QVBoxLayout(groupTable);
    m_tableCodes = new QTableWidget(this);
    m_tableCodes->setColumnCount(3);
    m_tableCodes->setHorizontalHeaderLabels({tr("字符"), tr("频率"), tr("哈夫曼编码")});
    m_tableCodes->horizontalHeader()->setStretchLastSection(true);
    m_tableCodes->setAlternatingRowColors(true);
    m_tableCodes->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tableCodes->setEditTriggers(QAbstractItemView::NoEditTriggers);
    layoutTable->addWidget(m_tableCodes);

    QGroupBox* groupTree = new QGroupBox(tr("哈夫曼树可视化"), this);
    QVBoxLayout* layoutTree = new QVBoxLayout(groupTree);
    m_treeWidget = new HuffmanTreeWidget(this);
    layoutTree->addWidget(m_treeWidget);

    QSplitter* splitterMid = new QSplitter(Qt::Horizontal, this);
    splitterMid->addWidget(groupTable);
    splitterMid->addWidget(groupTree);
    splitterMid->setStretchFactor(0, 1);
    splitterMid->setStretchFactor(1, 2);

    // 3. Bottom Section: Received Data & Decoded Output
    QGroupBox* groupRecvLocal = new QGroupBox(tr("本地接收 (模拟)"), this);
    QVBoxLayout* layoutRecvLocal = new QVBoxLayout(groupRecvLocal);
    m_textReceivedLocal = new QTextEdit(this);
    m_textReceivedLocal->setPlaceholderText(tr("本地直接接收的编码..."));
    m_textReceivedLocal->setReadOnly(true);
    layoutRecvLocal->addWidget(m_textReceivedLocal);

    QGroupBox* groupRecvServer = new QGroupBox(tr("服务器接收 (网络)"), this);
    QVBoxLayout* layoutRecvServer = new QVBoxLayout(groupRecvServer);
    m_textReceivedServer = new QTextEdit(this);
    m_textReceivedServer->setPlaceholderText(tr("从服务器接收的编码..."));
    m_textReceivedServer->setReadOnly(true);
    layoutRecvServer->addWidget(m_textReceivedServer);

    QGroupBox* groupDecoded = new QGroupBox(tr("译码结果"), this);
    QVBoxLayout* layoutDecoded = new QVBoxLayout(groupDecoded);
    m_textDecoded = new QTextEdit(this);
    m_textDecoded->setPlaceholderText(tr("最终译码还原的文字..."));
    m_textDecoded->setReadOnly(true);
    layoutDecoded->addWidget(m_textDecoded);

    QSplitter* splitterBottom = new QSplitter(Qt::Horizontal, this);
    splitterBottom->addWidget(groupRecvLocal);
    splitterBottom->addWidget(groupRecvServer);
    splitterBottom->addWidget(groupDecoded);

    // Main Layout
    QSplitter* splitterMain = new QSplitter(Qt::Vertical, this);
    splitterMain->addWidget(splitterTop);
    splitterMain->addWidget(splitterMid);
    splitterMain->addWidget(splitterBottom);
    splitterMain->setStretchFactor(0, 1);
    splitterMain->setStretchFactor(1, 2);
    splitterMain->setStretchFactor(2, 1);

    QVBoxLayout* mainLayout = new QVBoxLayout(central);
    mainLayout->addWidget(splitterMain);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Connect selection change for path highlighting
    connect(m_textEncoded, &QTextEdit::selectionChanged, [this](){
        QString selected = m_textEncoded->textCursor().selectedText();
        QString cleanCode;
        for(QChar c : selected) {
            if(c == '0' || c == '1') cleanCode.append(c);
        }
        if(!cleanCode.isEmpty()) {
            m_treeWidget->highlightPath(cleanCode);
        }
    });
}

void MainWindow::setupStyle()
{
    // Modern Flat Style
    QString qss = R"(
        QMainWindow {
            background-color: #f0f2f5;
        }
        QGroupBox {
            font-weight: bold;
            border: 1px solid #dcdfe6;
            border-radius: 6px;
            margin-top: 12px;
            background-color: #ffffff;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px;
            padding: 0 5px;
            color: #303133;
        }
        QTextEdit, QTableWidget, QListWidget {
            border: 1px solid #e4e7ed;
            border-radius: 4px;
            background-color: #ffffff;
            selection-background-color: #409eff;
            font-family: "Consolas", "Microsoft YaHei";
            font-size: 10pt;
        }
        QHeaderView::section {
            background-color: #f5f7fa;
            padding: 4px;
            border: none;
            border-bottom: 1px solid #e4e7ed;
            border-right: 1px solid #e4e7ed;
            font-weight: bold;
            color: #606266;
        }
        QSplitter::handle {
            background-color: #dcdfe6;
            width: 2px;
            height: 2px;
        }
        QStatusBar {
            background-color: #ffffff;
            color: #606266;
        }
        QLabel {
            color: #606266;
        }
    )";
    this->setStyleSheet(qss);
}

// 初始化工具栏和操作按钮
void MainWindow::setupToolbar()
{
    QToolBar* toolbar = addToolBar(tr("工具"));
    toolbar->setMovable(false);
    toolbar->setStyleSheet(
        "QToolBar { background:#e2e8f5; border-bottom:1px solid #a5b3d6; }"
        "QToolButton { padding:4px 10px; margin:2px; }"
        "QToolButton:hover { background:#d0dcf5; }"
        );

    QAction* actBuildTree = toolbar->addAction(tr("统计并构造树"));
    QAction* actEncode    = toolbar->addAction(tr("对原文编码"));
    QAction* actSendLocal = toolbar->addAction(tr("本地发送"));
    QAction* actDecode    = toolbar->addAction(tr("本地译码"));
    QAction* actCompare   = toolbar->addAction(tr("比较校验"));
    toolbar->addSeparator();
    QAction* actOpenFile  = toolbar->addAction(tr("打开文件"));
    QAction* actSaveFile  = toolbar->addAction(tr("保存编码"));
    toolbar->addSeparator();
    QAction* actStartServer = toolbar->addAction(tr("启动服务器"));
    QAction* actStopServer  = toolbar->addAction(tr("关闭服务器"));
    QAction* actConnect     = toolbar->addAction(tr("连接服务器"));
    QAction* actSendSocket  = toolbar->addAction(tr("客户端发送"));

    connect(actBuildTree, &QAction::triggered, this, &MainWindow::onBuildTree);
    connect(actEncode,    &QAction::triggered, this, &MainWindow::onEncode);
    connect(actSendLocal, &QAction::triggered, this, &MainWindow::onSendLocal);
    connect(actDecode,    &QAction::triggered, this, &MainWindow::onDecodeLocal);
    connect(actCompare,   &QAction::triggered, this, &MainWindow::onCompare);
    connect(actOpenFile,  &QAction::triggered, this, &MainWindow::onOpenFile);
    connect(actSaveFile,  &QAction::triggered, this, &MainWindow::onSaveFile);
    connect(actStartServer, &QAction::triggered, this, &MainWindow::onStartServer);
    connect(actStopServer,  &QAction::triggered, this, &MainWindow::onStopServer);
    connect(actConnect,     &QAction::triggered, this, &MainWindow::onConnectToServer);
    connect(actSendSocket,  &QAction::triggered, this, &MainWindow::onSendViaSocket);
}

// 初始化停靠窗口，用于显示 Huffman 树生成步骤
void MainWindow::setupDock()
{
    QDockWidget* dock = new QDockWidget(tr("哈夫曼树生成过程"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_listSteps = new QListWidget(dock);
    m_listSteps->setStyleSheet(
        "QListWidget { background:#ffffff; border:1px solid #b0c4de; }"
        );
    dock->setWidget(m_listSteps);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

// 在状态栏显示信息
void MainWindow::showStatus(const QString& text)
{
    if (m_statusLabel)
        m_statusLabel->setText(text);
}

// 统计字符频率并构造 Huffman 树
void MainWindow::onBuildTree()
{
    QString text = m_textSource->toPlainText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先在原文输入框中输入文本。"));
        return;
    }

    m_freqMap = m_codec.buildFrequency(text);
    m_codec.buildTree(m_freqMap);
    m_codes = m_codec.buildCodes();

    updateCodeTable();
    updateStepsList();
    updateTreeView();

    showStatus(tr("已构造 Huffman 树"));
}
// 刷新字符频率及编码表
void MainWindow::updateCodeTable()
{
    m_tableCodes->clearContents();
    m_tableCodes->setRowCount(m_freqMap.size());

    QList<QChar> keys = m_freqMap.keys();
    std::sort(keys.begin(), keys.end(), [this](const QChar& a, const QChar& b){
        int fa = m_freqMap.value(a);
        int fb = m_freqMap.value(b);
        if (fa != fb) return fa > fb;
        return a < b;
    });

    int row = 0;
    for (QChar c : keys) {
        int freq = m_freqMap.value(c);
        QString code = m_codes.value(c);

        QString displayChar;
        if (c == ' ') displayChar = "[space]";
        else if (c == '\n') displayChar = "[\\n]";
        else if (c == '\t') displayChar = "[\\t]";
        else displayChar = QString(c);

        m_tableCodes->setItem(row, 0, new QTableWidgetItem(displayChar));
        m_tableCodes->setItem(row, 1, new QTableWidgetItem(QString::number(freq)));
        m_tableCodes->setItem(row, 2, new QTableWidgetItem(code));
        row++;
    }
    m_tableCodes->resizeColumnsToContents();
}
// 刷新构造步骤列表
void MainWindow::updateStepsList()
{
    m_listSteps->clear();
    const auto& steps = m_codec.steps();
    int stepNum = 1;
    for (const auto& s : steps) {
        QString line = tr("步骤 %1: 合并 [\"%2\" (%3)] 和 [\"%4\" (%5)] -> 频率 %6")
                           .arg(stepNum++)
                           .arg(s.leftChars)
                           .arg(s.leftFreq)
                           .arg(s.rightChars)
                           .arg(s.rightFreq)
                           .arg(s.mergedFreq);
        m_listSteps->addItem(line);
    }
}

// 刷新 Huffman 树视图
void MainWindow::updateTreeView()
{
    m_treeWidget->setRoot(m_codec.root());
}

// 对原文进行编码
void MainWindow::onEncode()
{
    if (m_codes.isEmpty()) {
        onBuildTree();
        if (m_codes.isEmpty())
            return;
    }

    QString text = m_textSource->toPlainText();
    QString bits = m_codec.encode(text);
    m_textEncoded->setPlainText(bits);
    
    updateCompressionRatio();
    showStatus(tr("已对原文进行编码"));
}

// 本地发送：将编码结果复制到本地接收区
void MainWindow::onSendLocal()
{
    QString bits = m_textEncoded->toPlainText().trimmed();
    m_textReceivedLocal->setPlainText(bits);
    showStatus(tr("已本地发送（复制编码到接收区）"));
}

// 本地译码：对本地接收区的 0/1 序列进行译码
void MainWindow::onDecodeLocal()
{
    QString bits = m_textReceivedLocal->toPlainText().trimmed();
    if (bits.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("接收区为空。"));
        return;
    }
    if (!m_codec.root()) {
        QMessageBox::warning(this, tr("提示"), tr("请先构造 Huffman 树。"));
        return;
    }

    bool ok = false;
    QString decoded = m_codec.decode(bits, &ok);
    if (!ok) {
        QMessageBox::critical(this, tr("错误"), tr("译码失败：接收的 0/1 序列不合法。"));
        return;
    }
    
    QString time = QTime::currentTime().toString("HH:mm:ss");
    m_textDecoded->append(QString("[%1] 本地译码: %2").arg(time).arg(decoded));
    
    // 保存纯文本用于校验
    m_lastDecodedContent = decoded;
    
    showStatus(tr("已完成本地译码"));
}

// 比较校验：比较原文和译码后的文本是否一致
void MainWindow::onCompare()
{
    QString src = m_textSource->toPlainText();
    // 使用最后一次成功译码的纯文本进行比较
    QString dec = m_lastDecodedContent;

    if (dec.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("尚未进行译码，无法比较。"));
        return;
    }

    if (src == dec) {
        QMessageBox::information(this, tr("比较结果"),
                                 tr("校验成功！\n原文与最后一次译码结果完全一致。"));
    } else {
        int len1 = src.length();
        int len2 = dec.length();
        int minLen = qMin(len1, len2);
        int diffIndex = -1;
        for (int i = 0; i < minLen; ++i) {
            if (src[i] != dec[i]) {
                diffIndex = i;
                break;
            }
        }
        if (diffIndex == -1)
            diffIndex = minLen;

        QMessageBox::warning(this, tr("比较结果"),
                             tr("校验失败。\n"
                                "原文长度: %1\n"
                                "译码长度: %2\n"
                                "首个差异位置: %3")
                                 .arg(len1).arg(len2).arg(diffIndex));
    }
}

//Socket 部分

// 启动服务器监听
void MainWindow::onStartServer()
{
    if (m_server->isListening()) {
        QMessageBox::information(this, tr("提示"), tr("服务器已经在运行。"));
        return;
    }
    if (m_server->start(5555)) {
        showStatus(tr("服务器已启动，监听端口 5555"));
        QMessageBox::information(this, tr("服务器"),
                                 tr("服务器已启动，监听端口 5555。请在客户端连接 127.0.0.1:5555。"));
    }
}

// 关闭服务器
void MainWindow::onStopServer()
{
    m_server->stop();
    showStatus(tr("服务器已关闭"));
    QMessageBox::information(this, tr("服务器"), tr("服务器已关闭。"));
}

// 连接服务器
void MainWindow::onConnectToServer()
{
    if (m_client->isConnected()) {
        QMessageBox::information(this, tr("提示"), tr("已经连接到服务器。"));
        return;
    }

    bool ok;
    QString ip = QInputDialog::getText(this, tr("连接服务器"),
                                       tr("请输入服务器 IP 地址:"), QLineEdit::Normal,
                                       "127.0.0.1", &ok);
    if (!ok || ip.isEmpty()) return;

    m_client->connectToServer(ip, 5555);
    showStatus(tr("正在连接服务器 %1...").arg(ip));
}

// 客户端发送：通过 Socket 发送编码后的数据
void MainWindow::onSendViaSocket()
{
    QString bits = m_textEncoded->toPlainText().trimmed();
    if (bits.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先对原文编码，或编码结果为空。"));
        return;
    }

    if (!m_client->isConnected()) {
        QMessageBox::warning(this, tr("提示"), tr("未连接服务器，请先点击“连接服务器”按钮。"));
        return;
    }
    
    m_client->send(bits);

    // 将自己发送的数据也显示在接收区，形成完整的聊天记录
    QString time = QTime::currentTime().toString("HH:mm:ss");
    m_textReceivedServer->append(QString("<font color='blue'>[%1] 我发送: %2</font>").arg(time).arg(bits));

    showStatus(tr("已通过 socket 发送电文"));
}

// 服务器接收到数据时的处理槽函数
void MainWindow::onServerDataReceived(const QString &data)
{
    QString time = QTime::currentTime().toString("HH:mm:ss");
    m_textReceivedServer->append(QString("[%1] 服务器收到: %2").arg(time).arg(data));
    showStatus(tr("服务器已接收到电文"));

    // 自动译码
    if (m_codec.root()) {
        bool ok = false;
        QString decoded = m_codec.decode(data, &ok);
        if (ok) {
            m_textDecoded->append(QString("[%1] %2").arg(time).arg(decoded));
            // 更新最后一次译码内容，方便校验
            m_lastDecodedContent = decoded;
        }
    }
}

// 客户端接收到数据时的处理槽函数
void MainWindow::onClientDataReceived(const QString &data)
{
    // 如果收到的数据和刚才自己发送的一样，就忽略（避免重复显示）
    // 注意：这种简单的去重方式在极短时间内连续发送相同内容时可能会误判
    // 但对于简单的演示程序来说足够了。更严谨的做法是协议头带 ID。
    
    // 这里我们假设服务器已经做了 exclude 处理（即不发回给发送者）
    // 如果你仍然收到自己发的消息，说明服务器广播逻辑可能有漏网，或者你既是服务器又是客户端
    
    // 既然我们在 onSendViaSocket 里已经手动显示了“我发送: ...”
    // 那么这里收到的应该都是“别人发的”
    
    QString time = QTime::currentTime().toString("HH:mm:ss");
    m_textReceivedServer->append(QString("[%1] 收到: %2").arg(time).arg(data));
    showStatus(tr("收到广播消息"));
    
    // 自动译码
    if (m_codec.root()) {
        bool ok = false;
        QString decoded = m_codec.decode(data, &ok);
        if (ok) {
            m_textDecoded->append(QString("[%1] %2").arg(time).arg(decoded));
            // 更新最后一次译码内容，方便校验
            m_lastDecodedContent = decoded;
        }
    }
}

// 服务器发生错误时的处理槽函数
void MainWindow::onServerError(const QString &msg)
{
    QMessageBox::critical(this, tr("服务器错误"), msg);
}

// 客户端发生错误时的处理槽函数
void MainWindow::onClientError(const QString &msg)
{
    showStatus(tr("客户端错误：%1").arg(msg));
}

// 客户端成功连接到服务器时的处理槽函数
void MainWindow::onClientConnectedToServer()
{
    showStatus(tr("客户端已连接服务器"));
    QMessageBox::information(this, tr("连接成功"), tr("已成功连接到服务器！"));
}

// 服务器端有新客户端连接时的处理槽函数
void MainWindow::onServerClientConnected(const QString &addr)
{
    showStatus(tr("有客户端连接到服务器: %1").arg(addr));
}

void MainWindow::onOpenFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("打开文件"), "", tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法打开文件"));
        return;
    }

    QTextStream in(&file);
    m_textSource->setPlainText(in.readAll());
    file.close();
    showStatus(tr("已加载文件: %1").arg(fileName));
}

void MainWindow::onSaveFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("保存编码"), "", tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("错误"), tr("无法保存文件"));
        return;
    }

    QTextStream out(&file);
    out << m_textEncoded->toPlainText();
    file.close();
    showStatus(tr("已保存编码到: %1").arg(fileName));
}

void MainWindow::updateCompressionRatio()
{
    QString source = m_textSource->toPlainText();
    QString encoded = m_textEncoded->toPlainText();
    
    if (source.isEmpty() || encoded.isEmpty()) {
        m_lblCompressionRatio->setText(tr("压缩率: N/A"));
        return;
    }

    // 过滤掉非01字符
    QString cleanEncoded;
    for(QChar c : encoded) {
        if(c == '0' || c == '1') cleanEncoded.append(c);
    }

    // 原文按8位/字符计算
    // 注意：如果是中文，toPlainText()返回的是QString(UTF-16)，但通常压缩率对比的是ASCII或UTF-8字节流
    // 这里题目公式是 "原文总字符数 * 8"，我们按 QString::length() * 8 计算
    double originalBits = source.length() * 8.0;
    double compressedBits = cleanEncoded.length();
    
    if (originalBits == 0) {
        m_lblCompressionRatio->setText(tr("压缩率: N/A"));
        return;
    }

    double ratio = (compressedBits / originalBits) * 100.0;
    m_lblCompressionRatio->setText(tr("压缩率: %1%").arg(ratio, 0, 'f', 2));
}

