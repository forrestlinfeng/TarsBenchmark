/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          EventLoop.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.11
* Description:        事件管理器主循环
* History:    
*                     2016.11.11             首次创建
**************************************************************************/
#ifndef _EVENTLOOP_H_
#define _EVENTLOOP_H_

namespace bm
{
    class Transport;
    class EPollLoop
    {
    public:
        EPollLoop() : _epFd(-1), _pevs(NULL) {}
        ~EPollLoop();

        void initialize(int max_connections = 10240);
        void loop(int waittime = 100);
        void addConn(Transport *conn, uint32_t events);
        void delConn(Transport *conn, uint32_t events);
        void modConn(Transport *conn, uint32_t events);  
    private:
        int _epFd;
        int _max_connections;
        struct epoll_event* _pevs;
    };
};
#endif
