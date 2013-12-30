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

QMultiHash<QString, Message*> InsertWorker::getGrouped(MessageList messages) {
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

    emit seenGroupsChanged(groups.uniqueKeys().size());
    return groups;
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

void InsertWorker::run() {
    MessageList messages = this->messages;

    // Parse groups
    QMultiHash<QString, Message*> groups;
    groups = this->getGrouped(messages);
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
        return;
    }

    CommHistory::Group dbGroup;
    QSet<QString> dbGroupRemoteUids; // This is all of them
    QStringList groupUids; // This is loop-local
    for (int i=0; i<groupModel.rowCount(); i++) {
        // Assume uniqueness
        dbGroup = groupModel.group(groupModel.index(i, 0));
        groupUids = dbGroup.remoteUids();
        groupUids.sort();
        dbGroupRemoteUids.insert(groupUids.join(","));
    }
    //qDebug() << "found in db" << dbGroupRemoteUids;

    // Then do a similar set of our own stuff
    for (int i=0; i<keys.size(); i++) {
        keySet.insert(keys.at(i));
    }
    //qDebug() << "found in csv" << keySet;

    QSet<QString> diff;
    diff = keySet.subtract(dbGroupRemoteUids);
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
        dbGroupRemoteUids.insert(groupUids.join(","));
    }
    emit newGroupsChanged(newGroups.size());

    // TODO: Actual insertion
    emit insertedSMSChanged(messages.size());
    for (int i=0; i<messages.size(); i++) {
        delete messages.at(i);
    }
}
