#include<string>
#include<evhttp.h>
#include"webserver.h"
#include<glog/logging.h>
#include<fcntl.h>
#include<errno.h>


webServer::webServer(int port, int backlog, int threadnum):port_m(port),backlog_m(backlog),threadNum_m(threadnum)
{
    LOG(INFO)<<"webserver初始化成功"<<endl;
}

void webServer::logInit()
{
    google::InitGoogleLogging("log");//日志前缀
    FLAGS_log_dir = "/home/wang/lieventServer/log";
    FLAGS_logbufsecs = 5;//延迟写
    FLAGS_max_log_size = 100;//日志文件最大为100MB
    FLAGS_stop_logging_if_full_disk = true;//磁盘满时停止记录
    FLAGS_colorlogtostderr = true;

    google::SetLogDestination(google::INFO, "../log/info_");
    //google::SetLogDestination(google::WARNING, (basePath + "WARNING_").data());
    //google::SetLogDestination(google::ERROR, (basePath + "ERROR_").data());
    //google::SetLogDestination(google::FATAL, (basePath + "FATAL_").data());

    google::SetStderrLogging(google::INFO);
}


int webServer::openServer()
{
    int listenfd=socket(AF_INET,SOCK_STREAM,0);
    if(listenfd<0)
    {
        LOG(ERROR)<<"监听socket创建失败"<<endl;
        return -1;
    }
    LOG(INFO)<<listenfd<<endl;

    int one = 1;
    //设置套接字的属性，例如SO_REUSEADDR，可以快速进行端口的复用
    //一般套接字在closesocket后会等待time_wait时间，此时间段内新套接字想绑定这个端口时无法成功的
    //设置这个套接字后就可以在time_wait时间内再次绑定此端口
    int r = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&one, sizeof(int));
    if(r<0)
    {
        LOG(ERROR)<<"设置通用套接字失败"<<errno<<endl;
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_m);

    r = bind(listenfd, (struct sockaddr*)&addr, sizeof(addr));
    if (r < 0) {
        LOG(ERROR)<<"bind error "<<endl;
        return -1;
    }

    r = listen(listenfd, backlog_m);
    if (r < 0) {
        LOG(ERROR)<<"listen error"<<endl;
        return -1;
    }
 
    int flags;
    //设置监听socket为非阻塞。
    if ((flags = fcntl(listenfd, F_GETFL, 0)) < 0
        || fcntl(listenfd, F_SETFL, flags | O_NONBLOCK) < 0) {
        LOG(ERROR)<<"fcntl O_NONBLOCK error"<<endl;
        return -1;
    }
    LOG(INFO)<<"返回监听socket"<<endl;
    return listenfd;
}


void webServer::start()
{
    LOG(INFO)<<"创建服务器的监听socket"<<endl;
    fd_m=openServer();
    LOG(INFO)<<"创建多线程，每个线程一个base，监听listenfd"<<endl;
    startWorkers();
    waitWorkers();
}



void webServer::startWorkers() 
{
    for(int i=0; i<threadNum_m; i++) 
    {
        //每一个线程做一个标记
        workers[i].num=i;
        //初始化evhttp对应的event——base，设置好相应的回调函数
        workers[i].init(fd_m);
        //创建线程池，每一个线程阻塞在dispatch，等待监听fd有事件响应
        threadpool.push_back(shared_ptr<thread>(new thread(&Worker::run, &workers[i])));
    }
}

void webServer::waitWorkers() 
{
    for(int i=0; i<threadNum_m; i++) 
    {
        //主线程等待所有子线程都执行完毕才退出。
        threadpool[i]->join();
        threadpool[i].reset();
    }
}