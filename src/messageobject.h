#ifndef MESSAGEOBJECT_H
#define MESSAGEOBJECT_H

#include <QObject>

class MessageObject : public QObject
{
    Q_OBJECT
public:
    explicit MessageObject(QObject *parent = 0);
    quint32 id;
    QString eventTypeName;
    bool isOutgoing;
    quint64 storageTime;
    quint64 startTime;
    quint64 endTime;
    bool isRead;
    // XXX: Guesswork
    quint8 flags;
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

#endif // MESSAGEOBJECT_H
