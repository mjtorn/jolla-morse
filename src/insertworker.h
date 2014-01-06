#ifndef INSERTWORKER_H
#define INSERTWORKER_H

#include <QDateTime>
#include <QMultiHash>
#include <QSet>
#include <QString>
#include <QThread>
#include <QObject>

#include "CommHistory/group.h"
#include "glogevent.h"

class InsertWorker : public QThread
{
    Q_OBJECT
private:
    Q_INVOKABLE void run();
    Q_INVOKABLE void setGrouped(GlogEventList glogevents);
    Q_INVOKABLE CommHistory::Group createGroup(QStringList remoteUids);
    Q_INVOKABLE QHash<QString, CommHistory::Group> handleGroups(GlogEventList glogevents);
    Q_INVOKABLE void handleGlogEvents(QHash<QString, CommHistory::Group> dbGroupRemoteUids);
    Q_INVOKABLE QString getCheck(GlogEvent *glogEvent, QDateTime *startTime, QDateTime *endTime, QString *key);
    Q_INVOKABLE int createEvent(GlogEvent *glogEvent, CommHistory::Group group, QDateTime startTime, QDateTime endTime, QString key);

    GlogEventList glogevents;
    QMultiHash<QString, GlogEvent*> groups;

public:
    explicit InsertWorker(GlogEventList glogevents);

signals:
    void seenGroupsChanged(int seenGroups);
    void newGroupsChanged(int newGroups);
    void duplicateSMSChanged(int duplicateSMS);
    void insertedSMSChanged(int insertedSMS);
    void insertedCallsChanged(int insertedCalls);
};

#endif // INSERTWORKER_H
