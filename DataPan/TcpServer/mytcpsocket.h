#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H
#include <QTimer>
#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include <QFile>
class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();

signals:
    void offline(MyTcpSocket *mysocket);

public slots:
    void recvMsg();
    void clientOffline();
    void sendFileToClient();

private:
    QString m_strName;
    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecved;
    bool m_bUpload;
    QTimer *m_pTimer;

};

#endif // MYTCPSOCKET_H
