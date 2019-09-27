#include <QApplication>
#include <QSqlError>
#include <QSqlRecord>

#include "log.h"
#include "sql.h"

Sql::Sql(QObject *parent) :
    QObject(parent)
{
    db = QSqlDatabase::addDatabase("QPSQL", QApplication::applicationName());
    db.setConnectOptions(QString("application_name=%1").arg(QApplication::applicationName()));
}

Sql::~Sql()
{
    db.close();
    QSqlDatabase::removeDatabase(QApplication::applicationName());
}

void Sql::connect(const QString &host, const int &port, const QString &user, const QString &password)
{
    db.setHostName(host);
    db.setPort(port);
    db.setUserName(user);
    db.setPassword(password);
    reconnect(true);
}

void Sql::reconnect(bool logInfoConnection)
{
    if (db.open()) {
        if (logInfoConnection) {
            logNotice(QString("Connect (%1:%2 [%3])")
                      .arg(db.hostName())
                      .arg(db.port())
                      .arg(db.userName()));
        }
        SuccessConnect();
        query = QSqlQuery(db);
        query.setForwardOnly(true);
    } else {
        logError(db.lastError().text());
        ErrorConnect();
    }
}

bool Sql::isOpen()
{
    return db.isOpen();
}

bool Sql::execute(const QString &script)
{
    if (!this->query.exec(script)) {
        logError(this->query.lastError().text());
        reconnect();
        return false;
    }
    return true;
}

QList<Sql::Record> Sql::select(const QString &script)
{
    QList<Sql::Record> res;
    if (execute(script)) {
        while (query.next()) {
            Sql::Record record;
            QSqlRecord rec = query.record();
            for (int i = 0; i < rec.count(); ++i) {
                record.append(Field(rec.fieldName(i), rec.value(i)));
            }
            res.append(record);
        }
    }
    return res;
}

bool Sql::rollback(QString id)
{
    return execute(QString("select pg_terminate_backend(%1)").arg(id));
}

bool Sql::reloadConf()
{
    return execute("select pg_reload_conf();");
}

bool Sql::setParam(const QString &name, const QString &value)
{
    return execute(QString("alter system set %1 to '%2';")
                   .arg(name)
                   .arg(value));
}

bool Sql::resetParam(const QString &name)
{
    return execute(QString("alter system reset %1;")
                   .arg(name));
}

bool Sql::resetAllParam()
{
    return execute("alter system reset all;");
}

QStringList Sql::getBlock(QString pid)
{
    QStringList res = QStringList();
    foreach (Record rBlock, select(QString("select pid from pg_locks where transactionid in "
                                           "(select transactionid from pg_locks where pid = %1) "
                                           "and pid <> %1;")
                                   .arg(pid))) {
        res.append(rBlock.get("pid").value().toString());
    }
    return res;
}


Sql::Field::Field(QString name, QVariant value) :
    m_name(name),
    m_value(value)
{
}

QString Sql::Field::name() const
{
    return m_name;
}

void Sql::Field::setName(const QString &value)
{
    m_name = value;
}

QVariant Sql::Field::value() const
{
    return m_value;
}

void Sql::Field::setValue(const QVariant &value)
{
    m_value = value;
}

bool Sql::Field::isNull() const
{
    return name().isEmpty();
}

Sql::Field &Sql::Field::null()
{
    static Field _null;
    return _null;
}


Sql::Field &Sql::Record::get(const int &index)
{
    if (index >= 0 && index < count()) {
        return operator [](index);
    }
    return Field::null();
}

Sql::Field &Sql::Record::get(const QString &name)
{
    for (int i = 0; i < count(); ++i) {
        Field &field = get(i);
        if (field.name() == name) {
            return field;
        }
    }
    return Field::null();
}
