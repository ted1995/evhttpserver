#pragma once
#include<evhttp.h>
#include<iostream>
#include<string>
#include"webserver.h"
#include"sqlpool.h"
using namespace std;


void general_request_handler(struct evhttp_request *req, void *arg);

//定义一个请求句柄函数指针为变量类型
typedef void(*request_handler)(struct evhttp_request* req,void* arg);

//请求句柄结构体
struct request_handler_t
{
    const char* path;
    request_handler handler;
    void* arg;
};

//请求句柄结构体数组
static request_handler_t request_handlers[]=
{
    {nullptr,nullptr,nullptr}
};