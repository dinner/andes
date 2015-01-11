#include "mythead.h"
#include "qutil.h"
#include <QFileInfo>
#include <QDebug>
#include <QFile>
#include <QDateTime>
#include "mainwindow.h"
#include <QTimer>
#include <QCoreApplication>
#include <QTextCodec>
#include <QHostAddress>

#define MAX_NUM 200             //最大值
#define SPLIT   "&&&"           //分隔符
#define PARA_SPLIT  ','
#define STR_NULL     ""
#define HEART_TIMER 50123       //心跳时间

#define DEVICE_STATUS        "设备状态"
#define REALTIME             "实时"
#define DAYRUN               "轨迹"
#define MONTHCHECK           "月考勤"
#define FLAG                 "下井人数"
#define PALARM               "报警表"
#define DEVICE_DEFINE        "设备定义"
#define STAFF                "人员档案"

QString file_deviceStatus = "";        //设备状态
QString file_realData ="";      //实时数据
QString file_dayRun ="";  //轨迹数据    yyyyMMddhhmm
QString file_monthCheck="";//月考勤数据
QString file_flag = "";//下井人数
QString file_palarm ="";//报警表  pb    yyyyMMddhhmm
QString file_deviceDefine = "";//采集点定义信息
QString file_staff = "";//人员档案

QDateTime m_currentTime;//当期时间
QFile* m_file_deviceStatus;
QFile* m_file_realData;
QFile* m_file_dayRun;
QFile* m_file_monthCheck;
QFile* m_file_flag;
QFile* m_file_palarm;
QFile* m_file_deviceDefine;
QFile* m_file_staff;

QString m_strCoal;
QTimer* heart_timer;//心跳时间
QTimer* timer;
bool m_bIsSend;//是否正在发送

myThead::myThead(QObject *parent) :
    QThread(parent)
{
    this->initTheData();
    m_bIsSend= false;
    heart_timer = new QTimer();
    connect(heart_timer,SIGNAL(timeout()),this,SLOT(heartTimeOut()),Qt::UniqueConnection);
    heart_timer->start(HEART_TIMER);
    m_count = 0;
    m_socket = NULL;
    m_bIsConnect = false;
    //    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    //    QTextCodec::setCodecForLocale(codec);

    timer = new QTimer();
    connect(timer,SIGNAL(timeout()),this,SLOT(connectToTheServer()),Qt::UniqueConnection);
    timer->start(5000);
}
void myThead::run(){
    while(1){
        if(!m_bIsSend && m_bIsConnect)//没有数据发送
        {
            m_currentTime = this->getCurrentTime();
            m_bIsSend = true;
            m_count = 0;
            QTime timeStart = QTime::currentTime();//开始时间
            if(!QUtil::global_test)
            {
                QDateTime dt = this->getCurrentTime();
                file_dayRun = QString("%1persontrack%2.txt").arg(QUtil::global_dir).arg(dt.toString("yyyyMMddhhmm"));
                file_palarm = QString("%1palert%2.txt").arg(QUtil::global_dir).arg(dt.toString("yyyyMMddhhmmss"));
            }
            qDebug()<<"file_deviceStatus地址"<<m_file_deviceStatus;
            oper(e_deviceStatus,m_file_deviceStatus,file_deviceStatus,DEVICE_STATUS);
            oper(e_realData,m_file_realData,file_realData,REALTIME);
            oper(e_dayRun,m_file_dayRun,file_dayRun,DAYRUN);
            oper(e_monthCheck,m_file_monthCheck,file_monthCheck,MONTHCHECK);
            oper(e_flag,m_file_flag,file_flag,FLAG);
            oper(e_palarm,m_file_palarm,file_palarm,PALARM);
            oper(e_deviceDefine,m_file_deviceDefine,file_deviceDefine,DEVICE_DEFINE);
            oper(e_staff,m_file_staff,file_staff,STAFF);
            m_bIsSend =false;
            QTime timeEnd = QTime::currentTime();//结束时间
            int nSec = timeStart.secsTo(timeEnd);
            emit writeToListView(QString("信息发送完成，耗时%1s,共发送%2条数据").arg(nSec).arg(m_count));
            m_currentTime = this->getCurrentTime();
            this->msleep(QUtil::global_timer);
        }
    }
}
//socket初始化
void myThead::connectToTheServer(){
    if(m_socket != NULL){
        m_socket->abort();
        m_socket->close();
        m_socket->deleteLater();
        m_socket = NULL;
    }
    m_socket = new QTcpSocket();
    connect(m_socket,SIGNAL(readyRead()),this,SLOT(socketAcceptMes()),Qt::UniqueConnection);
    m_socket->connectToHost(QHostAddress(QUtil::global_serverIp), QUtil::global_port);
    bool bIs= m_socket->waitForConnected();
    if(bIs){
        timer->stop();
        m_bIsConnect = true;
        emit writeToListView("服务端连接成功");
    }
    else{
        emit writeToListView("服务端连接失败");
    }
}

//数据初始化
void myThead::initTheData(){
    m_file_deviceStatus = NULL;
    m_file_realData = NULL;
    m_file_dayRun = NULL;
    m_file_monthCheck = NULL;
    m_file_flag = NULL;
    m_file_palarm = NULL;
    m_file_deviceDefine = NULL;
    m_file_staff = NULL;

    file_deviceStatus = QString("%1/devicestate.txt").arg(QUtil::global_dir);        //设备状态
    file_realData =QString("%1/realtimedata.txt").arg(QUtil::global_dir);      //实时数据
    file_dayRun = QString("%1/persontrack201210091423.txt").arg(QUtil::global_dir);  //轨迹数据    yyyyMMddhhmm
    file_monthCheck=QString("%1/attendace.txt").arg(QUtil::global_dir);//月考勤数据
    file_flag = QString("%1/enterprisestate.txt").arg(QUtil::global_dir);//下井人数
    file_palarm =QString("%1/palert20121009142330.txt").arg(QUtil::global_dir);//报警表  pb    yyyyMMddhhmm
    file_deviceDefine = QString("%1/device.txt").arg(QUtil::global_dir);//采集点定义信息
    file_staff = QString("%1/workerinfo.txt").arg(QUtil::global_dir);//人员档案

    m_file_deviceStatus = new QFile(file_deviceStatus);
    m_file_realData = new QFile(file_realData);
    m_file_dayRun = new QFile(file_dayRun);
    m_file_monthCheck = new QFile(file_monthCheck);
    m_file_flag = new QFile(file_flag);
    m_file_palarm = new QFile(file_palarm);
    m_file_deviceDefine = new QFile(file_deviceDefine);
    m_file_staff = new QFile(file_staff);
}

QByteArray myThead::deviceStatus_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//设备编码
    btAr.append(list[2]).append(PARA_SPLIT);//运行状态
    btAr.append(list[3]).append(PARA_SPLIT);//状态变动时间
    btAr.append(list[4]).append(PARA_SPLIT);//状态变动描述
    btAr.append(STR_NULL).append(PARA_SPLIT);//实时人数
    btAr.append(list[5]).append(PARA_SPLIT);//报警状态
    btAr.append(STR_NULL);
    return btAr;
}
QByteArray myThead::deviceStatus_construct_str(QString line){
    QByteArray btAr;
    QStringList list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//设备编码
    btAr.append(list[2]).append(PARA_SPLIT);//运行状态
    btAr.append(list[3]).append(PARA_SPLIT);//状态变动时间
    btAr.append(list[4]).append(PARA_SPLIT);//状态变动描述
    btAr.append(STR_NULL).append(PARA_SPLIT);//实时人数
    btAr.append(list[5]).append(PARA_SPLIT);//报警状态
    btAr.append(STR_NULL);
    return btAr;
}
QByteArray myThead::realData_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//标示卡号
    btAr.append(list[2]).append(PARA_SPLIT);//当前位置
    btAr.append(STR_NULL).append(PARA_SPLIT);//来源位置
    btAr.append(list[3]).append(PARA_SPLIT);//入井位置
    btAr.append(list[4]).append(PARA_SPLIT);//下井时间
    btAr.append(STR_NULL).append(PARA_SPLIT);//来源时间
    btAr.append(list[5]).append(PARA_SPLIT);//当前时间
    btAr.append(list[6]).append(PARA_SPLIT);//班次名
    btAr.append(list[7]).append(PARA_SPLIT);//入井时长
    btAr.append(list[8]).append(PARA_SPLIT);//是否报警
    btAr.append(STR_NULL).append(PARA_SPLIT);//报警时间
    btAr.append(list[10]);//是否出井
    return btAr;
}
QByteArray myThead::dayRun_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//标示卡号
    btAr.append(list[2]).append(PARA_SPLIT);//设备编码
    btAr.append(list[3]);//经过时间
    return btAr;
}
QByteArray myThead::monthCheck_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//标示卡号
    btAr.append(list[2]).append(PARA_SPLIT);//入井位置
    btAr.append(list[3]).append(PARA_SPLIT);//入井时间
    btAr.append(list[4]).append(PARA_SPLIT);//出井位置
    btAr.append(list[5]).append(PARA_SPLIT);//出井时间
    btAr.append(list[6]).append(PARA_SPLIT);//班次
    btAr.append(STR_NULL).append(PARA_SPLIT);//归属日期
    btAr.append(list[7]).append(PARA_SPLIT);//
    return btAr;
}
QByteArray myThead::flag_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//额定总人数
    btAr.append(list[2]).append(PARA_SPLIT);//当前井下总人数
    btAr.append(STR_NULL).append(PARA_SPLIT);//井下报警人数
    btAr.append(STR_NULL).append(PARA_SPLIT);//今日下井总人数
    btAr.append(list[3]);//刷新数据
    return btAr;
}
QByteArray myThead::palarm_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//标示卡号
    btAr.append(list[2]).append(PARA_SPLIT);//报警地点
    btAr.append(list[3]).append(PARA_SPLIT);//报警开始时间
    btAr.append(list[4]).append(PARA_SPLIT);//报警结束时间    //为空时后台有误
    btAr.append(list[5]).append(PARA_SPLIT);//报警类型
    btAr.append(list[8]);//处理意见
    return btAr;
}
QByteArray myThead::deviceDefine_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//设备编码
    btAr.append(list[3]).append(PARA_SPLIT);//位置描述
    btAr.append(list[4]).append(PARA_SPLIT);//测点额定人数
    btAr.append(list[5]).append(PARA_SPLIT);//测点停留报警时间
    btAr.append(STR_NULL);//限制进入人员识别卡号
    return btAr;
}
QByteArray myThead::staff_construct(QByteArray line){
    QByteArray btAr;
    QList<QByteArray> list = line.split(PARA_SPLIT);
    btAr.resize(0);
    btAr.append(QUtil::global_kbh).append(PARA_SPLIT);//矿编码
    btAr.append(list[1]).append(PARA_SPLIT);//标示卡号
    btAr.append(list[2]).append(PARA_SPLIT);//姓名
    btAr.append(list[3]).append(PARA_SPLIT);//部门编号
    btAr.append(STR_NULL).append(PARA_SPLIT);//灯架号
    btAr.append(STR_NULL).append(PARA_SPLIT);//灯号
    btAr.append(list[4]).append(PARA_SPLIT);//工号
    btAr.append(list[5]).append(PARA_SPLIT);//职务编号
    btAr.append(STR_NULL).append(PARA_SPLIT);//工种编号
    btAr.append(list[7]).append(PARA_SPLIT);//职称编号
    btAr.append(list[12]).append(PARA_SPLIT);//性别
    btAr.append(list[9]).append(PARA_SPLIT);//民族
    btAr.append(list[10]).append(PARA_SPLIT);//身份证号码
    btAr.append(list[11]).append(PARA_SPLIT);//出生日期
    btAr.append(list[13]).append(PARA_SPLIT);//学历
    btAr.append(list[19]).append(PARA_SPLIT);//相片名称
    btAr.append(list[16]).append(PARA_SPLIT);//电话
    btAr.append(list[8]).append(PARA_SPLIT);//籍贯
    btAr.append(list[14]).append(PARA_SPLIT);//专业
    btAr.append(STR_NULL).append(PARA_SPLIT);//班制
    btAr.append(list[17]).append(PARA_SPLIT);//干部标志
    btAr.append(list[15]).append(PARA_SPLIT);//社保号
    btAr.append(list[18]).append(PARA_SPLIT);//入职时间
    btAr.append(STR_NULL).append(PARA_SPLIT);//入井时长
    btAr.append(STR_NULL);//入井月规定次数
    return btAr;
}
//
void myThead::oper(OC type,QFile* file,QString strFile,QString str){
    qDebug()<<"file_deviceStatus地址"<<file;
    Func_Construct pFunc = NULL;
    switch(type){
    case e_deviceStatus:
        pFunc = &myThead::deviceStatus_construct;
        break;
    case e_realData:
        pFunc = &myThead::realData_construct;
        break;
    case e_dayRun:
        pFunc = &myThead::dayRun_construct;
        break;
    case e_monthCheck:
        pFunc = &myThead::monthCheck_construct;
        break;
    case e_flag:
        pFunc = &myThead::flag_construct;
        break;
    case e_palarm:
        pFunc = &myThead::palarm_construct;
        break;
    case e_deviceDefine:
        pFunc = &myThead::deviceDefine_construct;
        break;
    case e_staff:
        pFunc = &myThead::staff_construct;
    }
    enum sendResult result = file_oper(file,strFile,type,pFunc);
    switch(result){
    case sendFail:
        emit writeToListView(QString("%1数据发送失败").arg(str));
        break;
    case sendSuc:
        emit writeToListView(QString("%1数据发送成功").arg(str));
        break;
    case NoDataSend:
        emit writeToListView(QString("%1没有新数据").arg(str));
        break;
    default:
        emit writeToListView(QString("%1文件打开失败").arg(str));
    }
}
SR myThead:: file_oper(QFile* file,QString fileName,OC code,Func_Construct pFunc){
    QFileInfo oFileInfo(fileName);
    QDateTime lastModiTime = oFileInfo.lastModified();
    bool bIsSend = false;
    //    QFile* file = *pFile;
    //    if(lastModiTime >= m_currentTime){
    if(file == NULL){
        file = new QFile(fileName);
        if(!file->open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug()<<"open file failed！"<<endl;
            return fileOpenFail;
        }
    }
    if(!file->isOpen()){
        if(!file->open(QIODevice::ReadOnly | QIODevice::Text)){
            qDebug()<<"open file failed！"<<endl;
            return fileOpenFail;
        }
    }
    int nCount=0;
    QByteArray btArray;
    btArray.resize(0);
    file->readLine();
    file->readLine();
    QTextCodec *codec=QTextCodec::codecForName("GBK");
    while(!file->atEnd()){
        if(nCount != 0){
            btArray.append(SPLIT);
        }
        QByteArray line = file->readLine();
        QString strLine = codec->toUnicode(line);
        QByteArray btLine = (this->*pFunc)(strLine.toUtf8());
        btArray.append(btLine);
        m_count++;//数据添加
        nCount++;
        if(nCount % MAX_NUM == 0){
            QByteArray btSend = this->createData(nCount,btArray);
            bIsSend = this->send(m_strCoal,code,btSend);
            btArray.resize(0);
            nCount = 0;
        }
    }
    if(nCount != 0){
        QByteArray btSend = this->createData(nCount,btArray);
        bIsSend = this->send(m_strCoal,code,btSend);
    }
    file->seek(0);
    return bIsSend==true?sendSuc:sendFail;
}
//    else{
//        return NoDataSend;
//    }
//}

//获取当前时间
QDateTime myThead::getCurrentTime(){
    QDateTime dt;
    QTime time;
    QDate date;
    dt.setTime(time.currentTime());
    dt.setDate(date.currentDate());
    return dt;
}
//获取当前时间
QString myThead::getCurrentTime_str(){
    QDateTime dt;
    QTime time;
    QDate date;
    dt.setTime(time.currentTime());
    dt.setDate(date.currentDate());
    return dt.toString("yyyy-MM-dd hh:mm:ss");
}
//发送数据
bool myThead::send(QString strCoal,int type,QByteArray oByteData)
{
    int nLength = 0;
    QString strFirst = QUtil::global_countryId+"-"+QUtil::global_coalId+"-1";
    QByteArray btFirst = QByteArray(strFirst.toLatin1());
    int nFirstSize = btFirst.size();
    if(nFirstSize <20){
        int nDvalue = 20 - nFirstSize;
        for (int i = 0;i < nDvalue;i++)
        {
            btFirst.append("0");
        }
    }
    nLength = oByteData.size() + 8 + 20;
    QByteArray oByteArray;
    QByteArray oByteArrayLength = this->intToByte(nLength);
    QByteArray oByteArrayType = this->intToByte(type);
    oByteArray.append(oByteArrayLength).append(oByteArrayType).append(btFirst).append(oByteData);
    if (m_socket->state() != QAbstractSocket::UnconnectedState){//
        try{
            m_socket->write(oByteArray);
            m_socket->flush();
        }
        catch(QString exception){
            qDebug()<<exception;
        }
        return true;
    }
    else{//未连接
        emit writeToListView("连接断开，尝试重新连接服务器");
        bool bIs = this->reConnectToServer();//重新连接
        if(bIs){
            //重连成功
            emit writeToListView("重连成功");
            m_socket->write(oByteArray);
            m_socket->flush();
            return true;
        }
        else{
            //重连失败
            emit writeToListView("重连失败，请检测服务器是否开启服务");
            return false;
        }
    }
}
//构建数据
QByteArray myThead::createData(int count,QByteArray btArray){
    QByteArray btSend;
    QString strCurrentTime = this->getCurrentTime_str();
    btSend.append(strCurrentTime);
    btSend.append(SPLIT);
    btSend.append(QString("%1").arg(count));
    btSend.append(SPLIT);
    btSend.append(btArray);
    return btSend;
}
//重连
bool myThead::reConnectToServer(){
    //尝试重新连接 3次连接失败告知无法连接
    int reCoCount = 0;//重连次数
    while(reCoCount < 3){
        m_socket->abort();
        m_socket->close();
        m_socket = NULL;
        m_socket = new QTcpSocket();
        m_socket->connectToHost(QHostAddress(QUtil::global_serverIp), QUtil::global_port);
        if(m_socket->waitForConnected()){
            //               connect(m_socket,SIGNAL(readyRead()),this,SLOT(socketAcceptMes()),Qt::UniqueConnection);
            return true;
            break;
        }
        else{
            reCoCount++;
        }
    }
    return false;
}
//心跳事件
void myThead::heartTimeOut(){
    if(!m_bIsSend){
        int nType = 0;
        QByteArray bt_type = this->intToByte(nType);
        QString strPrefix =QUtil::global_countryId+"-"+QUtil::global_coalId+"-1";
        QByteArray bt_Prefix(strPrefix.toLatin1());
        int nPrefixLength = bt_Prefix.size();
        if(nPrefixLength < 20){
            int nDefault = 20-nPrefixLength;
            for(int i = 0;i < nDefault;i++){
                bt_Prefix.append("0");
            }
        }
        QString strContent = "hello";
        QByteArray btContent(strContent.toLatin1());
        int nLength = btContent.size()+20+8;
        QByteArray bt_length = this->intToByte(nLength);
        bt_length.append(bt_type).append(bt_Prefix).append(btContent);
        if(m_socket->state() !=  QAbstractSocket::UnconnectedState){
            m_socket->write(bt_length);
            m_socket->flush();
        }
        else{//未连接
            emit writeToListView("与服务器连接断开，正尝试重连");
            bool bIsReCon = this->reConnectToServer();
            if(bIsReCon){//重连成功
                emit writeToListView("重连成功");
            }
            else{//重连失败
                emit writeToListView("重连失败，请检查服务器服务是否启动");
            }
        }
    }
    emit clearWidget();
}
//
QByteArray myThead:: intToByte(int n){
    QByteArray oByte;
    oByte.resize(4);
    oByte[0] = (uchar)  (0x000000ff & n);
    oByte[1] = (uchar) ((0x0000ff00 & n) >> 8);
    oByte[2] = (uchar) ((0x00ff0000 & n) >> 16);
    oByte[3] = (uchar) ((0xff000000 & n) >> 24);
    return oByte;
}
//socket错误
void myThead::socketError(QAbstractSocket::SocketError socketError){

}


