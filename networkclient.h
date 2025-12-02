#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

class NetworkClient : public QObject
{
    Q_OBJECT
public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient();

    // 连接到指定 IP 和端口的服务器
    void connectToServer(const QString &ip, int port);
    // 断开与服务器的连接
    void disconnectFromServer();
    // 向服务器发送消息
    void send(const QString &message);
    // 检查是否已连接到服务器
    bool isConnected() const;

signals:
    // 连接成功信号
    void connected();
    // 断开连接信号
    void disconnected();
    // 接收到数据信号
    void dataReceived(QString data);
    // 发生错误信号
    void errorOccurred(QString errorMsg);

private slots:
    // 处理连接成功
    void onConnected();
    // 处理数据读取
    void onReadyRead();
    // 处理 Socket 错误
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
};

#endif // NETWORKCLIENT_H
