#pragma once

class Worker
{
private:
    struct event_base* evbase;
    struct event* exitEvent;
    
public:
	int num;

    int exitPipe[2];

    void init(int fd);
    
    void run();

    void Stop();

    void Wexit();
};


