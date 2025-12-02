#include "networkserver.h"

// 构造函数：初始化 TCP 服务器
NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_clientSocket(nullptr)
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
    if (m_clientSocket) {
        // 断开所有信号连接，防止触发 onClientDisconnected 导致重复删除或逻辑错误
        m_clientSocket->disconnect(this);
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
}

// 检查服务器是否正在监听
bool NetworkServer::isListening() const
{
    return m_server->isListening();
}

// 向已连接的客户端发送消息
void NetworkServer::sendToClient(const QString &message)
{
    if (m_clientSocket && m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        m_clientSocket->write(message.toUtf8());
        m_clientSocket->flush();
    } else {
        emit errorOccurred("No client connected or socket not writable.");
    }
}

// 处理新客户端连接请求
void NetworkServer::onNewConnection()
{
    if (m_clientSocket) {
        // 断开旧 Socket 的信号，防止其 disconnected 信号误删新 Socket
        m_clientSocket->disconnect(this);
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }

    m_clientSocket = m_server->nextPendingConnection();
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &NetworkServer::onClientDisconnected);
    
    emit clientConnected(m_clientSocket->peerAddress().toString());
}

// 读取客户端发送的数据
void NetworkServer::onReadyRead()
{
    if (!m_clientSocket) return;
    QByteArray data = m_clientSocket->readAll();
    emit dataReceived(QString::fromUtf8(data));
}

// 处理客户端断开连接
void NetworkServer::onClientDisconnected()
{
    // 确保是当前活动的 Socket 发出的信号
    QTcpSocket *senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (senderSocket && senderSocket != m_clientSocket) {
        return;
    }

    emit clientDisconnected();
    if (m_clientSocket) {
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
}
