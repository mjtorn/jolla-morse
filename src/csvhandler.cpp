#include "csvhandler.h"

#include <QDebug>
#include <QFile>
#include <QDir>
#include <QString>

QString BASEDIR_NAME = QString("/etc/mersdk/share/");

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
    this->filepath = BASEDIR_NAME + filename;
}

QString CSVHandler::getFile() {
    return this->filepath;
}

void CSVHandler::parseFile() {
    QFile file(filepath);
    qDebug() << file.size();
}
