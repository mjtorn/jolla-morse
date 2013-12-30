#include "csvhandler.h"
#include "csvworker.h"
#include "insertworker.h"

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

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
    // Come from CSVWorker
    this->workerRunning = false;
    this->readBytes = 0;
    this->seenEntries = 0;
    this->seenSMS = 0;

    // And this from inserting
    this->seenGroups = 0;
    this->insertedSMS = 0;
    this->insertRunning = false;
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

void CSVHandler::setSeenGroups(int seenGroups) {
    this->seenGroups = seenGroups;
    emit seenGroupsChanged();
}

int CSVHandler::getSeenGroups() {
    return this->seenGroups;
}

void CSVHandler::setNewGroups(int newGroups) {
    this->newGroups = newGroups;
    emit newGroupsChanged();
}

int CSVHandler::getNewGroups() {
    return this->newGroups;
}

void CSVHandler::setInsertedSMS(int insertedSMS) {
    this->insertedSMS = insertedSMS;
    emit insertedSMSChanged();
}

int CSVHandler::getInsertedSMS() {
    return this->insertedSMS;
}

void CSVHandler::workerFinished() {
    this->workerRunning = false;
}

void CSVHandler::insertFinished() {
    this->insertRunning = false;
}

void CSVHandler::parseFile() {
    qRegisterMetaType<MessageList>("MessageList");
    qDebug() << "CSVHandler::parseFile() called, registered QList<Message*>";
    if (!this->workerRunning) {
        // This is called when the qml is activated, reset some state
        this->setSeenSMS(0);
        this->setSeenCSVDuplicates(0);
        this->setSeenGroups(0);
        this->setNewGroups(0);
        this->setInsertedSMS(0);

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

    if (!this->insertRunning) {
        this->insertRunning = true;
        InsertWorker *insertWorker = new InsertWorker(messages);

        // Required methods
        connect(insertWorker, &InsertWorker::finished, insertWorker, &QObject::deleteLater);
        connect(insertWorker, &InsertWorker::finished, this, &CSVHandler::insertFinished);
        // Relay state to qml
        connect(insertWorker, &InsertWorker::seenGroupsChanged, this, &CSVHandler::setSeenGroups);
        connect(insertWorker, &InsertWorker::newGroupsChanged, this, &CSVHandler::setNewGroups);
        connect(insertWorker, &InsertWorker::insertedSMSChanged, this, &CSVHandler::setInsertedSMS);
        insertWorker->start();
    } else {
        qDebug() << "Refuse to run another thread!";
    }
}
