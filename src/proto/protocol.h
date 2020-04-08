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
#ifndef _PROTOCOL_INCLUDE_
#define _PROTOCOL_INCLUDE_

#include "util/tc_dyn_object.h"

using namespace tars;

namespace bm
{
    class Protocol : public TC_DYN_Object
    {
        // DECLARE_DYNCREATE(Protocol)
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
