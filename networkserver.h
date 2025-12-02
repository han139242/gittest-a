#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class NetworkServer : public QObject
{
    Q_OBJECT
public:
    explicit NetworkServer(QObject *parent = nullptr);
    ~NetworkServer();

    // 启动服务器监听指定端口
    bool start(int port);
    // 停止服务器监听并断开连接
    void stop();
    // 检查服务器是否正在监听
    bool isListening() const;
    // 向所有连接的客户端广播消息
    void broadcastMessage(const QString &message, QTcpSocket* exclude = nullptr);

signals:
    // 服务器启动信号
    void serverStarted(int port);
    // 服务器停止信号
    void serverStopped();
    // 客户端连接信号
    void clientConnected(QString clientAddress);
    // 客户端断开连接信号
    void clientDisconnected(QString clientAddress);
    // 接收到数据信号
    void dataReceived(QString data);
    // 发生错误信号
    void errorOccurred(QString errorMsg);

private slots:
    // 处理新连接
    void onNewConnection();
    // 处理数据读取
    void onReadyRead();
    // 处理客户端断开
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients; // 维护所有连接的客户端
};

#endif // NETWORKSERVER_H
