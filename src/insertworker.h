#ifndef INSERTWORKER_H
#define INSERTWORKER_H

#include <QMultiHash>
#include <QSet>
#include <QString>
#include <QThread>
#include <QObject>

#include "CommHistory/group.h"
#include "message.h"

class InsertWorker : public QThread
{
    Q_OBJECT
private:
    Q_INVOKABLE void run();
    Q_INVOKABLE QMultiHash<QString, Message*> getGrouped(MessageList messages);
    Q_INVOKABLE CommHistory::Group createGroup(QStringList remoteUids);
    Q_INVOKABLE QSet<QString> handleGroups(MessageList messages);
    MessageList messages;
    QMultiHash<QString, Message*> groups;

public:
    explicit InsertWorker(MessageList messages);

signals:
    void seenGroupsChanged(int seenGroups);
    void newGroupsChanged(int newGroups);
    void duplicateSMSChanged(int duplicateSMS);
    void insertedSMSChanged(int insertedSMS);
};

#endif // INSERTWORKER_H
