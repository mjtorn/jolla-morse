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

// FIXME: Should be based on a -D flag
//QString BASEDIR_NAME = QString("/etc/mersdk/share/");
#include <QStandardPaths>
QString BASEDIR_NAME = QStandardPaths::displayName(QStandardPaths::DocumentsLocation) + QString("/");

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
    this->setState(QString("Idle"));

    // A kind of mask for running threads
    this->working = 0;

    // Come from CSVWorker
    this->readBytes = 0;
    this->seenEntries = 0;
    this->seenSMS = 0;

    // And this from inserting
    this->seenGroups = 0;
    this->duplicateSMS = 0;
    this->insertedSMS = 0;
    this->insertedCalls = 0;
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

void CSVHandler::setState(QString state) {
    this->state = state;
    emit stateChanged();
}

QString CSVHandler::getState() {
    return this->state;
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

void CSVHandler::setDuplicateSMS(int duplicateSMS) {
    this->duplicateSMS = duplicateSMS;
    emit duplicateSMSChanged();
}

int CSVHandler::getDuplicateSMS() {
    return this->duplicateSMS;
}

void CSVHandler::setInsertedSMS(int insertedSMS) {
    this->insertedSMS = insertedSMS;
    emit insertedSMSChanged();
}

int CSVHandler::getInsertedSMS() {
    return this->insertedSMS;
}

void CSVHandler::setInsertedCalls(int insertedCalls) {
    this->insertedCalls = insertedCalls;
    emit insertedCallsChanged();
}

int CSVHandler::getInsertedCalls() {
    return this->insertedCalls;
}

void CSVHandler::workerFinished() {
    this->working--;
}

void CSVHandler::insertFinished() {
    this->working--;
    this->setState(QString("DONE! You may quit now."));
}

void CSVHandler::parseFile() {
    qRegisterMetaType<GlogEventList>("GlogEventList");
    qDebug() << "CSVHandler::parseFile() called, registered QList<GlogEvent*>";
    if (this->working == 0) {
        this->setState(QString("Parsing"));

        // This is called when the qml is activated, reset some state
        this->setSeenSMS(0);
        this->setSeenCSVDuplicates(0);
        this->setSeenGroups(0);
        this->setNewGroups(0);
        this->setDuplicateSMS(0);
        this->setInsertedSMS(0);
        this->setInsertedCalls(0);

        this->working = 1;
        CSVWorker *csvWorker = new CSVWorker(this->getFilePath());

        // Required methods
        connect(csvWorker, &CSVWorker::parseFileCompleted, this, &CSVHandler::insertGlogEvents);
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

void CSVHandler::insertGlogEvents(GlogEventList glogevents) {
    qDebug() << "Inserting glogevents:" << glogevents.size();

    if (this->working == 1) {
        this->working = 2;
        InsertWorker *insertWorker = new InsertWorker(glogevents);

        // Required methods
        connect(insertWorker, &InsertWorker::finished, insertWorker, &QObject::deleteLater);
        connect(insertWorker, &InsertWorker::finished, this, &CSVHandler::insertFinished);
        // Relay state to qml
        connect(insertWorker, &InsertWorker::seenGroupsChanged, this, &CSVHandler::setSeenGroups);
        connect(insertWorker, &InsertWorker::newGroupsChanged, this, &CSVHandler::setNewGroups);
        connect(insertWorker, &InsertWorker::duplicateSMSChanged, this, &CSVHandler::setDuplicateSMS);
        connect(insertWorker, &InsertWorker::insertedSMSChanged, this, &CSVHandler::setInsertedSMS);
        connect(insertWorker, &InsertWorker::insertedCallsChanged, this, &CSVHandler::setInsertedCalls);
        insertWorker->start();

        this->setState(QString("Inserting"));
    } else {
        qDebug() << "Refuse to run another thread!";
    }
}
