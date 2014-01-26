#include "csvworker.h"

#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QSet>
#include <QString>

QString FIRST_LINE = QString("\"ID\";\"EventTypes.name\";\"Events.Outgoing\";\"storage_time\";\"start_time\";\"end_time\";\"is_read\";\"flags\";\"bytes_sent\";\"bytes_received\";\"local_uid\";\"local_name\";\"remote_uid\";\"remote_name\";\"channel\";\"free_text\";\"group_uid\"\r\n");

CSVWorker::CSVWorker(QString filepath) :
    QThread()
{
    this->filepath = filepath;
    this->seenEntries = 0;
    this->seenCSVDuplicates = 0;
}

int CSVWorker::getCSVBytes() {
    return csvData.size();
}

int CSVWorker::getSeenEntries() {
    return this->seenEntries;
}

int CSVWorker::getSeenSMS() {
    return this->seenSMS;
}

int CSVWorker::getSeenCSVDuplicates() {
    return this->seenCSVDuplicates;
}

void CSVWorker::run() Q_DECL_OVERRIDE {
    QString filepath = this->filepath;
    qDebug() << "CSVWorker::run(" << filepath << ")";
    GlogEventList glogevents;

    QFile file(filepath);
    file.open(QIODevice::ReadOnly);

    qDebug() << file.size();
    QString line = file.readLine();
    int compResult = line.compare(FIRST_LINE);

    if (compResult != 0) {
        qCritical() << "Need to fail visibly here: " << compResult;
        goto end;
    }
    qDebug() << "OK!";

    // Does not read all from beginning, just as we want
    csvData = file.readAll();
    emit readBytesChanged(csvData.size());

    glogevents = actualParse();
    qDebug() << glogevents.size();

    end:
        file.close();

    emit parseFileCompleted(glogevents);
}
GlogEventList CSVWorker::actualParse() {
    int ROW_LENGTH = 17;

    GlogEvent *glogEvent = new GlogEvent();
    GlogEventList glogevents;
    QSet<quint32> csvIds;
    QByteArray cell;
    bool inQuotes = false;
    char c;
    char c1;
    char c2;
    int quoteDepth = 0;
    int seenCells = 0;
    int rowNum = 0;

    // Some entries don't have "remote_uid" so they must be resolved through "group_uid"
    QHash<QString, QString> groupUidMap;

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
                        //qDebug() << "got ID" << cell;
                        glogEvent->id = toInt(cell);
                        break;
                    case 2:
                        //qDebug() << "got type" << cell;
                        glogEvent->eventTypeName = QString::fromUtf8(cell);
                        break;
                    case 3:
                        glogEvent->isOutgoing = (bool) toInt(cell);
                        //qDebug() << "got outgoing" << glogEvent->isOutgoing;
                        break;
                    case 4:
                        glogEvent->storageTime = toLL(cell);
                        //qDebug() << "got storageTime" << glogEvent->storageTime;
                        break;
                    case 5:
                        glogEvent->startTime = toLL(cell);
                        //qDebug() << "got startTime" << glogEvent->startTime;
                        break;
                    case 6:
                        glogEvent->endTime = toLL(cell);
                        //qDebug() << "got endTime" << glogEvent->endTime;
                        break;
                    case 7:
                        glogEvent->isRead = (bool) toInt(cell);
                        //qDebug() << "got isRead" << glogEvent->isRead;
                        break;
                    case 8:
                        glogEvent->flags = toInt(cell);
                        //qDebug() << "got flags" << glogEvent->flags;
                        break;
                    case 9:
                        glogEvent->bytesSent = toInt(cell);
                        //qDebug() << "got bytesSent" << glogEvent->bytesSent;
                        break;
                    case 10:
                        glogEvent->bytesReceived = toInt(cell);
                        //qDebug() << "got bytesReceived" << glogEvent->bytesReceived;
                        break;
                    case 11:
                        glogEvent->localUID = QString::fromUtf8(cell);
                        //qDebug() << "got localUID" << glogEvent->localUID;
                        break;
                    case 12:
                        glogEvent->localName = QString::fromUtf8(cell);
                        //qDebug() << "got localName" << glogEvent->localName;
                        break;
                    case 13:
                        glogEvent->remoteUID = QString::fromUtf8(cell);
                        //qDebug() << "got remoteUID" << glogEvent->remoteUID;
                        break;
                    case 14:
                        glogEvent->remoteName = QString::fromUtf8(cell);
                        //qDebug() << "got remoteName" << glogEvent->remoteName;
                        break;
                    case 15:
                        glogEvent->channel = QString::fromUtf8(cell);
                        //qDebug() << "got channel" << glogEvent->channel;
                        break;
                    case 16:
                        // FIXME: The parser should handle a glogevent whose content is only ;
                        if (cell.size() == 0) {
                            glogEvent->freeText = QString(";");
                        } else {
                            glogEvent->freeText = QString::fromUtf8(cell);
                        }
                        //qDebug() << "got freeText" << glogEvent->freeText;
                        break;
                    default:
                        //qDebug()  << "Unhandled cell count" << seenCells << cell;
                        // FIXME: This is just horrible, only because of a glogevent whose content is ;
                        seenCells--;
                }

                // 1 extra is enough to see what broke
                Q_ASSERT_X(seenCells < ROW_LENGTH + 1, "cells", "cells overflow");

                // And reset the state a bit
                cell = "";
            }
        } else if (c == '\n') {
            rowNum++;
            //qDebug() << "Hit newline with seenCells" << seenCells << "and cell" << cell;
            if (seenCells == ROW_LENGTH - 1 && c1 == '\r' && c2 == '"') {
                if (glogEvent->isSupported()) {
                    glogEvent->groupUID = cell;
                    //qDebug() << "got groupUID" << glogEvent->groupUID;

                    // Don't insert empty remote uids or such into map
                    if (glogEvent->remoteUID.compare(QString("")) != 0 && !groupUidMap.contains(glogEvent->groupUID)) {
                        groupUidMap.insert(glogEvent->groupUID, glogEvent->remoteUID);
                    }

                    //qDebug() << rowNum + 1 << glogEvent->id << glogEvent->remoteUID << glogEvent->freeText;
                    if (!csvIds.contains(glogEvent->id)) {
                        glogevents.push_back(glogEvent);
                        csvIds.insert(glogEvent->id);
                    } else {
                        this->seenCSVDuplicates++;
                        emit seenCSVDuplicatesChanged(this->seenCSVDuplicates);
                        delete glogEvent;
                    }

                    if (glogevents.size() % 100 == 0) {
                        emit seenSMSChanged(glogevents.size());
                        usleep(10 * 1000); // Sleep 10ms to make sure this works
                    }
                } else {
                    delete glogEvent;
                }
                this->seenEntries++;
                emit seenEntriesChanged(this->seenEntries);
                glogEvent = new GlogEvent();

                // Reset state
                seenCells = 0;
                inQuotes = false;
                cell = "";
            } else {
                // Or maybe it's a glogevent \n
                cell.push_back(c);
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
    // Resolve the missing ones
    //qDebug() << "keys in map" << groupUidMap.uniqueKeys().size();
    for (int i=0; i<glogevents.size(); i++) {
        if (glogevents.at(i)->remoteUID.compare(QString("")) == 0) {
            //qDebug() << "Found empty" << glogevents.at(i)->id;
            QString remoteUid = groupUidMap.value(glogevents.at(i)->groupUID);
            //qDebug() << remoteUid << "for" << glogevents.at(i)->groupUID;
            glogevents.at(i)->remoteUID = remoteUid;
        }
        // XXX: This char* thing is probably bad :D
        Q_ASSERT_X(glogevents.at(i)->remoteUID.compare(QString("")) != 0, (char*)glogevents.at(i)->id, "empty remoteUID");

        if (glogevents.at(i)->isCall()) {
            Q_ASSERT(glogevents.at(i)->freeText.size() == 0);
        } else {
            Q_ASSERT(glogevents.at(i)->freeText.size() >= 1);
        }
    }
    //qDebug() << seenCells;
    emit seenSMSChanged(glogevents.size());
    return glogevents;
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
