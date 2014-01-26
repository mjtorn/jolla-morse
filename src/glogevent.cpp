#include "glogevent.h"

#include <QDateTime>
#include <QString>

GlogEvent::GlogEvent(QObject *parent) :
    QObject(parent)
{
    this->SMS_TYPE = QString("RTCOM_EL_EVENTTYPE_SMS_MESSAGE");
    this->CALL_TYPE = QString("RTCOM_EL_EVENTTYPE_CALL");
    this->CALL_MISSED_TYPE = QString("RTCOM_EL_EVENTTYPE_CALL_MISSED");
}

bool GlogEvent::isSupported() {
    return (this->eventTypeName.compare(this->SMS_TYPE) == 0 \
             || this->eventTypeName.compare(this->CALL_TYPE) == 0 \
             || this->eventTypeName.compare(this->CALL_MISSED_TYPE) == 0);
}

bool GlogEvent::isCall() {
    return (this->eventTypeName.compare(this->CALL_TYPE) == 0 \
             || this->eventTypeName.compare(this->CALL_MISSED_TYPE) == 0);
}

QString GlogEvent::startTimeToString() {
    QDateTime startTime;
    startTime.setTime_t(this->startTime);

    return startTime.toString(Qt::TextDate);
}
