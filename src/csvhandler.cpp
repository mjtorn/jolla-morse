#include "csvhandler.h"

#include <QDebug>
#include <QByteArray>
#include <QIODevice>
#include <QDir>
#include <QFile>
#include <QList>
#include <QString>

QString BASEDIR_NAME = QString("/etc/mersdk/share/");
QString FIRST_LINE = QString("\"ID\";\"EventTypes.name\";\"Events.Outgoing\";\"storage_time\";\"start_time\";\"end_time\";\"is_read\";\"flags\";\"bytes_sent\";\"bytes_received\";\"local_uid\";\"local_name\";\"remote_uid\";\"remote_name\";\"channel\";\"free_text\";\"group_uid\"\r\n");
QString SMS_TYPE = QString("RTCOM_EL_EVENTTYPE_SMS_MESSAGE");

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
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

int CSVHandler::getCSVBytes() {
    return csvData.size();
}

void CSVHandler::parseFile() {
    QList<MessageObject*> messages;
    QFile file(filepath);
    file.open(QIODevice::ReadOnly);

    qDebug() << file.size();
    QString line = file.readLine();
    int compResult = line.compare(FIRST_LINE);

    if (compResult != 0) {
        qDebug() << "Need to fail visibly here: " << compResult;
        goto end;
    }
    qDebug() << "OK!";

    // Does not read all from beginning, just as we want
    csvData = file.readAll();
    emit readBytesChanged(csvData.size());

    messages = actualParse();
    qDebug() << messages;

    end:
        file.close();
        for (int i=0; i<messages.size(); i++) {
            delete messages.at(i);
        }
}

QList<MessageObject*> CSVHandler::actualParse() {
    int ROW_LENGTH = 17;

    MessageObject *msg = new MessageObject();
    QList<MessageObject*> messages;
    QString cell;
    bool inQuotes = false;
    char c;
    char c1;
    char c2;
    int quoteDepth = 0;
    int seenCells = 0;

    for (int i=0; i<csvData.size(); i++) {
        c = csvData.at(i);
        if (i > 1) {
            c1 = csvData.at(i - 1);
        }
        if (i > 2) {
            c2 = csvData.at(i - 2);
        }

        if (c == ';') {
            if (c1 == '"' && inQuotes) {
                inQuotes = false;
            }

            if (inQuotes) {
                cell.push_back(c);
            } else {
                seenCells++;

                switch (seenCells) {
                    // ID is the first one
                    case 1:
                        qDebug() << "got ID" << cell;
                        msg->id = toInt(cell);
                        break;
                    case 2:
                        qDebug() << "got type" << cell;
                        msg->eventTypeName = cell;
                        break;
                    case 3:
                        msg->isOutgoing = (bool) toInt(cell);
                        qDebug() << "got outgoing" << msg->isOutgoing;
                        break;
                    case 4:
                        msg->storageTime = toLL(cell);
                        qDebug() << "got storageTime" << msg->storageTime;
                        break;
                    case 5:
                        msg->startTime = toLL(cell);
                        qDebug() << "got startTime" << msg->startTime;
                        break;
                    case 6:
                        msg->endTime = toLL(cell);
                        qDebug() << "got endTime" << msg->endTime;
                        break;
                    case 7:
                        msg->isRead = (bool) toInt(cell);
                        qDebug() << "got isRead" << msg->isRead;
                        break;
                    case 8:
                        msg->flags = toInt(cell);
                        qDebug() << "got flags" << msg->flags;
                        break;
                    case 9:
                        msg->bytesSent = toInt(cell);
                        qDebug() << "got bytesSent" << msg->bytesSent;
                        break;
                    case 10:
                        msg->bytesReceived = toInt(cell);
                        qDebug() << "got bytesReceived" << msg->bytesReceived;
                        break;
                    case 11:
                        msg->localUID = cell;
                        qDebug() << "got localUID" << msg->localUID;
                        break;
                    case 12:
                        msg->localName = cell;
                        qDebug() << "got localName" << msg->localName;
                        break;
                    case 13:
                        msg->remoteUID = cell;
                        qDebug() << "got remoteUID" << msg->remoteUID;
                        break;
                    case 14:
                        msg->remoteName = cell;
                        qDebug() << "got remoteName" << msg->remoteName;
                        break;
                    case 15:
                        msg->channel = cell;
                        qDebug() << "got channel" << msg->channel;
                        break;
                    case 16:
                        msg->freeText = cell;
                        qDebug() << "got freeText" << msg->freeText;
                        break;
                    default:
                        qDebug()  << "Unhandled cell count" << seenCells << cell;
                }

                // 1 extra is enough to see what broke
                Q_ASSERT_X(seenCells < ROW_LENGTH + 1, "cells", "cells overflow");

                // And reset the state a bit
                cell = "";
            }
        } else if (c == '\n') {
            qDebug() << "Hit newline with seenCells" << seenCells << "and cell" << cell;
            if (seenCells == ROW_LENGTH - 1 && c1 == '\r' && c2 == '"') {
                if (msg->eventTypeName.compare(SMS_TYPE) == 0) {
                    msg->groupUID = cell;
                    qDebug() << "got groupUID" << msg->groupUID;
                    messages.push_back(msg);
                } else {
                    delete msg;
                }
                msg = new MessageObject();

                // Reset state
                seenCells = 0;
                inQuotes = false;
                cell = "";
            }
        } else if (c == '"') {
            // " is escaped as "", this should work until int overflow.
            // Make sure we're not in a ;""; type of situation, though.
            if (c1 == '"' && c2 != ';') {
                quoteDepth++;
                if (quoteDepth % 2 == 1) {
                    cell.push_back(c);
                }
            } else {
                quoteDepth = 0;
            }

            if (quoteDepth == 0 && c1 == ';') {
                inQuotes = true;
            }
        } else {
            cell.push_back(c);
        }
    }
    qDebug() << seenCells;
    return messages;
}

quint32 toInt(QString s) {
    quint32 i = 0;
    bool convOk = false;

    i = s.toInt(&convOk);
    if (!convOk) {
        // TODO: Error handling!
        QByteArray sa = s.toUtf8();
        Q_ASSERT_X(convOk, sa.constData(), "Bad conversion");
    }

    return i;
}

quint64 toLL(QString s) {
    quint64 i = 0;
    bool convOk = false;

    i = s.toLongLong(&convOk);
    if (!convOk) {
        // TODO: Error handling!
        QByteArray sa = s.toUtf8();
        Q_ASSERT_X(convOk, sa.constData(), "Bad conversion");
    }

    return i;
}
