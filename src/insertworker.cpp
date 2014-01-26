#include <QDateTime>
#include <QMultiHash>
#include <QSet>
#include <QString>

#include "insertworker.h"

#include "CommHistory/conversationmodel.h"
#include "CommHistory/event.h"
#include "CommHistory/group.h"
#include "CommHistory/groupmodel.h"
#include "glogevent.h"
#include "time.h"

QString GROUP_LOCAL_UID = "/org/freedesktop/Telepathy/Account/ring/tel/account0";

InsertWorker::InsertWorker(GlogEventList glogevents) :
    QThread()
{
    this->glogevents = glogevents;
}

/*
 * This is reponsible for splicing multi-uid GlogEvents into single uids.
 *
 * Sets into this a QMultiHash of group ids and events where these are mapped.
 *
 */
void InsertWorker::setGrouped(GlogEventList glogevents) {
    QSet<QString> remoteUids;    // These take out duplicate UIDs
    QStringList remoteUidList;  // Actually stored in a group
    QString joinedRemoteUids;  // I can hash strings

    // These used to deal with glogevent data
    QString remoteUid;
    QString freeText;
    QString nextRemoteUid;
    QString nextFreeText;

    QMultiHash<QString, GlogEvent*> groups;

    if (glogevents.size() == 0) {
        qCritical() << "Got no glogevents!";
        return;
    }

    qDebug() << "Getting groups";

    // Check the glogevent body of the next glogevent to see if
    // we have glogevents of the same group.
    //
    // XXX: There's an issue where a glogevent that belongs to
    // many groups will get the wrong remoteUID.
    // Use the group keys later to counterbalance that.
    GlogEvent *glogEvent;
    for (int i=0; i<glogevents.size() - 1; i++) {
        glogEvent = glogevents.at(i);

        remoteUid = glogevents.at(i)->remoteUID;
        freeText = glogevents.at(i)->freeText;
        nextRemoteUid = glogevents.at(i + 1)->remoteUID;
        nextFreeText = glogevents.at(i + 1)->freeText;

        // Build a stack-like set here
        remoteUids.insert(remoteUid);

        // Assume same-group glogevents are in consequent order
        // but also split into new group if we're not an sms
        if (freeText.compare(nextFreeText) != 0 || glogEvent->isCall()) {
            remoteUidList = remoteUids.toList();
            remoteUidList.sort();
            joinedRemoteUids = remoteUidList.join(",");

            groups.insert(joinedRemoteUids, glogevents.at(i));
            remoteUids = QSet<QString>();

            // Component remoteUids must get the glogevent too!
            if (remoteUidList.size() > 1) {
                for (int j=0; j<remoteUidList.size(); j++) {
                    groups.insert(remoteUidList.at(j), glogevents.at(i));
                }
            }
        }
    }

    // After the for loop we have one more to take care of
    nextRemoteUid = glogevents.last()->remoteUID;
    nextFreeText = glogevents.last()->freeText;

    remoteUids.insert(nextRemoteUid);

    remoteUidList = remoteUids.toList();
    remoteUidList.sort();
    joinedRemoteUids = remoteUidList.join(",");

    groups.insert(joinedRemoteUids, glogevents.last());

    // Also populate the last one, to be sure, if it was a multi-uid group component
    if (remoteUidList.size() > 1) {
        for (int j=0; j<remoteUidList.size(); j++) {
            groups.insert(remoteUidList.at(j), glogevents.last());
        }
    }

    emit seenGroupsChanged(groups.uniqueKeys().size());
    this->groups = groups;
}


CommHistory::Group InsertWorker::createGroup(QStringList remoteUids) {
    //qDebug() << "Creating" << remoteUids;

    CommHistory::GroupModel groupModel;

    CommHistory::Group group;

    // This appears to be the same, at least at the time of the commit
    group.setLocalUid(GROUP_LOCAL_UID);

    // Defaults to 0 as do all on my phone, and Name is NULL
    group.setChatType(CommHistory::Group::ChatTypeP2P);
    group.setChatName(NULL);

    group.setRemoteUids(remoteUids);

    groupModel.addGroup(group);

    return group;
}

/*
 * This method syncs the groups in GlogEventList glogevents with what's in the
 * db and returns the end result.
 *
 * The return value's keys are remoteUid(s) and values the Group objects
 *
 */
QHash<QString, CommHistory::Group> InsertWorker::handleGroups(GlogEventList glogevents) {
    QHash<QString, CommHistory::Group> dbGroupRemoteUids; // This is all of them, our return value

    QStringList groupUidList;
    QStringList keys = groups.uniqueKeys();
    QSet<QString> keySet;

    qDebug() << "Got groups:" << keys.size();

    // Be nice with the db, don't hammer it to death
    CommHistory::GroupModel groupModel;
    QList<CommHistory::Group> dbGroups;
    bool dbSuccess = groupModel.getGroups(GROUP_LOCAL_UID, QString(""));
    if (!dbSuccess) {
        qCritical() << "Failed to get groups";
        emit newGroupsChanged(-1);
        return dbGroupRemoteUids;
    }

    CommHistory::Group dbGroup;
    QStringList groupUids; // This is loop-local
    for (int i=0; i<groupModel.rowCount(); i++) {
        // Assume uniqueness
        dbGroup = groupModel.group(groupModel.index(i, 0));
        groupUids = dbGroup.remoteUids();
        groupUids.sort();
        dbGroupRemoteUids.insert(groupUids.join(","), dbGroup);
        Q_ASSERT(dbGroupRemoteUids.values(groupUids.join(",")).size() == 1);
    }
    //qDebug() << "found in db" << dbGroupRemoteUids;

    // Then do a similar set of our own stuff
    for (int i=0; i<keys.size(); i++) {
        keySet.insert(keys.at(i));
    }
    //qDebug() << "found in csv" << keySet;

    QSet<QString> diff;
    diff = keySet.subtract(dbGroupRemoteUids.keys().toSet());
    //qDebug() << "difference set" << diff;

    // There will be no race condition on your phone :<
    QList<CommHistory::Group> newGroups;
    foreach (QString toCreateUids, diff) {
        CommHistory::Group group = this->createGroup(toCreateUids.split(","));
        newGroups.push_back(group);
        if (newGroups.size() % 10 == 0) {
            usleep(1000);
            emit newGroupsChanged(newGroups.size());
        }
        // Populate our new groups as they are in the db
        Q_ASSERT(group.id() != -1);
        groupUids = group.remoteUids();
        groupUids.sort();
        dbGroupRemoteUids.insert(groupUids.join(","), group);
        Q_ASSERT(dbGroupRemoteUids.values(groupUids.join(",")).size() == 1);
    }
    emit newGroupsChanged(newGroups.size());

    return dbGroupRemoteUids;
}

QString InsertWorker::getCheck(GlogEvent *glogEvent, QDateTime *startTime, QString *key) {
    QString s;

    startTime->setTime_t(glogEvent->startTime);

    s = startTime->toString(Qt::TextDate) + QString("|");

    // GlogEvents sent to many people have empty remoteUid
    if (!key->contains(',')) {
        s += key;
    } /*else {
        qDebug() << "Going to skip key" << key;
    }*/

    s += QString("|") + glogEvent->freeText;

    return s;
}

int InsertWorker::createEvent(GlogEvent *glogEvent, CommHistory::Group group, QDateTime startTime, QString key) {
    CommHistory::EventModel eventModel;

    CommHistory::Event e;

    // The n900 dump has end times only for incoming messages
    // but having 0 on Jolla screws up the ordering.
    QDateTime endTime;
    if (glogEvent->endTime == 0) {
        endTime.setTime_t(glogEvent->startTime);
    } else {
        endTime.setTime_t(glogEvent->endTime);
    }

    // Set type, sms or call
    if (!glogEvent->isCall()) {
        e.setType(CommHistory::Event::SMSEvent);
        (glogEvent->isOutgoing) ? e.setIsRead(true) : e.setIsRead(glogEvent->isRead);
    } else {
        e.setType(CommHistory::Event::CallEvent);
        if (glogEvent->eventTypeName.compare(glogEvent->CALL_MISSED_TYPE)) {
            e.setIsMissedCall(true);
        }
    }
    e.setLocalUid(GROUP_LOCAL_UID);
    e.setStartTime(startTime);
    e.setEndTime(endTime);
    (glogEvent->isOutgoing) ? e.setDirection(CommHistory::Event::Outbound) : e.setDirection(CommHistory::Event::Inbound);
    // Sent glogevents are sent
    // Also only they have lastModified
    if (e.direction() == CommHistory::Event::Outbound) {
        if (!glogEvent->isCall()) {
            e.setLastModified(startTime);
            e.setStatus(CommHistory::Event::SentStatus);
        } else {
            /*
             * n900 stores end_time = 0 for missed calls.
             * The top of this method resets that to equal
             * start_time, which is how Jolla stores missed calls!
             *
             * The n900 does not store call durations, unlike
             * Jolla, so every phone call will now have lasted a second.
             */
            if (glogEvent->eventTypeName.compare(glogEvent->CALL_TYPE) == 0) {
                e.setEndTime(e.endTime().addSecs(1));
            }
            // Outgoing calls are also considered read(!)
            // regardless of whether or not they were answered
            e.setIsRead(true);
        }
    }
    e.setGroupId(group.id());
    // For group messages make sure the event is inserted foreach group,
    // but the actual group version of the glogevent is without remoteUids.
    // Also hope this is true for phone calls.
    if (!key.contains(','))
        e.setRemoteUid(key);
    e.setFreeText(glogEvent->freeText);

    int retval = eventModel.addEvent(e);

    return retval;
}

/*
 * After this->groups has been populated, and the database has all the groups,
 * call this to take care of insertions
 *
 */
void InsertWorker::handleGlogEvents(QHash<QString, CommHistory::Group> dbGroupRemoteUids) {
    groups = this->groups;
    int duplicate = 0;
    int insertedSMS = 0;
    int insertedCalls = 0;

    // These are the groupIds to query for
    QList<int> groupIds;
    // Store mapping from groupId to group here
    // as a kind of cache for queries
    QHash<int, CommHistory::Group> groupMap;

    QList<CommHistory::Group> groupObjects = dbGroupRemoteUids.values();
    for (int i=0; i<groupObjects.size(); i++) {
        groupIds.push_back(groupObjects.at(i).id());
        groupMap.insert(groupObjects.at(i).id(), groupObjects.at(i));
    }

    // Get all the glogevents in the db
    // Store a QSet<QString> of hashlike things we can check against later
    QSet<QString> hashlets;
    QString s;

    // Setting up the query
    CommHistory::ConversationModel conversationModel;
    conversationModel.enableContactChanges(false);
    conversationModel.setFilter(CommHistory::Event::UnknownType, GROUP_LOCAL_UID, CommHistory::Event::UnknownDirection);

    // Query for all the groups
    for (int i=0; i<groupIds.size(); i++) {
        bool retval = conversationModel.getEvents(groupIds.at(i));
        if (!retval) {
            qCritical() << "Failed getting events for groups";
            return;
        }
        //qDebug() << conversationModel.rowCount();
        for (int i=0; i<conversationModel.rowCount(); i++) {
            CommHistory::Event e = conversationModel.event(conversationModel.index(i, 0));
            // It's in UTC by default
            QDateTime dbStartTime = e.startTime();
            //qDebug() << "found in db" << dbStartTime.toLocalTime().toString(Qt::TextDate);
            s = dbStartTime.toLocalTime().toString(Qt::TextDate) + QString("|") + e.remoteUid() + QString("|") + e.freeText();
            hashlets.insert(s);
        }
    }

    // Iterate over what we have and insert them all
    QList<QString> groupKeys = groups.uniqueKeys();
    for (int i=0;i<groupKeys.size(); i++) {
        QString key = groupKeys.at(i);
        GlogEventList glogevents = groups.values(key);
        CommHistory::Group group = dbGroupRemoteUids.value(key);
        //qDebug() << group.toString() << glogevents;

        // Go through our glogevents and see if we need to insert them.
        // Insert them if we do.
        bool prevWasSMS = false;
        for (int j=0; j<glogevents.size(); j++) {
            GlogEvent *glogEvent = glogevents.at(j);

            // Deal with supported types; SMS, CALL, CALL_MISSED
            if (glogEvent->isSupported()) {
                QDateTime startTime;
                s = getCheck(glogEvent, &startTime, &key);

                if (!hashlets.contains(s) && !glogEvent->isCall()) {
                    int retval = createEvent(glogEvent, group, startTime, key);
                    if (!retval) {
                        qCritical() << "Failed adding event for glogevent" << glogEvent->id;
                        emit duplicateSMSChanged(-1);
                        emit insertedSMSChanged(-1);
                        emit insertedCallsChanged(-1);
                        return;
                    }

                    if (!glogEvent->isCall()) {
                        insertedSMS++;
                        //qDebug() << "Added SMS" << glogEvent->startTime << glogEvent->startTimeToString() << glogEvent->remoteUID << glogEvent->freeText;
                        if (insertedSMS % 100 == 0 || !prevWasSMS) {
                            emit insertedSMSChanged(insertedSMS);
                        }
                        prevWasSMS = true;
                    } else {
                        insertedCalls++;
                        if (insertedCalls % 100 == 0 || prevWasSMS) {
                            emit insertedCallsChanged(insertedCalls);
                        }
                        prevWasSMS = false;
                    }
                } else {
                    duplicate++;
                    if (duplicate % 10 == 0) {
                        emit duplicateSMSChanged(duplicate);
                    }
                }
            }
        }
    }

    emit duplicateSMSChanged(duplicate);
    emit insertedSMSChanged(insertedSMS);
    emit insertedCallsChanged(insertedCalls);
}

void InsertWorker::run() {
    GlogEventList glogevents = this->glogevents;

    // Parse groups
    this->setGrouped(glogevents);

    QHash<QString, CommHistory::Group> dbGroupRemoteUids = this->handleGroups(glogevents);

    this->handleGlogEvents(dbGroupRemoteUids);

    for (int i=0; i<glogevents.size(); i++) {
        delete glogevents.at(i);
    }
}
