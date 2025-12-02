#include "networkclient.h"

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

void NetworkClient::connectToServer(const QString &ip, int port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) return;
    m_socket->connectToHost(ip, port);
}

void NetworkClient::disconnectFromServer()
{
    m_socket->disconnectFromHost();
}

void NetworkClient::send(const QString &message)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->write(message.toUtf8());
        m_socket->flush();
    } else {
        emit errorOccurred("Not connected to server.");
    }
}

bool NetworkClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void NetworkClient::onConnected()
{
    emit connected();
}

void NetworkClient::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataReceived(QString::fromUtf8(data));
}

void NetworkClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit errorOccurred(m_socket->errorString());
}
