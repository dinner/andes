#ifndef QUTIL_H
#define QUTIL_H

#include <QObject>

class QUtil : public QObject
{
    Q_OBJECT
public:
    explicit QUtil(QObject *parent = 0);

signals:

public slots:

public:
    static QString global_serverIp;//服务端ip
    static int global_port;//服务端端口
    static int global_timer;//时间戳
    static QString global_dbIp;
    static int global_dbPort;//
    static QString global_dbUser;
    static QString global_dbPass;
    static QString global_dbDbName;//
    static QString global_coalId;
    static QString global_countryId;
    static QString global_dir;
    static QString global_kbh;
    static bool global_test;//
};

#endif // QUTIL_H
