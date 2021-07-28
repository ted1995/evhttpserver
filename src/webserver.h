#pragma once
#include<iostream>
using namespace std;
#include<vector>
#include<thread>
#include<memory>
#include"workers.h"
#include"sqlpool.h"



class webServer
{
public:
    webServer(int port, int backlog, int threadNum);

    void logInit();

    void start();

    int openServer();

    void startWorkers();

    void waitWorkers();

private:
    int port_m;//端口号 
    int backlog_m;//最大监听数量
    int fd_m;//监听fd
    int threadNum_m;//线程池线程数量

    Worker workers[100];//工作者
    vector<shared_ptr<thread>> threadpool;//线程池
};
