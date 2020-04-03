/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          Protocol.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.11
* Description:        ����Э�����
* History:    
*                     2016.11.11             �״δ���
**************************************************************************/
#ifndef _PROTOCOL_INCLUDE_
#define _PROTOCOL_INCLUDE_

#include "DyncObject.h"

namespace bm
{
    class Protocol : public DyncObject
    {
    public:
        virtual ~Protocol() {}

        /** 
         * @brief  ��ʼ������
         * 
         * @param argc  ����
         * @param argv  ��������
         * 
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int initialize(int argc, char** argv) = 0;

        /** 
         * @brief  Э���Ƿ�֧�������
         * 
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int isSupportSeq() { return false; }

        /** 
         * ��֤���ݰ���������
         * 
         * @param data ���ݰ�ָ��
         * @param len  �������ݰ�����
         * 
         * @return �ο�PACKET_XXXX;
         */
        virtual int input(const char *buf, size_t len) = 0;
    
        /** 
         * ����/����Э��, ��ȡ�������ݰ���ʵ��
         * 
         * @param req  ҵ������ʵ��
         * @param len  �������ݰ�����
         * @param uniqId  ȫ��ΨһID
         * 
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int encode(char *buf, int& len, int& uniqId) = 0;
        virtual int decode(const char *buf, int len, int& uniqId) = 0;
    };
};
#endif
