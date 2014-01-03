#include <QMultiHash>
#include <QSet>
#include <QString>

#include "insertworker.h"

#include "CommHistory/group.h"
#include "CommHistory/groupmodel.h"
#include "message.h"
#include "time.h"

QString GROUP_LOCAL_UID = "/org/freedesktop/Telepathy/Account/ring/tel/account0";

InsertWorker::InsertWorker(MessageList messages) :
    QThread()
{
    this->messages = messages;
}

void InsertWorker::setGrouped(MessageList messages) {
    QSet<QString> remoteUids;    // These take out duplicate UIDs
    QStringList remoteUidList;  // Actually stored in a group
    QString joinedRemoteUids;  // I can hash strings

    // These used to deal with message data
    QString remoteUid;
    QString freeText;
    QString nextRemoteUid;
    QString nextFreeText;

    QMultiHash<QString, Message*> groups;

    qDebug() << "Getting groups";

    // Check the message body of the next message to see if
    // we have messages of the same group.
    //
    // XXX: There's an issue where a message that belongs to
    // many groups will get the wrong remoteUID.
    // Use the group keys later to counterbalance that.
    for (int i=0; i<messages.size() - 1; i++) {
        remoteUid = messages.at(i)->remoteUID;
        freeText = messages.at(i)->freeText;
        nextRemoteUid = messages.at(i + 1)->remoteUID;
        nextFreeText = messages.at(i + 1)->freeText;

        // Build a stack-like set here
        remoteUids.insert(remoteUid);

        // Assume same-group messages are in consequent order
        if (freeText.compare(nextFreeText) != 0) {
            remoteUidList = remoteUids.toList();
            remoteUidList.sort();
            joinedRemoteUids = remoteUidList.join(",");

            groups.insert(joinedRemoteUids, messages.at(i));
            remoteUids = QSet<QString>();

            // Component remoteUids must get the message too!
            if (remoteUidList.size() > 1) {
                for (int j=0; j<remoteUidList.size(); j++) {
                    groups.insert(remoteUidList.at(j), messages.at(i));
                }
            }
        }
    }

    // After the for loop we have one more to take care of
    nextRemoteUid = messages.last()->remoteUID;
    nextFreeText = messages.last()->freeText;

    remoteUids.insert(nextRemoteUid);

    remoteUidList = remoteUids.toList();
    remoteUidList.sort();
    joinedRemoteUids = remoteUidList.join(",");

    groups.insert(joinedRemoteUids, messages.last());

    // Also populate the last one, to be sure, if it was a multi-uid group component
    if (remoteUidList.size() > 1) {
        for (int j=0; j<remoteUidList.size(); j++) {
            groups.insert(remoteUidList.at(j), messages.last());
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

QHash<QString, CommHistory::Group> InsertWorker::handleGroups(MessageList messages) {
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

void InsertWorker::handleMessages(QHash<QString, CommHistory::Group> dbGroupRemoteUids) {
    emit duplicateSMSChanged(-1);

    emit insertedSMSChanged(-1);
}

void InsertWorker::run() {
    MessageList messages = this->messages;

    // Parse groups
    this->setGrouped(messages);

    QHash<QString, CommHistory::Group> dbGroupRemoteUids = this->handleGroups(messages);

    this->handleMessages(dbGroupRemoteUids);

    for (int i=0; i<messages.size(); i++) {
        delete messages.at(i);
    }
}
