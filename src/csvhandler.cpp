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
    bool inCell = false;
    char c;
    int seenCells = 0;

    for (int i=0; i<csvData.size(); i++) {
        c = csvData.at(i);

        if (c == ';') {
            seenCells++;
            inCell = !inCell;

            switch (seenCells) {
                // ID is the first one
                case 1:
                    qDebug() << "got ID" << cell;
                    msg->id = cell.toInt(&convOk);
                    if (!convOk) {
                        // TODO: Error handling!
                    }
                    break;
                case 2:
                    msg->eventTypeName = cell;
                default:
                    qDebug()  << "Unhandled cell count" << seenCells << cell;
            }

            // And reset the state a bit
            stack.push_back(cell);
            cell = "";
        } else if (c == '\n') {
            qDebug() << "Hit newline with seenCells" << seenCells;
            if (seenCells == ROW_LENGTH - 1 && !inCell) {
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
        // XXX: This check should not suffice, though " is escaped as ""
        // if the message has """lol""" type of content in it
        } else if ((c == '"' && csvData.at(i - 1) == '"') || (c != '"')) {
            cell.push_back(c);
        }
    }
    qDebug() << seenCells;
    return messages;
}
