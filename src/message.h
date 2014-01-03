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
    QString eventTypeName;  // Doesn't look interesting to Jolla
    bool isOutgoing;
    quint64 storageTime;    // Apparently ignored on Jolla
    quint64 startTime;
    quint64 endTime;
    bool isRead;
    // XXX: Guesswork
    quint32 flags;
    quint32 bytesSent;      // Does not exist on Jolla?
    quint32 bytesReceived;
    QString localUID;       // This is the GROUP_LOCAL_UID hardcode
    QString localName;      // contacts()? investigate later if neeeded
    QString remoteUID;
    QString remoteName;     // Appears empty
    QString channel;
    QString freeText;
    QString groupUID;       // Not the same on Jolla

signals:

public slots:

};

typedef QList<Message*> MessageList;

#endif // MESSAGEOBJECT_H
