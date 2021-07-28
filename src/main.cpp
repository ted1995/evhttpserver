#include"webserver.h"
#include<iostream>
using namespace std;

int main()
{
    string url="localhost";
    string user="root";
    string passwd="123456";
    string dbname="yourdb";
    int port=3306;
    //获得连接池实例
    sqlPool* sqlpool=sqlPool::get_instance();
    cout<<"单例模式下连接池对象："<<sqlpool<<endl;
    //初始化连接池
    sqlpool->init_sqlList(url,user,passwd,dbname,port);
    LOG(INFO)<<"预先获取数据库中数据，便于比较"<<endl;
    sqlpool->mysqlRresultInit();
    LOG(INFO)<<"开始初始化webServer"<<endl;
    webServer webserver(12345,1024,8);
    //初始化日志系统
    webserver.logInit();
    //开始
    webserver.start();
}