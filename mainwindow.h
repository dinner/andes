#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QSqlDatabase>
#include <QTcpSocket>
#include <QDomDocument>

namespace Ui {
class MainWindow;
}
class myThead;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QTcpSocket* getSocket(){
        return m_socket;
    }
private:
    Ui::MainWindow *ui;
    QListWidget* m_listWid;
    QSqlDatabase m_db;
    myThead* m_pThread;
    QTcpSocket* m_socket;

private:
    bool readXMLFile();
    void getConfigPathAndDataPath(QDomDocument*);//读取节点
    bool connectToDb();
    bool connectToServer();
    bool openXmlFile();
    bool changeSave(QString,QString);
    void closeEvent(QCloseEvent*);

private slots:
    void writeAMesToListWidget(QString);
    void clearWidget();//清空widget
    void socketAcceptMes();
    void startWork();
//    void socketError();//socket错误
};

#endif // MAINWINDOW_H
