#ifndef CSVWORKER_H
#define CSVWORKER_H

#include <QString>
#include <QThread>

#ifndef MESSAGEOBJECT_H
#include "message.h"
#endif

class CSVWorker : public QThread
{
    Q_OBJECT
private:
    int seenEntries;
    int seenSMS;
    QByteArray csvData;
    QString filepath;
    Q_INVOKABLE void run();
    Q_INVOKABLE MessageList actualParse();
public:
    explicit CSVWorker(QString filepath);
    Q_INVOKABLE int getCSVBytes();
    Q_INVOKABLE int getSeenEntries();
    Q_INVOKABLE int getSeenSMS();
    Q_PROPERTY(int readBytes READ getCSVBytes NOTIFY readBytesChanged);
    Q_PROPERTY(int seenEntries READ getSeenEntries NOTIFY seenEntriesChanged);
    Q_PROPERTY(int seenSMS READ getSeenSMS NOTIFY seenSMSChanged);
signals:
    void readBytesChanged(int newValue);
    void seenEntriesChanged(int seenEntries);
    void seenSMSChanged(int seenSMS);
    void parseFileCompleted(MessageList messages);
};

quint32 toInt(QString s);
quint64 toLL(QString s);

#endif // CSVWORKER_H
