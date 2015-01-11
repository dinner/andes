#include "qutil.h"

int QUtil::global_port = 0;
int QUtil::global_timer = 0;
int QUtil::global_dbPort = 0;
QString QUtil::global_serverIp = "";
QString QUtil::global_dbDbName = "";
QString QUtil::global_dbUser = "";
QString QUtil::global_dbPass = "";
QString QUtil::global_dbIp = "";
QString QUtil::global_coalId = "";
QString QUtil::global_countryId = "";
QString QUtil::global_dir = "";//文件目录
QString QUtil::global_kbh = "";//
bool QUtil::global_test = false;
QUtil::QUtil(QObject *parent) :
    QObject(parent)
{
}
