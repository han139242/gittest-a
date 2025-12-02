#include "networkclient.h"

// 构造函数：初始化 TCP Socket
NetworkClient::NetworkClient(QObject *parent)
    : QObject(parent), m_socket(new QTcpSocket(this))
{
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::disconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onReadyRead);
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
            this, &NetworkClient::onError);
}

NetworkClient::~NetworkClient()
{
    disconnectFromServer();
}

// 连接到指定 IP 和端口的服务器
void NetworkClient::connectToServer(const QString &ip, int port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) return;
    m_socket->connectToHost(ip, port);
}

// 断开与服务器的连接
void NetworkClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

// 向服务器发送消息
void NetworkClient::send(const QString &message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(message.toUtf8());
        m_socket->flush();
    } else {
        emit errorOccurred("Not connected to server.");
    }
}

// 检查是否已连接到服务器
bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

// 连接成功时的槽函数
void NetworkClient::onConnected()
{
    emit connected();
}

// 读取服务器发送的数据
void NetworkClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataReceived(QString::fromUtf8(data));
}

// 处理 Socket 错误
void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}
