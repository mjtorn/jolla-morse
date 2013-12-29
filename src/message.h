#ifndef MESSAGEOBJECT_H
#define MESSAGEOBJECT_H

#include <QList>
#include <QObject>

class Message : public QObject
{
    Q_OBJECT
public:
    explicit Message(QObject *parent = 0);
    quint32 id;
    QString eventTypeName;
    bool isOutgoing;
    quint64 storageTime;
    quint64 startTime;
    quint64 endTime;
    bool isRead;
    // XXX: Guesswork
    quint32 flags;
    quint32 bytesSent;
    quint32 bytesReceived;
    QString localUID;
    QString localName;
    QString remoteUID;
    QString remoteName;
    QString channel;
    QString freeText;
    QString groupUID;

signals:

public slots:

};

typedef QList<Message*> MessageList;

#endif // MESSAGEOBJECT_H
