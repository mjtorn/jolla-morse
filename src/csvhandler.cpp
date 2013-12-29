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

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
    // Come from CSVWorker
    this->workerRunning = false;
    this->readBytes = 0;
    this->seenEntries = 0;
    this->seenSMS = 0;

    // This is us
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
        csvWorker->start();
    } else {
        qDebug() << "Refuse to run another thread!";
    }
}

void CSVHandler::insertMessages(MessageList messages) {
    qDebug() << "Inserting messages:" << messages.size();
    // Only debug obviously
    // TODO: Actual functionality!
    this->insertedSMS = messages.size();
    emit insertedSMSChanged(messages.size());
}
