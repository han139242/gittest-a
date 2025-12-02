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

    bool start(int port);
    void stop();
    bool isListening() const;
    void sendToClient(const QString &message);

signals:
    void serverStarted(int port);
    void serverStopped();
    void clientConnected(QString clientAddress);
    void clientDisconnected();
    void dataReceived(QString data);
    void errorOccurred(QString errorMsg);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QTcpSocket *m_clientSocket; // 简化处理，暂只维护一个活跃连接
};

#endif // NETWORKSERVER_H
