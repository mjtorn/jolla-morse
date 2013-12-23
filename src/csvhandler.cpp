#include "csvhandler.h"

#include <QDir>
#include <QString>

QStringList getCSVFiles() {
    QDir qdir = QDir(QString("/etc/mersdk/share/"));

    QStringList list = qdir.entryList();
    return list;
}

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
}
