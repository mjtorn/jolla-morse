#include "CommHistory/group.h"
#include "CommHistory/groupmodel.h"
#include "csvhandler.h"
#include "csvworker.h"

#include <QDebug>
#include <QByteArray>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QMetaType>
#include <QMultiHash>
#include <QString>
#include <QThread>

#include <time.h>

QString BASEDIR_NAME = QString("/etc/mersdk/share/");
QString GROUP_LOCAL_UID = "/org/freedesktop/Telepathy/Account/ring/tel/account0";

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
    // Come from CSVWorker
    this->workerRunning = false;
    this->readBytes = 0;
    this->seenEntries = 0;
    this->seenSMS = 0;

    // This is us
    this->seenGroups = 0;
    this->insertedSMS = 0;
}

QStringList CSVHandler::getCSVFiles() {
    QStringList filter;
    filter << "*.csv";

    QDir qdir = QDir(BASEDIR_NAME);
    qdir.setNameFilters(filter);

    QStringList list = qdir.entryList();
    return list;
}

void CSVHandler::setFile(QString filename) {
    this->filename = filename;
    this->filepath = BASEDIR_NAME + filename;
}

QString CSVHandler::getFilePath() {
    return this->filepath;
}

QString CSVHandler::getFileName() {
    return this->filename;
}

void CSVHandler::setReadBytes(int readBytes) {
    this->readBytes = readBytes;
    emit readBytesChanged();
}

int CSVHandler::getReadBytes() {
    return this->readBytes;
}

void CSVHandler::setSeenEntries(int seenEntries) {
    this->seenEntries = seenEntries;
    emit seenEntriesChanged();
}

int CSVHandler::getSeenEntries() {
    return this->seenEntries;
}

void CSVHandler::setSeenSMS(int seenSMS) {
    this->seenSMS = seenSMS;
    emit seenSMSChanged();
}

int CSVHandler::getSeenSMS() {
    return this->seenSMS;
}

void CSVHandler::setSeenCSVDuplicates(int seenCSVDuplicates) {
    this->seenCSVDuplicates = seenCSVDuplicates;
    emit seenCSVDuplicatesChanged();
}

int CSVHandler::getSeenCSVDuplicates() {
    return this->seenCSVDuplicates;
}

int CSVHandler::getSeenGroups() {
    return this->seenGroups;
}

int CSVHandler::getNewGroups() {
    return this->newGroups;
}

int CSVHandler::getInsertedSMS() {
    return this->insertedSMS;
}

void CSVHandler::workerFinished() {
    this->workerRunning = false;
}

void CSVHandler::parseFile() {
    qRegisterMetaType<MessageList>("MessageList");
    qDebug() << "CSVHandler::parseFile() called, registered QList<Message*>";
    if (!this->workerRunning) {
        // This is called when the qml is activated, reset some state
        this->insertedSMS = 0;
        emit insertedSMSChanged();
        this->seenCSVDuplicates = 0;
        emit seenCSVDuplicatesChanged();
        this->seenGroups = 0;
        this->newGroups = 0;
        emit seenGroupsChanged();

        this->workerRunning = true;
        CSVWorker *csvWorker = new CSVWorker(this->getFilePath());

        // Required methods
        connect(csvWorker, &CSVWorker::parseFileCompleted, this, &CSVHandler::insertMessages);
        connect(csvWorker, &CSVWorker::finished, csvWorker, &QObject::deleteLater);
        connect(csvWorker, &CSVWorker::finished, this, &CSVHandler::workerFinished);
        // Transfer the state to QML
        connect(csvWorker, &CSVWorker::readBytesChanged, this, &CSVHandler::setReadBytes);
        connect(csvWorker, &CSVWorker::seenEntriesChanged, this, &CSVHandler::setSeenEntries);
        connect(csvWorker, &CSVWorker::seenSMSChanged, this, &CSVHandler::setSeenSMS);
        connect(csvWorker, &CSVWorker::seenCSVDuplicatesChanged, this, &CSVHandler::setSeenCSVDuplicates);
        csvWorker->start();
    } else {
        qDebug() << "Refuse to run another thread!";
    }
}

void CSVHandler::insertMessages(MessageList messages) {
    qDebug() << "Inserting messages:" << messages.size();

    // TODO: Actual functionality!

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
    }

    this->insertedSMS = messages.size();
    emit insertedSMSChanged();
    for (int i=0; i<messages.size(); i++) {
        delete messages.at(i);
    }
}

CommHistory::Group CSVHandler::createGroup(QStringList remoteUids) {
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

QMultiHash<QString, Message*> CSVHandler::getGrouped(MessageList messages) {
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

    this->seenGroups = groups.uniqueKeys().size();
    emit seenGroupsChanged();
    return groups;
}
