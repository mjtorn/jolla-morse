#ifndef INSERTWORKER_H
#define INSERTWORKER_H

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
    GlogEventList glogevents;
    QMultiHash<QString, GlogEvent*> groups;

public:
    explicit InsertWorker(GlogEventList glogevents);

signals:
    void seenGroupsChanged(int seenGroups);
    void newGroupsChanged(int newGroups);
    void duplicateSMSChanged(int duplicateSMS);
    void insertedSMSChanged(int insertedSMS);
};

#endif // INSERTWORKER_H
