#include <syslog.h>
#include <QApplication>

#include "log.h"

#ifdef DEVELOPER
#include <QDebug>
#endif

void _logWrite(int level, QString message, QString tag = QString())
{
    openlog(QApplication::applicationName().toAscii().constData(), 0, LOG_USER);
    if (tag.isEmpty()) {
        syslog(level, "%s", message.toAscii().constData());
#ifdef DEVELOPER
        qDebug() << message;
#endif
    } else {
        syslog(level, "%s", QString("[%1] %2").arg(tag).arg(message).toAscii().constData());
#ifdef DEVELOPER
        qDebug() << QString("[%1] %2").arg(tag).arg(message);
#endif
    }
    closelog();
}


void logDebug(QString debugText)
{
    _logWrite(LOG_DEBUG, debugText);
}

void logInfo(QString infoText, QString tag)
{
    _logWrite(LOG_INFO, infoText, tag);
}

void logNotice(QString noticeText)
{
    _logWrite(LOG_NOTICE, noticeText);
}

void logWarning(QString warningText)
{
    _logWrite(LOG_WARNING, warningText, "WARNING");
}

void logError(QString errorText)
{
    _logWrite(LOG_ERR, errorText, "ERROR");
}

void logCritical(QString criticalText)
{
    _logWrite(LOG_CRIT, criticalText, "CRITICAL");
}
