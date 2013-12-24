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
    Q_INVOKABLE void setFile(QString filename);
    Q_INVOKABLE QString getFile();
private:
    QString filepath;

signals:

public slots:

};

#endif // CSVHANDLER_H
