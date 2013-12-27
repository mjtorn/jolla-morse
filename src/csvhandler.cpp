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
    QStringList stack;
    QString cell;
    bool convOk = false;
    char c;
    int quoteDepth = 0;
    int seenCells = 0;

    for (int i=0; i<csvData.size(); i++) {
        c = csvData.at(i);

        if (c == ';') {
            seenCells++;

            switch (seenCells) {
                // ID is the first one
                case 1:
                    qDebug() << "got ID" << cell;
                    msg->id = cell.toInt(&convOk);
                    if (!convOk) {
                        // TODO: Error handling!
                        QByteArray cCell = cell.toUtf8();
                        Q_ASSERT_X(convOk, cCell.constData(), "ID conversion");
                    }
                    break;
                case 2:
                    qDebug() << "got type" << cell;
                    msg->eventTypeName = cell;
                    break;
                default:
                    qDebug()  << "Unhandled cell count" << seenCells << cell;
            }

            // 3 is about as margin as debugging might need...
            Q_ASSERT_X(seenCells < ROW_LENGTH + 3, "cells", "cells overflow");

            // And reset the state a bit
            stack.push_back(cell);
            cell = "";
        } else if (c == '\n') {
            qDebug() << "Hit newline with seenCells" << seenCells;
            if (seenCells == ROW_LENGTH - 1 && csvData.at(i - 1) == '\r' && csvData.at(i - 2) == '"') {
                // Do something with the stack
                // and reset the cells
                seenCells = 0;
                stack.push_back(cell);
                cell = "";
                if (msg->eventTypeName.compare(SMS_TYPE) == 0) {
                    messages.push_back(msg);
                } else {
                    delete msg;
                }
                msg = new MessageObject();
            }
        } else if (c == '"') {
            // " is escaped as "", this should work until int overflow.
            if (csvData.at(i - 1) == '"') {
                quoteDepth++;
                if (quoteDepth % 2 == 1) {
                    cell.push_back(c);
                }
            } else {
                quoteDepth = 0;
            }
        } else {
            cell.push_back(c);
        }
    }
    qDebug() << seenCells;
    return messages;
}
