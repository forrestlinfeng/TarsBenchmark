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
#include <Transport.h>
#include "util/tc_common.h"
using namespace tars;

#define CONNECT_TIMEOUT_MS 3000
namespace bm
{
    int Transport::handleWrite()
    {
        if (!isValid())
        {
            return BM_SOCK_INVALID;
        }

        if (_connStatus == eConnecting)
        {
            _connStatus = eConnected;
        }

        // 发送上次剩余数据
        while(_sendBuffer.size() > 0)
        {
            int iRet = this->send(_sendBuffer.c_str(), _sendBuffer.size(), 0);
            if (iRet < 0)
            {
                Monitor::getInstance()->report(iRet);
                return iRet;
            }
            else if(iRet == 0)
            {
                break; // 缓冲区数据太多，先处理一下回包
            }

            _sendBuffer.erase(_sendBuffer.begin(), _sendBuffer.begin() + iRet);
        }
        return 0;
    }

    int Transport::handleRead()
    {
        if (getfd() == -1)
        {
            Monitor::getInstance()->report(BM_SOCK_INVALID);
            return BM_SOCK_INVALID;
        }

        if (_connStatus == eConnecting && !checkConnect())
        {
            close();
            Monitor::getInstance()->report(BM_SOCK_CONN_ERROR);
            return BM_SOCK_CONN_ERROR;
        }

        int iRet = 0;
        int64_t tCurTime = TNOWMS;
        char buff[MAX_RECVBUF_SIZE] = {0};
        while ((iRet = this->recv(buff, MAX_RECVBUF_SIZE, 0)) > 0)
        {
            _recvBuffer.append(buff, iRet);

            if (iRet > 0)
            {
                Monitor::getInstance()->reportRecv(tCurTime, iRet);
            }
        }

        handleProcess();
        checkConnect();
        return 0;
    }

    int Transport::handleError()
    {
        close();
        connect();
        Monitor::getInstance()->report(BM_SOCK_ERROR);
        return 0;
    }

    int Transport::handleProcess()
    {
        int iRecvLen = 0;
        int64_t tCurTime = TNOWMS;
        while ((iRecvLen = _proto->input(_recvBuffer.c_str(), _recvBuffer.length())) > 0)
        {
            int reqIdx = -1;
            int ret = _proto->decode(_recvBuffer.c_str(), iRecvLen, reqIdx);
            auto it = _proto->isSupportSeq() ? _sendQueue.find(reqIdx) : _sendQueue.begin();
            if (it != _sendQueue.end())
            {
                Monitor::getInstance()->report(ret, (tCurTime - it->second));
                _sendQueue.erase(it);
            }

            _recvBuffer.erase(_recvBuffer.begin(), _recvBuffer.begin() + iRecvLen);
        }

        checkTimeOut(tCurTime);
        return 0;
    }

    bool Transport::checkTimeOut(int64_t tCurTime)
    {
        if (!isValid())
        {
            return false;
        }

        for(auto it = _sendQueue.begin(); it != _sendQueue.end(); )
        {
            if ((tCurTime - it->second) > _ep.getTimeout())
            {
                _sendQueue.erase(it++);
                Monitor::getInstance()->report(BM_SOCK_RECV_TIMEOUT, _ep.getTimeout());
            }
            else
            {
                ++it;
            }
        }
        return true;
    }

    int Transport::trySend(int uniqId)
    {
        if (!isValid())
        {
            return BM_SOCK_INVALID;
        }

        int64_t tCurTime = TNOWMS;
        if (_sendQueue.size() && !_proto->isSupportSeq())
        {
            return BM_SEQUENCE;
        }

        int reqIdx = uniqId;
        int bufLen = MAX_SENDBUF_SIZE;
        static char buf[MAX_SENDBUF_SIZE];
        int iRet = _proto->encode(buf, bufLen, reqIdx);
        if (iRet != 0)
        {
            Monitor::getInstance()->report(iRet);
            return BM_PACKET_ENCODE;
        }

        // 启动一次发送
        Monitor::getInstance()->reportSend(tCurTime, bufLen);
        _sendBuffer.append(buf, bufLen);
        _sendQueue[reqIdx] = tCurTime;
        handleWrite();
        return 0;
    }

    void Transport::close()
    {
        if (_sock != -1)
        {
            ::close(_sock);
            _sock = -1;
        }

        _sendQueue.clear();
        _sendBuffer.clear();
        _recvBuffer.clear();
        _connStatus = eUnconnected;
        _loop->delConn(this, EPOLLIN|EPOLLOUT);
    }

    bool Transport::parseHost(const string& sHost, struct in_addr& toAddr)
    {
        int iRet = inet_pton(AF_INET, sHost.c_str(), &toAddr);
        if (iRet > 0)
        {
            return true;
        }
        else if(iRet == 0)
        {
            char buf[2048] = "\0";
            struct hostent stHostent;
            struct hostent *pHostent;
            gethostbyname_r(sHost.c_str(), &stHostent, buf, sizeof(buf), &pHostent, &iRet);
            if (pHostent != NULL)
            {
                toAddr = *(struct in_addr *) pHostent->h_addr;
                return true;
            }
        }
        return false;
    }

    bool Transport::connect()
    {
        int ret = 0;
        if (_ep.getHost().empty())
        {
            return false;
        }

        // 重新打开SOCKET
        _sock = socket(AF_INET, _ep.isTcp() ? SOCK_STREAM : SOCK_DGRAM, 0);
        if (_sock < 0)
        {
            _sock = -1;
            return false;
        }

        // 设置成非阻塞模式
        int flags = 1;
        ioctl(_sock, FIONBIO, &flags);
        _loop->addConn(this, EPOLLIN|EPOLLOUT);

        // 发起连接操作
        if (_ep.isTcp())
        {
            struct sockaddr to;
            bzero(&to, sizeof(to));
            struct sockaddr_in *p = (struct sockaddr_in *)&to;
            p->sin_family = AF_INET;
            p->sin_port = htons(_ep.getPort());
            parseHost(_ep.getHost(), p->sin_addr);
            ret = ::connect(_sock, &to, sizeof(to));
            if (ret != 0 && ret == -1 && errno != EINPROGRESS)
            {
                close();
                Monitor::getInstance()->report(BM_SOCK_NCONNECTED);
                return false;
            }
        }

        _conTimeOut = TNOWMS + CONNECT_TIMEOUT_MS;
        _connStatus = ret == 0 ? eConnected : eConnecting;
        return true;
    }

    bool Transport::checkConnect()
    {
        struct tcp_info info;
        int len = sizeof(info);
        int ret = ::getsockopt(getfd(), IPPROTO_TCP, TCP_INFO, &info, (socklen_t *)&len);
        if (ret == 0 && info.tcpi_state == TCP_ESTABLISHED)
        {
            _connStatus = eConnected;
            return true;
        }
        return false;
    }

    bool Transport::isValid()
    {
        if (eConnected == _connStatus)
        {
            return true;
        }

        if (eConnecting == _connStatus && !checkConnect())
        {
            if (TNOWMS < _conTimeOut)
            {
                return true;
            }
            Monitor::getInstance()->report(BM_SOCK_CONN_TIMEOUT);
        }

        // 重新连接
        close();
        return connect();
    }
};
