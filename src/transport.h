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
#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include "monitor.h"
#include "commdefs.h"
#include "protocol.h"
#include "proto_factory.h"

namespace bm
{
    class Transport : TC_ClientSocket
    {
    public:
        Transport(const TC_Endpoint& ep, TC_Epoller* loop) : _ep(ep), _loop(loop)
        {
            this->close();
            this->init(ep.getHost(), ep.getPort(), ep.getTimeout());
        }

        virtual ~Transport()
        {
            _factory.destroyObject();
            close();
        }

        /**
         * @brief  ��ʼ������
         * @param proto     Э������
         * @param argc      ��������
         * @param argv      ��������
         *
         */
        virtual void initialize(const string& proto, int argc, char** argv)
        {
            _proto = _factory.get(proto, argc, argv);
        }

        /**
         * @brief  �ر���Դ
         *
         */
        virtual void close();

        /**
         * @brief  �¼�����
         *
         * @return int
         */
        virtual int handleRead();
        virtual int handleError();
        virtual int handleWrite();
        virtual int handleProcess();

        /**
         * @brief  ���Socket����
         *
         * @return int
         */
        virtual int checkSocket() = 0;

        /**
         * @brief  ��齨���Ƿ�ɹ�
         *
         * @return bool
         */
        virtual bool checkConnect() { return true; }

        /**
         * @brief  ����Ƿ��Ѿ���ʱ
         *
         * @return bool
         */
        virtual bool checkTimeOut(int64_t tCurTime);

        /**
         * @brief  ��ȡ���/socket
         *
         * @return int
         */
        virtual TC_Socket *getSocket() { return TC_ClientSocket::getSocket(); }
        virtual int getfd() { return this->getSocket()->getfd(); }

        /**
         * @brief  ���Է���
         * @param uniqId ҵ������ʵ��
         *
         * @return int
         */
        int trySend(int uniqId);

        /**
         * @brief  ����epollʱ��
         *
         */
        static void handle(TC_Epoller* loop, int time);
    protected:
        TC_Endpoint             _ep;
        TC_Epoller*             _loop;
        Protocol*               _proto;
        ProtoFactory            _factory;

        string                  _sendBuffer;
        string                  _recvBuffer;
        int64_t                 _conTimeOut;
        ConnectStatus           _connStatus;
        unordered_map<int, int64_t> _sendQueue;
    };

    class TCPTransport : public Transport
    {
    public:
        TCPTransport(const TC_Endpoint& ep, TC_Epoller* loop): Transport(ep, loop)
        {

        }

        /**
         * @brief  ���Socket����
         *
         * @return bool
         */

        virtual int checkSocket();
        virtual bool checkConnect();

        /**
         * ���ͺ���
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         * @param flag ��־λ
         *
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int send(const char *buf, size_t len)
        {
            if (getfd() == -1) return BM_SOCK_INVALID;
            if (eConnected != _connStatus) return BM_SOCK_CONN_ERROR;

            int ret = getSocket()->send(buf, len);
            if (ret < 0 && errno != EAGAIN)
            {
                this->close();
                return BM_SOCK_SEND_ERROR;
            }
            return ret < 0 ? 0 : ret;
        }

        /**
         * ���մ�����
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         *
         * @return 0�ɹ�, ����ʧ��
         */
        int recv(char *buf, size_t& len)
        {
            if (getfd() == -1)
            {
                return BM_SOCK_INVALID;
            }

            int rcvLen = this->getSocket()->recv((void*)buf, len, 0);
            if (rcvLen < 0 && errno != EAGAIN)
            {
                close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_RECV_ERROR;
            }

            len = rcvLen <= 0 ? 0 : rcvLen;
            return BM_SUCC;
        }
    };

    class UDPTransport : public Transport
    {
    public:
        UDPTransport(const TC_Endpoint& ep, TC_Epoller* loop) : Transport(ep, loop)
        {

        }

        /**
         * @brief  ���Socket����
         *
         * @return bool
         */
        int checkSocket();

        /**
         * ���ͺ���
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         *
         * @return 0�ɹ�, ����ʧ��
         */
        int send(const char *buf, size_t len)
        {
            if (getfd() < 0) return BM_SOCK_INVALID;

            int ret = this->getSocket()->sendto(buf, len, _ep.getHost(), _ep.getPort(), 0);
            if (ret < 0 && errno != EAGAIN)
            {
                close();
                return BM_SOCK_SEND_ERROR;
            }
            return ret <= 0 ? 0 : (int)len;
        }

        /**
         * ���մ�����
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         * @param flag ��־λ
         *
         * @return 0�ɹ�, ����ʧ��
         */
        int recv(char *buf, size_t& len)
        {
            if (getfd() < 0) return BM_SOCK_INVALID;

            string tmpIp;
            uint16_t tmpPort;
            int rcvLen = this->getSocket()->recvfrom(buf, len, tmpIp, tmpPort);
            if (rcvLen < 0 && errno != EAGAIN)
            {
                this->close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_CONN_ERROR;
            }

            len = rcvLen <= 0 ? 0 : rcvLen;
            return BM_SUCC;
        }
    };
};
#endif
