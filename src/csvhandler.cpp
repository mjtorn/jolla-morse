#include "CommHistory/group.h"
#include "csvhandler.h"
#include "csvworker.h"

#include <QDebug>
#include <QByteArray>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QMetaType>
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
    emit readBytesChanged(this->readBytes);
}

int CSVHandler::getReadBytes() {
    return this->readBytes;
}

void CSVHandler::setSeenEntries(int seenEntries) {
    this->seenEntries = seenEntries;
    emit seenEntriesChanged(this->seenEntries);
}

int CSVHandler::getSeenEntries() {
    return this->seenEntries;
}

void CSVHandler::setSeenSMS(int seenSMS) {
    this->seenSMS = seenSMS;
    emit seenSMSChanged(this->seenSMS);
}

int CSVHandler::getSeenSMS() {
    return this->seenSMS;
}

void CSVHandler::setSeenCSVDuplicates(int seenCSVDuplicates) {
    this->seenCSVDuplicates = seenCSVDuplicates;
    emit seenCSVDuplicatesChanged(this->seenCSVDuplicates);
}

int CSVHandler::getSeenCSVDuplicates() {
    return this->seenCSVDuplicates;
}

int CSVHandler::getSeenGroups() {
    return this->seenGroups;
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
        emit insertedSMSChanged(0);
        this->seenCSVDuplicates = 0;
        emit seenCSVDuplicatesChanged(0);
        this->seenGroups = 0;
        emit seenGroupsChanged(0);

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
    QList<CommHistory::Group> groups;
    groups = this->getGroups(messages);
    qDebug() << "Got groups:" << groups.size();

    // TODO: Insert groups with group manager

    this->insertedSMS = messages.size();
    emit insertedSMSChanged(messages.size());
    for (int i=0; i<messages.size(); i++) {
        delete messages.at(i);
    }
}

CommHistory::Group CSVHandler::createGroup(QStringList remoteUids) {
    //qDebug() << "Create group" << remoteUids.join(",");

    // What? No instances required?
    CommHistory::Group group;

    // This appears to be the same, at least at the time of the commit
    group.setLocalUid(GROUP_LOCAL_UID);

    // Defaults to 0 as do all on my phone, and Name is NULL
    group.setChatType(CommHistory::Group::ChatTypeP2P);
    group.setChatName(NULL);

    group.setRemoteUids(remoteUids);

    return group;
}

QList<CommHistory::Group> CSVHandler::getGroups(MessageList messages) {
    QSet<QString> seenRemoteUids; // Keep away duplicate groups
    QSet<QString> remoteUids;    // These away duplicates UIDs
    QStringList remoteUidList;  // Actually stored in a group
    QString joinedRemoteUids;  // I can hash strings

    // These used to deal with message data
    QString remoteUid;
    QString freeText;
    QString nextRemoteUid;
    QString nextFreeText;

    QList<CommHistory::Group> groups;

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

            if (seenRemoteUids.contains(joinedRemoteUids)) {
                //qDebug() << "Already seen" << joinedRemoteUids;
                remoteUids = QSet<QString>();
                continue;
            }

            seenRemoteUids.insert(joinedRemoteUids);

            CommHistory::Group group = this->createGroup(remoteUidList);

            groups.push_back(group);

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

    if (seenRemoteUids.contains(joinedRemoteUids)) {
        //qDebug() << "Already seen" << joinedRemoteUids;
    } else {
        // No need to bother anymore
        // seenRemoteUids.insert(joinedRemoteUids);
        CommHistory::Group group = this->createGroup(remoteUidList);

        groups.push_back(group);
    }

    this->seenGroups = groups.size();
    emit seenGroupsChanged(groups.size());
    return groups;
}
