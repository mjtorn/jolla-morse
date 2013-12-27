#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QList>
#include <QString>
#include <QThread>

#ifndef MESSAGEOBJECT_H
#include "messageobject.h"
#endif

typedef QList<MessageObject*> MessageObjectList;

class CSVWorker : public QThread
{
    Q_OBJECT
private:
    int seenEntries;
    int seenSMS;
    QByteArray csvData;
    QString filepath;
    Q_INVOKABLE void run();
    Q_INVOKABLE MessageObjectList actualParse();
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
    void parseFileCompleted(MessageObjectList messages);
};


class CSVHandler : public QObject
{
    Q_OBJECT
public:
    explicit CSVHandler(QObject *parent = 0);
    Q_INVOKABLE QStringList getCSVFiles();
    Q_INVOKABLE void setFile(QString filename);
    Q_INVOKABLE QString getFileName();
    Q_INVOKABLE QString getFilePath();
    Q_INVOKABLE void parseFile();
    Q_INVOKABLE void insertMessages(MessageObjectList messages);

private:
    QString filename;
    QString filepath;
    QFile file;

signals:

public slots:

};

quint32 toInt(QString s);
quint64 toLL(QString s);

#endif // CSVHANDLER_H
