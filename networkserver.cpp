#include "networkserver.h"

NetworkServer::NetworkServer(QObject *parent)
    : QObject(parent), m_server(new QTcpServer(this)), m_clientSocket(nullptr)
{
    connect(m_server, &QTcpServer::newConnection, this, &NetworkServer::onNewConnection);
}

NetworkServer::~NetworkServer()
{
    stop();
}

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

void NetworkServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
        emit serverStopped();
    }
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
}

bool NetworkServer::isListening() const
{
    return m_server->isListening();
}

void NetworkServer::sendToClient(const QString &message)
{
    if (m_clientSocket && m_clientSocket->state() == QAbstractSocket::ConnectedState) {
        m_clientSocket->write(message.toUtf8());
        m_clientSocket->flush();
    } else {
        emit errorOccurred("No client connected or socket not writable.");
    }
}

void NetworkServer::onNewConnection()
{
    if (m_clientSocket) {
        m_clientSocket->disconnectFromHost();
        m_clientSocket->deleteLater();
    }

    m_clientSocket = m_server->nextPendingConnection();
    connect(m_clientSocket, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
    connect(m_clientSocket, &QTcpSocket::disconnected, this, &NetworkServer::onClientDisconnected);
    
    emit clientConnected(m_clientSocket->peerAddress().toString());
}

void NetworkServer::onReadyRead()
{
    if (!m_clientSocket) return;
    QByteArray data = m_clientSocket->readAll();
    emit dataReceived(QString::fromUtf8(data));
}

void NetworkServer::onClientDisconnected()
{
    emit clientDisconnected();
    if (m_clientSocket) {
        m_clientSocket->deleteLater();
        m_clientSocket = nullptr;
    }
}
