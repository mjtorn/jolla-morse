#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QDir>
#include <QString>

class CSVHandler : public QObject
{
    Q_OBJECT
public:
    explicit CSVHandler(QObject *parent = 0);
    Q_INVOKABLE QStringList getCSVFiles();

signals:

public slots:

};

#endif // CSVHANDLER_H
