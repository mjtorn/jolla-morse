#include "csvhandler.h"

#include <QDir>
#include <QString>

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
}

QStringList CSVHandler::getCSVFiles() {
    QDir qdir = QDir(QString("/etc/mersdk/share/"));

    QStringList list = qdir.entryList();
    return list;
}

