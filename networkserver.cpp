#include "networkserver.h"

// 构造函数：初始化 TCP 服务器
NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

NetworkServer::~NetworkServer()
{
    stop();
}

// 启动服务器监听指定端口
bool NetworkServer::start(int port)
{
    if (m_server->isListening()) return true;

    if (!m_server->listen(QHostAddress::Any, port)) {
        emit errorOccurred(m_server->errorString());
        return false;
    }
    emit serverStarted(port);
    return true;
}

// 停止服务器监听并断开连接
void NetworkServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
        emit serverStopped();
    }
    
    // 断开所有客户端
    for (QTcpSocket* client : m_clients) {
        client->disconnect(this);
        client->disconnectFromHost();
        client->deleteLater();
    }
    m_clients.clear();
}

// 检查服务器是否正在监听
bool NetworkServer::isListening() const
{
    return m_server->isListening();
}

// 向所有连接的客户端广播消息
void NetworkServer::broadcastMessage(const QString &message, QTcpSocket* exclude)
{
    QByteArray data = message.toUtf8();
    for (QTcpSocket* client : m_clients) {
        if (client != exclude && client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
            client->flush();
        }
    }
}

// 处理新客户端连接请求
void NetworkServer::onNewConnection()
{
    while (m_server->hasPendingConnections()) {
        QTcpSocket* clientSocket = m_server->nextPendingConnection();
        connect(clientSocket, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
        connect(clientSocket, &QTcpSocket::disconnected, this, &NetworkServer::onClientDisconnected);
        
        m_clients.append(clientSocket);
        emit clientConnected(clientSocket->peerAddress().toString());
    }
}

// 读取客户端发送的数据
void NetworkServer::onReadyRead()
{
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QByteArray data = senderSocket->readAll();
    QString msg = QString::fromUtf8(data);
    
    emit dataReceived(msg);
    
    // 广播给其他客户端
    broadcastMessage(msg, senderSocket);
}

// 处理客户端断开连接
void NetworkServer::onClientDisconnected()
{
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    QString addr = senderSocket->peerAddress().toString();
    m_clients.removeAll(senderSocket);
    senderSocket->deleteLater();
    
    emit clientDisconnected(addr);
}
