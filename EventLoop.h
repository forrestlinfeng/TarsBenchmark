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
