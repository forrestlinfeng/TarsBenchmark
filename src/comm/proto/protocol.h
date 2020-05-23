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
         * @brief  ��ʼ������
         *
         * @param params  ����
         *
         * @return 0�ɹ�, ����ʧ��
         */
        virtual int initialize(const vector<string>& params) = 0;

        /**
         * @brief  Э���Ƿ�֧�������
         *
         * @return true: ֧��; false: ��֧��
         */
        virtual int isSupportSeq() { return false; }

        /**
         * @brief  �ж��հ��Ƿ�����
         *
         * @param buf  ���ݰ�ָ��
         * @param len  �������ݰ�����
         *
         * @return int =0���Ĳ�����; >0:ʵ�ʱ��ĳ���; <0:����
         */
        virtual int input(const char *buf, size_t len) = 0;

        /**
         * ����/����Э��, ��ȡ�������ݰ���ʵ��
         *
         * @param buf  ���ݰ�ָ��
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
