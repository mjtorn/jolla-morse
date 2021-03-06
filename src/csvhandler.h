#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QByteArray>
#include <QFile>
#include <QDir>
#include <QList>
#include <QMultiHash>
#include <QString>
#include <QThread>

#ifndef GLOGEVENTOBJECT_H
#include "glogevent.h"
#endif

#include "csvworker.h"

#include "CommHistory/group.h"

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
    Q_INVOKABLE void insertFinished();
    Q_INVOKABLE void parseFile();
    Q_INVOKABLE void insertGlogEvents(GlogEventList glogevents);
    Q_INVOKABLE QString getState();
    Q_INVOKABLE int getReadBytes();
    Q_INVOKABLE int getSeenEntries();
    Q_INVOKABLE int getSeenSMS();
    Q_INVOKABLE int getSeenCSVDuplicates();
    Q_INVOKABLE int getSeenGroups();
    Q_INVOKABLE int getNewGroups();
    Q_INVOKABLE int getDuplicateSMS();
    Q_INVOKABLE int getInsertedSMS();
    Q_INVOKABLE int getInsertedCalls();
    Q_INVOKABLE void setState(QString state);
    Q_INVOKABLE void setReadBytes(int readBytes);
    Q_INVOKABLE void setSeenEntries(int seenEntries);
    Q_INVOKABLE void setSeenSMS(int seenSMS);
    Q_INVOKABLE void setSeenCSVDuplicates(int seenCSVDuplicates);
    Q_INVOKABLE void setSeenGroups(int seenGroups);
    Q_INVOKABLE void setNewGroups(int newGroups);
    Q_INVOKABLE void setDuplicateSMS(int seenSMS);
    Q_INVOKABLE void setInsertedSMS(int insertedSMS);
    Q_INVOKABLE void setInsertedCalls(int insertedCalls);
    Q_PROPERTY(QString state READ getState NOTIFY stateChanged);
    Q_PROPERTY(int readBytes READ getReadBytes NOTIFY readBytesChanged);
    Q_PROPERTY(int seenEntries READ getSeenEntries NOTIFY seenEntriesChanged);
    Q_PROPERTY(int seenSMS READ getSeenSMS NOTIFY seenSMSChanged);
    Q_PROPERTY(int seenCSVDuplicates READ getSeenCSVDuplicates NOTIFY seenCSVDuplicatesChanged);
    Q_PROPERTY(int seenGroups READ getSeenGroups NOTIFY seenGroupsChanged);
    Q_PROPERTY(int newGroups READ getNewGroups NOTIFY newGroupsChanged);
    Q_PROPERTY(int duplicateSMS READ getDuplicateSMS NOTIFY duplicateSMSChanged);
    Q_PROPERTY(int insertedSMS READ getInsertedSMS NOTIFY insertedSMSChanged);
    Q_PROPERTY(int insertedCalls READ getInsertedCalls NOTIFY insertedCallsChanged);

private:
    QString filename;
    QString filepath;
    QString state;
    QFile file;
    int working;
    int readBytes;
    int seenEntries;
    int seenSMS;
    int seenCSVDuplicates;
    int seenGroups;
    int newGroups;
    int duplicateSMS;
    int insertedSMS;
    int insertedCalls;

signals:
    void stateChanged();
    void readBytesChanged();
    void seenEntriesChanged();
    void seenSMSChanged();
    void seenCSVDuplicatesChanged();
    void seenGroupsChanged();
    void newGroupsChanged();
    void duplicateSMSChanged();
    void insertedSMSChanged();
    void insertedCallsChanged();

public slots:

};

#endif // CSVHANDLER_H
