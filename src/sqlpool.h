#pragma once
#include<mysql/mysql.h>
#include<iostream>
#include<list>
#include<string>
#include<mutex>
#include<condition_variable>
#include<glog/logging.h>
#include<unordered_map>
using namespace std;



//因为连接池类要统一管理整个应用程序的连接，所以整个系统中只需要维护一个连接池对象，如果定义了多个连接池对象，那么每一个对象都可以创建一个自己的连接池，无法统一处理

//懒汉式单例模式，双重锁定
class sqlPool
{
public:
    //静态公共方法，获取连接池实例
    static sqlPool* get_instance();
    
    //初始化数据库连接池
    void init_sqlList(string url,string user,string passwd,string dbname,int port);
    
    //获取数据库中存储的用户数据
    void mysqlRresultInit();

    //获得一个MYSQL实例
    MYSQL* get_MYSQL();

    //释放一个MYSQL实例
    void free_MYSQL(MYSQL* mysql);

    int get_free();

private:
    //单例模式，构造函数私有化
    sqlPool(){}
    //静态实例，所有连接池对象共享。
    static sqlPool* sqlpool;
    //
    list<MYSQL*> sqlList;
    //
    string url;
    //
    string user;
    //
    string passwd;
    //
    string dbname;
    //
    int port;
    //
    int maxsize;
    //
    int free;
    //
    int used;
};


