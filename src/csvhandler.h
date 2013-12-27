#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QList>
#include <QString>

#ifndef MESSAGEOBJECT_H
#include "messageobject.h"
#endif

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
    Q_INVOKABLE int getCSVBytes();
    Q_INVOKABLE int getSeenEntries();
    Q_INVOKABLE int getSeenSMS();
    Q_PROPERTY(int readBytes READ getCSVBytes NOTIFY readBytesChanged);
    Q_PROPERTY(int seenEntries READ getSeenEntries NOTIFY seenEntriesChanged);
    Q_PROPERTY(int seenSMS READ getSeenSMS NOTIFY seenSMSChanged);

private:
    int seenEntries;
    int seenSMS;
    Q_INVOKABLE QList<MessageObject*> actualParse();
    QString filename;
    QString filepath;
    QFile file;
    QByteArray csvData;

signals:
    void readBytesChanged(int newValue);
    void seenEntriesChanged(int seenEntries);
    void seenSMSChanged(int seenSMS);

public slots:

};

quint32 toInt(QString s);
quint64 toLL(QString s);

#endif // CSVHANDLER_H
