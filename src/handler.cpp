
#include "handler.h"
#include <string>
#include <stdlib.h>
#include <memory.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>    

#include<fstream>

#include <glog/logging.h>

#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>

static string rootpath="/home/wang/root";


extern std::unordered_map<string,string> users;


string http_response(string& str)
{
    string filepath=rootpath+str;
    //stat取得指定文件的文件属性，文件属性存储在结构体file_stat里
    struct stat file_stat;
    if (stat(filepath.c_str(), &file_stat) < 0)
    {
        LOG(INFO)<<"空文件属性:"<<strerror(errno)<<endl;
    }
    //宏S_IROTH代表其他人拥有读权限
    if (!(file_stat.st_mode & S_IROTH))
    {
        LOG(INFO)<<"无读权限，禁止访问"<<endl;
    }
    //S_ISDIR是否是目录
    if (S_ISDIR(file_stat.st_mode))
    {
        LOG(INFO)<<"是目录"<<endl;
    }
    LOG(INFO)<<"访问的文件存在"<<endl;
    
    //读取html文件中的内容
    fstream fs(filepath.c_str());
    stringstream ss;
    ss<<fs.rdbuf();
    string filedata=ss.str();
    return filedata;
}


void parse_content(char* p,string &name,string &passwd)
{
    p+=5;
    for (; *p != '&'; p++)
    {
        name.push_back(*p);
    }
    p+=10;
    for (; *p != '\0'; p++)
    {
        passwd.push_back(*p);
    }
}


void general_request_handler(struct evhttp_request *req, void *arg) 
{
    

    const struct evhttp_uri* evhttp_uri = evhttp_request_get_evhttp_uri(req);
    char url[8192];
    //const_cast是强制类型转换，去掉变量的常量属性
    evhttp_uri_join(const_cast<struct evhttp_uri*>(evhttp_uri), url, 8192);
    string str(url);
    string filedata;
    if(str=="/")
    {
        //get请求，请求判断界面

        str+="judge.html";
        filedata=http_response(str);
    }
    else if(str=="/0")
    {
        //get请求，请求注册界面

        str.erase(str.end()-1);
        str+="register.html";
        filedata=http_response(str);
    }
    else if(str=="/3")
    {
        //post请求，请求登录界面
        
        //先获得http请求数据中的正文内容
        evhttp_request_uri(req);
        size_t inputSize=EVBUFFER_LENGTH(req->input_buffer);
        char buf[inputSize+1]={'\0'};
        memcpy(buf,EVBUFFER_DATA(req->input_buffer),inputSize);
        //解析用户名与密码
        string name,passwd;
        parse_content(&buf[0],name,passwd);
        cout<<"name"<<name<<endl;
        cout<<"passwd"<<passwd<<endl;
        //先校验数据库中是否有同user名的
        //没有同名
        if(users.find(name)==users.end())
        {
            cout<<"此用户未注册"<<endl;
            //从数据库连接池中取出一个实例，向数据库中插入注册的用户名与密码,，同时加入本地mapsss
            sqlPool* sqlpool=sqlPool::get_instance();
            MYSQL* mysql=sqlpool->get_MYSQL();
            cout<<"获得MYSQL实例："<<mysql<<endl;
            string sqlCommand="INSERT INTO user(username,passwd) VALUES('";
            sqlCommand+=name;
            sqlCommand+="','";
            sqlCommand+=passwd;
            sqlCommand+="')";
            if(mysql_query(mysql,sqlCommand.c_str()));
            {
                cout<<"注册用户的用户名与密码成功存入数据库"<<endl;
            }
            sqlpool->free_MYSQL(mysql);
            users[name]=passwd;
            //制作回应报文
            str.erase(str.end()-1);
            str+="log.html";
            filedata=http_response(str);
        }
        //有同名
        else
        {
            str.erase(str.end()-1);
            str+="registererror.html";
            filedata=http_response(str);
        }
    }
    else if(str=="/1")
    {
        //get请求，请求登录页面
        str.erase(str.end()-1);
        str+="log.html";
        filedata=http_response(str);
    }
    else if(str=="/2")
    {
        //post请求，请求主页

        //先判断账号和密码是否正确
        //先获得http请求数据中的正文内容
        evhttp_request_uri(req);
        size_t inputSize=EVBUFFER_LENGTH(req->input_buffer);
        char buf[inputSize+1]={'\0'};
        memcpy(buf,EVBUFFER_DATA(req->input_buffer),inputSize);
        string name,passwd;
        parse_content(&buf[0],name,passwd);
        if(users.find(name)==users.end())
        {
            //没找到name，登录失败，返回登录界面
            str.erase(str.end()-1);
            str+="logerror.html";
            filedata=http_response(str);
        }
        else
        {
            //找到了name，再比对密码对不对
            if(users[name]==passwd)
            {
                //密码相同，返回主页
                str.erase(str.end()-1);
                str+="welcome.html";
                filedata=http_response(str);
            }
            else
            {
                //密码不对
                //返回登录错误界面
                //密码相同，返回主页
                str.erase(str.end()-1);
                str+="logerror.html";
                filedata=http_response(str);
            }
        }
        
    }
    else
    {
        cout<<"str:"<<str<<endl;
        evhttp_send_error(req, 403, "Forbidden");
    }
    //制作http回应报文
    struct evkeyvalq *output_headers=evhttp_request_get_output_headers(req);
    evhttp_add_header(output_headers, "Content-Type", "text/html");
    struct evbuffer *evb = evbuffer_new();
    evbuffer_add_printf(evb, "%s", filedata.c_str());
    evhttp_send_reply(req, 200, "OK", evb);
    evbuffer_free(evb);  
}



