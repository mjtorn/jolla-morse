#include "csvhandler.h"

#include <QDir>
#include <QString>

CSVHandler::CSVHandler(QObject *parent) :
    QObject(parent)
{
}

QStringList CSVHandler::getCSVFiles() {
    QStringList filter;
    filter << "*.csv";

    QDir qdir = QDir(QString("/etc/mersdk/share/"));
    qdir.setNameFilters(filter);

    QStringList list = qdir.entryList();
    return list;
}

