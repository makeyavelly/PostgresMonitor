#ifndef LOG_H
#define LOG_H

#ifndef DEVELOPER
#define DEVELOPER
#endif

void logDebug(QString debugText);
void logInfo(QString infoText, QString tag = QString());
void logNotice(QString noticeText);
void logWarning(QString warningText);
void logError(QString errorText);
void logCritical(QString criticalText);

#endif // LOG_H
