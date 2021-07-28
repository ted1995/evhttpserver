#include"workers.h"
#include<event2/event.h>
#include<evhttp.h>
#include<unistd.h>
#include<glog/logging.h>
#include"handler.h"


void Worker::Wexit()
{
    if(evbase)
    {
        event_base_loopexit(evbase,nullptr);
    }
}


void exit_cb(int fd,short events,void* arg)
{
    Worker* w=(Worker*)arg;
    //从管道中读取到一个字节，退出evbase循环监听事件
    char buf[1];
    if(read(fd,buf,1)!=1)
    {
        LOG(ERROR)<<"退出管道发生错误\n";
    }
    w->Wexit();
}


void Worker::init(int fd)
{
    //创建一个event——base
    evbase=event_base_new();
    //创建evhttp并关联到event——base
    struct evhttp* http=evhttp_new(evbase);
    //为服务器监听socket创建event事件，设置事件回调函数，然后event加入event——base监听，当监听fd有事件发生时，说明有客户端请求连接
    //event——base会将此event放入就绪队列，调用对应的回调函数接受客户端的连接，并创建相关event，将客户端fd加入event——base中监听。
    int r=evhttp_accept_socket(http,fd);

    //遍历请求句柄结构体数组，对特定的URI设置回调函数
    for(request_handler_t* h=request_handlers;h&&h->path;h++)
    {
        //为特定的URI设置一个回调函数
        evhttp_set_cb(http,h->path,h->handler,h->arg);
    }

    //设置通用请求回调函数，URI没有设置回调函数的就会来到这里
    evhttp_set_gencb(http, general_request_handler, NULL);

    pipe(exitPipe);
    //创建一个退出管道事件处理器，当检测到管道有读事件，调用退出回调函数，结束evbase循环
    exitEvent = event_new(evbase, exitPipe[0], EV_READ|EV_PERSIST, exit_cb, (void*)this);
    //将管道退出事件加入监听队列
    event_add(exitEvent, NULL);
}

void Worker::run()
{
    //主循环监听事件，并自动调用回调函数

    //EVLOOP_NONBLOCK，处理当前激活的事件，然后退出loop，如果没有激活的事件，就直接退出
    //EVLOOP_ONCE,等待到第一个激活事件，处理激活的event，处理完就退出loop
    while(1)
    {
        LOG(INFO)<<"thread"<<this->num<<endl;
        event_base_loop(evbase,EVLOOP_ONCE);
        LOG(INFO)<<"thread"<<this->num<<endl;
    }
    
    event_free(exitEvent);
    event_base_free(evbase);
}

void Worker::Stop()
{
    char buf[1]={'-'};
    write(exitPipe[1],buf,1);
}