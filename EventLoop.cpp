/**
 * Tencent is pleased to support the open source community by making Tars available.
 *
 * Copyright (C) 2016THL A29 Limited, a Tencent company. All rights reserved.
 *
 * Licensed under the BSD 3-Clause License (the "License"); you may not use this file except 
 * in compliance with the License. You may obtain a copy of the License at
 *
 * https://opensource.org/licenses/BSD-3-Clause
 *
 * Unless required by applicable law or agreed to in writing, software distributed 
 * under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 */
#include "CommDefs.h"
#include "EventLoop.h"
#include "Transport.h"

namespace bm
{
    void EPollLoop::initialize(int max_connections)
    {
        _max_connections = max_connections;
        _epFd = epoll_create(_max_connections + 1);
        if (_pevs != NULL)
        {
            delete[] _pevs;
        }
        _pevs = new epoll_event[_max_connections + 1];
    }

    EPollLoop::~EPollLoop()
    {
        if (_pevs != NULL)
        {
            delete[] _pevs;
            _pevs = NULL;
        }

        if (_epFd != 0)
        {
            close(_epFd);
            _epFd = -1;
        }
    }

    void EPollLoop::addConn(Transport* conn, uint32_t events)
    {
        struct epoll_event ev;
        ev.data.u64 = (uint64_t)conn;
        ev.events   = events | EPOLLET;
        epoll_ctl(_epFd, EPOLL_CTL_ADD, conn->getfd(), &ev);
    }

    void EPollLoop::delConn(Transport* conn, uint32_t events)
    {
        struct epoll_event ev;
        ev.data.u64 = (uint64_t)conn;
        ev.events   = events | EPOLLET;
        epoll_ctl(_epFd, EPOLL_CTL_DEL, conn->getfd(), &ev);
    }

    void EPollLoop::modConn(Transport* conn, uint32_t events)
    {
        struct epoll_event ev;
        ev.data.u64 = (uint64_t)conn;
        ev.events   = events | EPOLLET;
        epoll_ctl(_epFd, EPOLL_CTL_MOD, conn->getfd(), &ev);
    }

    void  EPollLoop::loop(int waittime)
    {
        int num = epoll_wait(_epFd, _pevs, _max_connections + 1, waittime);
        for (int i = 0; i < num; ++i)
        {
            const epoll_event& ev = _pevs[i];
            if(ev.data.u64 == 0) continue;
            Transport *conn = (Transport*)ev.data.u64;

            // 读写一次
            conn->handleRead();
            conn->handleWrite();

            // err
            if (ev.events & EPOLLERR)
            {
                conn->handleError();
            }
        }
    }
};
