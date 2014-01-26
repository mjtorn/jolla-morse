#ifndef GLOGEVENTOBJECT_H
#define GLOGEVENTOBJECT_H

#include <QList>
#include <QString>
#include <QObject>

class GlogEvent : public QObject
{
    Q_OBJECT
public:
    QString SMS_TYPE;
    QString CALL_TYPE;
    QString CALL_MISSED_TYPE;

    explicit GlogEvent(QObject *parent = 0);
    Q_INVOKABLE bool isSupported(); // SMS and phone calls we can do.
    Q_INVOKABLE bool isCall(); // Both kinds of calls
    Q_INVOKABLE QString startTimeToString();
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

typedef QList<GlogEvent*> GlogEventList;

#endif // GLOGEVENTOBJECT_H
