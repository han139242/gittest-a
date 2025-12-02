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

    void connectToServer(const QString &ip, int port);
    void disconnectFromServer();
    void send(const QString &message);
    bool isConnected() const;

signals:
    void connected();
    void disconnected();
    void dataReceived(QString data);
    void errorOccurred(QString errorMsg);

private slots:
    void onConnected();
    void onReadyRead();
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
};

#endif // NETWORKCLIENT_H
