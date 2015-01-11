#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "QUtil.h"
#include "mythead.h"
#include <qfile.h>
#include <QMessageBox>
#include <QSqlError>
#include <QHostAddress>
#include <QDateTime>
#include <QCloseEvent>
#include <QTimer>

#define XMLFILE         "/config.xml"

QDomDocument* m_doc;
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_listWid = ui->listWidget;
    m_pThread = NULL;
    this->setWindowTitle("采集器");
    //
    bool bIsReadConfigSuc = this->readXMLFile();
    /*
    if(bIsReadConfigSuc)
    {
        bool bConServer = false;
        bConServer= this->connectToServer();
        if(!bConServer){
            while(true){

            }
        }
        this->startWork();
    }
    */
//    bool bConServer = false;
//    bConServer= this->connectToServer();
//    if(!bConServer){
//        while(true){
//            m_socket->abort();
//            m_socket->close();
//            m_socket = NULL;
//            m_socket = new QTcpSocket();
//            m_socket->connectToHost(QHostAddress(QUtil::global_serverIp), QUtil::global_port);
//            if(m_socket->waitForConnected()){
//                connect(m_socket,SIGNAL(readyRead()),this,SLOT(socketAcceptMes()),Qt::UniqueConnection);
//               break;
//            }
//            this->writeAMesToListWidget("服务器连接失败");
//        }
//    }
    QTimer::singleShot(10000,this,SLOT(startWork()));//只执行一次，间隔5秒
//    this->startWork();
}

MainWindow::~MainWindow()
{
    delete ui;
}
//连接xmlfile
bool MainWindow::readXMLFile(){
    QString strFile = QCoreApplication::applicationDirPath();
    strFile += XMLFILE;
    QFile* pFile = new QFile(strFile);
    if (!pFile->open(QFile::ReadOnly| QIODevice::Text))
    {
        return false;
    }
    QDomDocument* dom = new QDomDocument();
    if (!dom->setContent(pFile))
    {
        pFile->close();
        qDebug("配置文件读取失败");
        this->writeAMesToListWidget("配置文件读取失败");
        return false;
    }
    else{
        this->getConfigPathAndDataPath(dom);
        qDebug("配置文件读取成功");
        this->writeAMesToListWidget("配置文件读取成功");
        return true;
    }
}
//读取节点
void MainWindow::getConfigPathAndDataPath(QDomDocument* dom){
    QDomElement docElem = dom->documentElement();
    QDomNode n = docElem.firstChild();
    while (!n.isNull())
    {
        if (n.isElement())
        {
            QDomElement e = n.toElement();
            qDebug()<<qPrintable(e.tagName())
                   <<qPrintable(e.attribute("tag"));
            QString strValue = e.text();
            QString strTag = e.tagName();
            if (strTag == "ServerIP")
            {
                QUtil::global_serverIp = strValue;
            }
            if (strTag == "ServerPortNo")
            {
                QUtil::global_port = strValue.toInt();
            }
            if (strTag == "DataBaseIP")
            {
                QUtil::global_dbIp = strValue;
            }
            if (strTag == "DataBasePort")
            {
                QUtil::global_dbPort = strValue.toInt();
            }
            if (strTag == "DataBaseID")
            {
                QUtil::global_dbUser = strValue;
            }
            if (strTag == "DataBasePass")
            {
                QUtil::global_dbPass = strValue;
            }
            if (strTag == "DataBaseName")
            {
                QUtil::global_dbDbName = strValue;
            }
            if(strTag == "Country"){
                QUtil::global_countryId = strValue;
            }
            if (strTag == "TimeOut")
            {
                QUtil::global_timer = strValue.toInt();
            }
            if (strTag == "Log")
            {

            }
            if(strTag == "Test"){
                QUtil::global_test = strValue.toInt() == 1?true:false;
            }
            if(strTag == "Dir"){
                QUtil::global_dir = strValue;
            }
            if(strTag == "Kbh"){
                QUtil::global_kbh = strValue;
            }
            n = n.nextSibling();//
        }
    }
}
//连接db
bool MainWindow::connectToDb(){
    m_db = QSqlDatabase::addDatabase("QODBC");
    m_db.setDatabaseName(QString("DRIVER={SQL SERVER};"
                                 "SERVER=%1;" //
                                 "DATABASE=%2;"//
                                 "UID=%3;"           //
                                 "PWD=%4;"        //
                                 ).arg(QUtil::global_dbIp)
                         .arg(QUtil::global_dbDbName)
                         .arg(QUtil::global_dbUser)
                         .arg(QUtil::global_dbPass)
                         );
    if (!m_db.open())
    {
        qDebug("fail");
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
                              m_db.lastError().databaseText(), QMessageBox::Cancel);
        this->writeAMesToListWidget("数据库读取失败");
        return false;
    }
    else{
        qDebug("suc");
        this->writeAMesToListWidget("数据库读取成功");
        return true;
    }
}
//连接server
bool MainWindow::connectToServer(){
    m_socket = new QTcpSocket();
    connect(m_socket,SIGNAL(readyRead()),this,SLOT(socketAcceptMes()),Qt::UniqueConnection);
    m_socket->connectToHost(QHostAddress(QUtil::global_serverIp), QUtil::global_port);
    bool bIs= m_socket->waitForConnected();
    if(bIs){
        this->writeAMesToListWidget("服务端连接成功");
    }
    else{
        this->writeAMesToListWidget("服务端连接失败");
    }
    return bIs;
}
//线程开启
void MainWindow::startWork(){
    m_pThread = new myThead(this);
//    m_socket->moveToThread(m_pThread);
    connect(m_pThread,SIGNAL(writeToListView(QString)),this,SLOT(writeAMesToListWidget(QString)),Qt::UniqueConnection);
    connect(m_pThread,SIGNAL(clearWidget()),this,SLOT(clearWidget()),Qt::UniqueConnection);
    m_pThread->start();
}
//
void MainWindow::writeAMesToListWidget(QString str){
    QDateTime current_date_time = QDateTime::currentDateTime();
    QString strDateTime = current_date_time.toString("yyyy-MM-dd hh:mm:ss ");
    QString strMes = str.toUtf8();
    strMes = strDateTime + strMes;
    QListWidgetItem* pItem = new QListWidgetItem(strMes);
    m_listWid->addItem(pItem);
}
//关闭窗口事件
void MainWindow::closeEvent(QCloseEvent *e){
    QMessageBox mes(QMessageBox::Warning,"提示","确定退出程序？",QMessageBox::Yes|QMessageBox::No,NULL);
    if(mes.exec() == QMessageBox::Yes){
        qDebug("quit");
        if(m_pThread != NULL){
            m_pThread->quit();
            m_pThread->wait();
        }
        e->accept();
    }
    else{
        qDebug("not quit");
        e->ignore();
    }
}
//
void MainWindow::clearWidget(){
    while(1){
        int nRow = m_listWid->count();
        if(nRow){
        delete m_listWid->takeItem(0);
        }
        else{
            break;
        }
    }
}
//
void MainWindow::socketAcceptMes(){
    this->writeAMesToListWidget("收到服务器指令");
    QByteArray btArray = m_socket->readAll();
    QString strReceive(btArray);
    qDebug()<<strReceive;
    QStringList strList = strReceive.split(",");
    QString strIp = strList[1];//ip
    QString strPort = strList[2];//端口
    //ip和端口重置
    QUtil::global_serverIp = strIp;
    QUtil::global_port = strPort.toInt();
    //停止现在的信息传输
    m_pThread->wait();
    m_socket->close();
    if(m_pThread != NULL){
        m_pThread->exit();
    }
    //连接到新的server
    m_socket->connectToHost(QHostAddress(QUtil::global_serverIp),QUtil::global_port);
    if(m_socket->waitForConnected()){//连接成功
        this->writeAMesToListWidget("新的服务端连接成功");
    }
    else{
        this->writeAMesToListWidget("新的服务端连接失败");
    }
    this->changeSave(strIp,strPort);
}
//打开xml文件
bool MainWindow::openXmlFile()
{
    QString FilePath = QCoreApplication::applicationDirPath();
    QString strPath = FilePath+"/config.xml";
    QFile file( strPath );
    if( !file.open( QFile::ReadOnly | QFile::Text  ) )
    {
        qDebug() << QObject::tr("error::ParserXML->OpenXmlFile->file.open->%s\n") << FilePath;
        return false;
    }
    m_doc = new QDomDocument();
    if( !m_doc->setContent(&file) )
    {
        qDebug() << QObject::tr("error::ParserXML->OpenXmlFile->doc.setContent\n") << FilePath;

        file.close();
        return false;
    }
    file.close();
    return true;
}
//修改文件
bool MainWindow::changeSave(QString strIp,QString strPort){
    if(!this->openXmlFile()){
        return false;
    }
    QDomElement root = m_doc->documentElement();
    QDomNode n = root.firstChild();
    QString strTag = root.tagName();
    while(!n.isNull()){
         if(n.isElement()){
             if(n.nodeName() == "ServerIP")//ip地址
             {
             QDomNode oldNode = n.firstChild();
             n.firstChild().setNodeValue(strIp);
             QDomNode newNode = n.firstChild();
             n.replaceChild(newNode,oldNode);
             }
             if(n.nodeName() == "ServerPortNo"){//port端口号
                 QDomNode oldNode = n.firstChild();
                 n.firstChild().setNodeValue(strPort);
                 QDomNode newNode = n.firstChild();
                 n.replaceChild(newNode,oldNode);
             }
         }
         n = n.nextSibling();
    }
    QString FilePath = QCoreApplication::applicationDirPath();
    QString strPath = FilePath+"/config.xml";
    QFile fileXml( strPath );
    if(!fileXml.open(QFile::WriteOnly | QFile::Truncate)){
        return false;
    }
    QTextStream ts(&fileXml);
    ts.reset();
    m_doc->save(ts,4,QDomNode::EncodingFromTextStream);
    fileXml.close();
    return true;
}
////socket错误
//void MainWindow::socketError(){

//}

