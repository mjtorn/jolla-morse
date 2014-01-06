#include "glogevent.h"

GlogEvent::GlogEvent(QObject *parent) :
    QObject(parent)
{
    this->SMS_TYPE = QString("RTCOM_EL_EVENTTYPE_SMS_MESSAGE");
    this->CALL_TYPE = QString("RTCOM_EL_EVENTTYPE_CALL");
    this->CALL_MISSED_TYPE = QString("RTCOM_EL_EVENTTYPE_CALL_MISSED");
}
