/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          Transport.h
* Author:             linfengchen
* Version:            1.0
* Date:               2016.11.8
* Description:        ���紫��ʵ��
* History:
*                     2016.11.8
**************************************************************************/
#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#include "Monitor.h"
#include "CommDefs.h"
#include "Protocol.h"
#include "EventLoop.h"
#include "ProtoFactory.h"

namespace bm
{
    class Endpoint
    {
    public:
        /**
         * @brief ���ַ�������������
         * @param desc
         */
        Endpoint() {};

        /**
         * @brief ���캯��
         * @param host
         * @param port
         * @param timeout, ��ʱʱ��, ����
         */
        Endpoint(const string& host, int port, int timeout, int istcp = true)
        {
            _host    = host;
            _port    = port;
            _timeout = timeout;
            _istcp   = istcp;
        }

        /**
         * @brief ��������
         * @param l
         */
        Endpoint(const Endpoint& l)
        {
            _host   = l._host;
            _port   = l._port;
            _timeout= l._timeout;
            _istcp  = l._istcp;
        }

        /**
         * @brief ��ֵ����
         * @param l
         *
         * @return Endpoint&
         */
        Endpoint& operator = (const Endpoint& l)
        {
            if(this != &l)
            {
                _host   = l._host;
                _port   = l._port;
                _timeout= l._timeout;
                _istcp  = l._istcp;
            }
            return *this;
        }

        /**
         * ==
         * @param l
         *
         * @return bool
         */
        bool operator == (const Endpoint& l)
        {
            return (_host == l._host && _port == l._port && _timeout == l._timeout && _istcp == l._istcp);
        }

        /**
         * @brief ����ip
         *
         * @param str
         */
        void setHost(const string& host)    { _host = host; }

        /**
         * @brief ��ȡip
         *
         * @return const string&
         */
        string getHost() const              { return _host; }

        /**
         * @brief ���ö˿�
         * @param port
         */
        void setPort(int port)              { _port = port; }

        /**
         * @brief ��ȡ�˿�
         *
         * @return int
         */
        int getPort() const                 { return _port; }

        /**
         * @brief ���ó�ʱʱ��
         * @param timeout
         */
        void setTimeout(int timeout)        { _timeout = timeout; }

        /**
         * @brief ��ȡ��ʱʱ��
         *
         * @return int
         */
        int getTimeout() const              { return _timeout; }

        /**
         * @brief  �Ƿ���TCP, ������ΪUDP
         *
         * @return bool
         */
        bool isTcp() const                  { return _istcp; }

        /**
         * @brief ����ΪTCP��UDP
         * @param bTcp
         */
        void setTcp(bool bTcp)              { _istcp = bTcp; }

        /**
         * @brief �ַ�������
         *
         * @return string
         */
        string toString()
        {
            ostringstream os;
            os << (isTcp()?"tcp" : "udp") << " -h " << _host << " -p " << _port << " -t " << _timeout;
            return os.str();
        }
    protected:
        string      _host;
        int         _port;
        int         _timeout;
        int         _istcp;
    };

    typedef struct tagMsgItem
    {
        int         uniqId;
        int64_t     sendTime;
        char*       bufPtr;
        int         bufLen;
    } MsgItem;

    inline bool operator==(const MsgItem&l, const MsgItem&r)
    {
        return (l.uniqId == r.uniqId);
    }
    inline bool operator!=(const MsgItem&l, const MsgItem&r)
    {
        return (l.uniqId != r.uniqId);
    }
    inline bool operator<(const MsgItem&l, const MsgItem&r)
    {
        return (l.uniqId < r.uniqId);
    }
    inline bool operator<=(const MsgItem&l, const MsgItem&r)
    {
        return !(r < l);
    }
    inline bool operator>(const MsgItem&l, const MsgItem&r)
    {
        return r < l;
    }
    inline bool operator>=(const MsgItem&l, const MsgItem&r)
    {
        return !(l < r);
    }

    class Transport
    {
    public:
        Transport(const Endpoint& ep, EPollLoop* loop) : _ep(ep), _loop(loop), _sock(-1)
        {
            close();
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
         * @brief  ���紦������
         *
         * @param buf   ����ָ��
         * @param len   ���ݳ���
         * @param flag  ��־λ
         *
         * @return int
         */
        virtual int send(const char *buf, uint32_t len, uint32_t flag) = 0;
        virtual int recv(char *buf, uint32_t len, uint32_t flag) = 0;

        /**
         * @brief  EPOLLЭ�鴦��
         *
         * @return int
         */
        virtual int handleRead();
        virtual int handleError();
        virtual int handleWrite();
        virtual int handleProcess();

        /**
         * @brief  �ڲ�socket����
         *
         * @return bool
         */
        void close();
        bool connect();
        bool isValid();
        bool checkConnect();
        bool checkTimeOut(int64_t tCurTime);
        bool parseHost(const string& sHost, struct in_addr& toAddr);

        /**
         * @brief  ���Է���
         * @param uniqId ҵ������ʵ��
         *
         * @return int
         */
        int trySend(int uniqId);

        /**
         * @brief  ��ȡ���
         *
         * @return int
         */
        int getfd() { return _sock; }
    protected:
        Endpoint                _ep;
        EPollLoop*              _loop;
        Protocol*               _proto;
        ProtoFactory            _factory;

        int                     _sock;
        string                  _sendBuffer;
        string                  _recvBuffer;
        int64_t                 _conTimeOut;
        ConnectStatus           _connStatus;
        unordered_map<int, int64_t> _sendQueue;
    };

    class TCPTransport : public Transport
    {
    public:
        TCPTransport(const Endpoint& ep, EPollLoop* loop): Transport(ep, loop)
        {

        }

        /**
         * ���ͺ���
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         * @param flag ��־λ
         *
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int send(const char *buf, uint32_t len, uint32_t flag)
        {
            if (getfd() == -1) return BM_SOCK_INVALID;
            if (eConnected != _connStatus) return BM_SOCK_CONN_ERROR;

            int iRet = ::send(getfd(), buf, len, flag);
            if (iRet < 0 && errno != EAGAIN)
            {
                close();
                return BM_SOCK_SEND_ERROR;
            }
            return iRet < 0 ? 0 : iRet;
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
        int recv(char *buf, uint32_t len, uint32_t flag)
        {
            if (eConnected != _connStatus || getfd() == -1)
            {
                Monitor::getInstance()->report(BM_SOCK_CONN_ERROR);
                return BM_SOCK_CONN_ERROR;
            }

            int iRet = ::recv(getfd(), buf, len, flag);
            if (iRet < 0 && errno != EAGAIN)
            {
                close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_RECV_ERROR;
            }
            else if (iRet == 0) // ����˶Ͽ�����
            {
                _connStatus = eUnconnected;
            }
            return iRet;
        }
    };

    class UDPTransport : public Transport
    {
    public:
        UDPTransport(const Endpoint& ep, EPollLoop* loop): Transport(ep, loop)
        {

        }

        /**
         * ���ͺ���
         *
         * @param buf  ����ָ��
         * @param len  ���ݳ���
         * @param flag ��־λ
         *
         * @return 0�ɹ�, ����ʧ��
         */
        int send(const char *buf, uint32_t len, uint32_t flag)
        {
            if (getfd() == -1) return BM_SOCK_INVALID;

            struct sockaddr to;
            bzero(&to, sizeof(struct sockaddr));
            struct sockaddr_in *p = (struct sockaddr_in *)&to;
            p->sin_family = AF_INET;
            p->sin_port  = htons(_ep.getPort());
            parseHost(_ep.getHost(), p->sin_addr);
            int iRet = ::sendto(getfd(), buf, len, flag, &to, sizeof(to));
            if (iRet < 0)
            {
                if (errno != EAGAIN)
                {
                    close();
                    return BM_SOCK_SEND_ERROR;
                }
                iRet=0;
            }
            else if(iRet>0 && iRet != (int)len)
            {
                iRet = len;
            }

            return iRet;
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
        int recv(char *buf, uint32_t len, uint32_t flag)
        {
            int iRet = ::recvfrom(getfd(), buf, len, flag, NULL, NULL);
            if (iRet < 0  && errno != EAGAIN)
            {
                close();
                Monitor::getInstance()->report(BM_SOCK_RECV_ERROR);
                return BM_SOCK_RECV_ERROR;
            }
            return iRet;
        }
    };
};
#endif
