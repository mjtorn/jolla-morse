#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QString>
#include <QThread>

#ifndef MESSAGEOBJECT_H
#include "message.h"
#endif

#include "csvworker.h"

class CSVHandler : public QObject
{
    Q_OBJECT
public:
    explicit CSVHandler(QObject *parent = 0);
    Q_INVOKABLE QStringList getCSVFiles();
    Q_INVOKABLE void setFile(QString filename);
    Q_INVOKABLE QString getFileName();
    Q_INVOKABLE QString getFilePath();
    Q_INVOKABLE void workerFinished();
    Q_INVOKABLE void parseFile();
    Q_INVOKABLE void insertMessages(MessageList messages);
    Q_INVOKABLE int getReadBytes();
    Q_INVOKABLE int getSeenEntries();
    Q_INVOKABLE int getSeenSMS();
    Q_INVOKABLE int getSeenCSVDuplicates();
    Q_INVOKABLE int getInsertedSMS();
    Q_INVOKABLE void setReadBytes(int readBytes);
    Q_INVOKABLE void setSeenEntries(int seenEntries);
    Q_INVOKABLE void setSeenSMS(int seenSMS);
    Q_INVOKABLE void setSeenCSVDuplicates(int seenCSVDuplicates);
    Q_PROPERTY(int readBytes READ getReadBytes NOTIFY readBytesChanged);
    Q_PROPERTY(int seenEntries READ getSeenEntries NOTIFY seenEntriesChanged);
    Q_PROPERTY(int seenSMS READ getSeenSMS NOTIFY seenSMSChanged);
    Q_PROPERTY(int seenCSVDuplicates READ getSeenCSVDuplicates NOTIFY seenCSVDuplicatesChanged);
    Q_PROPERTY(int insertedSMS READ getInsertedSMS NOTIFY insertedSMSChanged);

private:
    QString filename;
    QString filepath;
    QFile file;
    bool workerRunning;
    int readBytes;
    int seenEntries;
    int seenSMS;
    int seenCSVDuplicates;
    int insertedSMS;

signals:
    void readBytesChanged(int newValue);
    void seenEntriesChanged(int seenEntries);
    void seenSMSChanged(int seenSMS);
    void seenCSVDuplicatesChanged(int seenCSVDuplicates);
    void insertedSMSChanged(int insertedSMS);

public slots:

};

#endif // CSVHANDLER_H
