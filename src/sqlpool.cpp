#include"sqlpool.h"


//获取数据库连接池实例的互斥量
std::mutex mtx;
//单例模式的互斥量
std::mutex mtx_sql;
//数据库连接池的条件变量
std::condition_variable cv;

//全局变量users，存储用户的注册信息，方便进行注册与登录校验
std::unordered_map<string,string> users;

//静态成员变量由于存储在静态存储区，所以需要在编译时完成定义，但是类内的只是申明，需要在类外再进行一次定义（分配内存），甚至初始化
//这是静态成员变量必须要做的定义，但是最好放在cpp文件中进行，因为h头文件容易被包含，这样此语句就会被包含文件重复编译，造成sqlpool的重复定义。
sqlPool* sqlPool::sqlpool=nullptr;

void sqlPool::init_sqlList(string url,string user,string passwd,string dbname,int port)
{
    //初始化数据库的相关配置
    this->url=url;
    this->user=user;
    this->passwd=passwd;
    this->dbname=dbname;
    this->port=port;
    this->free=0;
    this->used=0;
    this->maxsize=10;

    //初始化数据库连接实例
    for(int i=0;i<maxsize;i++)
    {
        MYSQL* mysql=mysql_init(nullptr);
        mysql=mysql_real_connect(mysql,url.c_str(),user.c_str(),passwd.c_str(),dbname.c_str(),port,nullptr,0);
        if(mysql==nullptr)
        {
            LOG(INFO)<<"创建MYSQL实例失败"<<endl;
        }
        sqlList.push_back(mysql);
        free++;
    }
    LOG(INFO)<<"数据库连接池创建成功"<<endl;
}


void sqlPool::mysqlRresultInit()
{
    MYSQL* mysql=nullptr;
    mysql=sqlpool->get_MYSQL();
    if(mysql==nullptr)
    {
        LOG(INFO)<<"获取MYSQL实例失败"<<endl;
    }
    if(mysql_query(mysql,"SELECT username,passwd FROM user"))
    {
        LOG(INFO)<<"使用此数据库实例查询数据"<<endl;
    }
    LOG(INFO)<<"获取完整的检索结果集"<<endl;
    MYSQL_RES* result=mysql_store_result(mysql);
    //返回字段数
    //int num=mysql_num_fields(result);
    //LOG(INFO)<<"字段数："<<num<<endl;
    //返回代表字段的数组
    //MYSQL_FIELD* field=mysql_fetch_fields(result);
    //从结果集中将用户名与密码存入map中，方便校验
    while(MYSQL_ROW row=mysql_fetch_row(result))
    {
        string temp1(row[0]);
        string temp2(row[1]);
        LOG(INFO)<<"user:"<<temp1<<" passwd:"<<temp2<<endl;
        users[temp1]=temp2;
    }
    sqlpool->free_MYSQL(mysql);
}


int sqlPool::get_free()
{
    return sqlList.size();
}


sqlPool* sqlPool::get_instance()
{
    //如果此连接池实例为空，创建一个，如果不为空，直接返回此实例
    if(sqlpool==nullptr)
    {
        mtx_sql.lock();
        if(sqlpool==nullptr)
        {
            sqlpool=new sqlPool();
        }
        mtx_sql.unlock();
    }
    return sqlpool;
}



MYSQL* sqlPool::get_MYSQL()
{
    cout<<"开始获取MYSQL"<<endl;
    MYSQL* mysql=nullptr;
    //条件变量，阻塞，直到连接池中有数据库实例可用
    std::unique_lock<std::mutex> lock(mtx);
    cout<<"被锁阻塞？"<<endl;
    while(sqlList.empty())
    {
        LOG(INFO)<<"MYSQL连接池为空"<<endl;
        cv.wait(lock);
    }
    mysql=sqlList.front();
    sqlList.pop_front();
    free--;
    used++;
    return mysql;
}

void sqlPool::free_MYSQL(MYSQL* mysql)
{
    if(mysql==nullptr)
    {
        return;
    }
    std::unique_lock<std::mutex> lock(mtx);
    sqlList.push_back(mysql);
    free++;
    used--;
    cv.notify_one();
}