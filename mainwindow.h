#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QHash>

#include "huffman.h"
#include "huffmantreewidget.h"
#include "networkserver.h"
#include "networkclient.h"

class QTextEdit;
class QTableWidget;
class QListWidget;
class QPushButton;
class QLabel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // 统计字符频率并构造 Huffman 树
    void onBuildTree();
    // 对原文进行编码
    void onEncode();
    // 本地发送：将编码结果复制到本地接收区
    void onSendLocal();
    // 本地译码：对本地接收区的 0/1 序列进行译码
    void onDecodeLocal();
    // 比较校验：比较原文和译码后的文本是否一致
    void onCompare();

    // 文件操作
    void onOpenFile();
    void onSaveFile();

    // Socket 相关
    // 启动服务器监听
    void onStartServer();
    // 关闭服务器
    void onStopServer();
    // 连接服务器
    void onConnectToServer();
    // 客户端发送：通过 Socket 发送编码后的数据
    void onSendViaSocket();
    
    // 新增槽函数用于处理 NetworkServer/Client 信号
    // 服务器接收到数据时的处理槽函数
    void onServerDataReceived(const QString &data);
    // 客户端接收到数据时的处理槽函数
    void onClientDataReceived(const QString &data);
    // 服务器发生错误时的处理槽函数
    void onServerError(const QString &msg);
    // 客户端发生错误时的处理槽函数
    void onClientError(const QString &msg);
    // 客户端成功连接到服务器时的处理槽函数
    void onClientConnectedToServer();
    // 服务器端有新客户端连接时的处理槽函数
    void onServerClientConnected(const QString &addr);

private:
    // UI 控件指针
    QTextEdit* m_textSource;
    QTextEdit* m_textEncoded;
    QTextEdit* m_textReceivedLocal;
    QTextEdit* m_textReceivedServer;
    QTextEdit* m_textDecoded;
    QTableWidget* m_tableCodes;
    HuffmanTreeWidget* m_treeWidget;
    QListWidget* m_listSteps;
    QLabel* m_statusLabel;
    QLabel* m_lblCompressionRatio; // 显示压缩率

    // Huffman 编码器
    HuffmanCodec m_codec;
    QMap<QChar, int> m_freqMap;
    QHash<QChar, QString> m_codes;

    // Socket
    NetworkServer* m_server;
    NetworkClient* m_client;

    void setupUi();             // 创建和布局 UI
    void setupStyle();          // 新增：设置样式表
    void setupToolbar();        // 创建工具栏和按钮
    void setupDock();           // 创建步骤日志停靠窗口

    void updateCodeTable();     // 刷新频率/编码表
    void updateStepsList();     // 刷新构造步骤列表
    void updateTreeView();      // 刷新 Huffman 树视图
    void updateCompressionRatio(); // 计算并显示压缩率

    void showStatus(const QString& text);
};

#endif // MAINWINDOW_H
