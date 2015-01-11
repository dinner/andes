#ifndef MYTHEAD_H
#define MYTHEAD_H

#include <QThread>
#include <QTcpSocket>

class QFile;
class QDateTime;

enum sendResult{NoDataSend=0,sendFail,sendSuc,fileOpenFail};
enum orderCode{e_deviceStatus=70,e_realData,e_dayRun,
        e_monthCheck,e_flag=75,e_palarm,e_deviceDefine,e_staff=82};//命令编码
typedef enum sendResult SR;
typedef enum orderCode  OC;

class myThead : public QThread
{
    Q_OBJECT

    typedef QByteArray (myThead::*Func_Construct)(QByteArray line);//定义函数指针
public:
    explicit myThead(QObject *parent = 0);

signals:
    void writeToListView(QString);
    void clearWidget();

public slots:
    void heartTimeOut();//心跳事件
    void socketError(QAbstractSocket::SocketError socketError);//socket错误
    void connectToTheServer();//连接服务器
private:
    void run();
    void initTheData();
    void oper(OC type,QFile* file,QString strFile,QString str);
    bool reConnectToServer();//重连

    SR file_oper(QFile* file,QString fileName,OC code,Func_Construct pFunc);

    QByteArray deviceStatus_construct(QByteArray);
    QByteArray realData_construct(QByteArray line);
    QByteArray dayRun_construct(QByteArray);
    QByteArray monthCheck_construct(QByteArray line);
    QByteArray flag_construct(QByteArray line);
    QByteArray palarm_construct(QByteArray line);
    QByteArray deviceDefine_construct(QByteArray line);
    QByteArray staff_construct(QByteArray line);
    QByteArray deviceStatus_construct_str(QString line);

    QDateTime getCurrentTime();
    QString getCurrentTime_str();
    QByteArray intToByte(int);
    bool send(QString strCoal,int type,QByteArray oByteData);
    QByteArray createData(int count,QByteArray btArray);
    QTcpSocket* m_socket;
    int m_count;
    bool m_bIsConnect;//是否连接服务器
};

#endif // MYTHEAD_H
