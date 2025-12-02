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
    void onBuildTree();
    void onEncode();
    void onSendLocal();
    void onDecodeLocal();
    void onCompare();

    // Socket 相关
    void onStartServer();
    void onStopServer();
    void onSendViaSocket();
    
    // 新增槽函数用于处理 NetworkServer/Client 信号
    void onServerDataReceived(const QString &data);
    void onClientDataReceived(const QString &data);
    void onServerError(const QString &msg);
    void onClientError(const QString &msg);
    void onClientConnectedToServer();
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

    // Huffman 编码器
    HuffmanCodec m_codec;
    QMap<QChar, int> m_freqMap;
    QHash<QChar, QString> m_codes;

    // Socket
    NetworkServer* m_server;
    NetworkClient* m_client;

    void setupUi();             // 创建和布局 UI
    void setupToolbar();        // 创建工具栏和按钮
    void setupDock();           // 创建步骤日志停靠窗口

    void updateCodeTable();     // 刷新频率/编码表
    void updateStepsList();     // 刷新构造步骤列表
    void updateTreeView();      // 刷新 Huffman 树视图

    void showStatus(const QString& text);
};

#endif // MAINWINDOW_H
