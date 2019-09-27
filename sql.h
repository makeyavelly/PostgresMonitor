#ifndef SQL_H
#define SQL_H

#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>


class Sql : public QObject
{
public:
    class Field
    {
    public:
        Field(QString name = QString(), QVariant value = QVariant());

        QString name() const;
        void setName(const QString &value);

        QVariant value() const;
        void setValue(const QVariant &value);

        bool isNull() const;

        static Field &null();

    private:
        QString m_name;
        QVariant m_value;
    };

    class Record : public QList<Field>
    {
    public:
        Field &get(const int &index);
        Field &get(const QString &name);
    };

    Q_OBJECT

public:
    Sql(QObject *parent = 0x0);
    ~Sql();

    void connect(const QString &host, const int &port, const QString &user, const QString &password);
    void reconnect(bool logInfoConnection = false);
    bool isOpen();

    bool execute(const QString &script);
    QList<Record> select(const QString &script);
    bool rollback(QString id);
    bool reloadConf();

    bool setParam(const QString &name, const QString &value);
    bool resetParam(const QString &name);
    bool resetAllParam();

    QStringList getBlock(QString pid);

private:
    QSqlDatabase db;
    QSqlQuery query;

signals:
    void SuccessConnect();
    void ErrorConnect();
};

#endif // SQL_H
