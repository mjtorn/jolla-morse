#ifndef CSVHANDLER_H
#define CSVHANDLER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QString>

class CSVHandler : public QObject
{
    Q_OBJECT
public:
    explicit CSVHandler(QObject *parent = 0);
    Q_INVOKABLE QStringList getCSVFiles();
    Q_INVOKABLE void setFile(QString filename);
    Q_INVOKABLE QString getFile();
    Q_INVOKABLE void parseFile();
private:
    QString filepath;
    QFile file;

signals:

public slots:

};

#endif // CSVHANDLER_H
