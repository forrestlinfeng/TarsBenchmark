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
        virtual int initialize(const vector<string>& params) { return 0; }

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
    protected:
        /**
         * @brief  ����ȡ�������
         *
         * @param rmin  ��Сֵ
         * @param rmax  ���ֵ
         *
         * @return long ���ֵ
         */
        virtual long genRandomValue(const string& rmin, const string& rmax)
        {
            long max = TC_Common::strto<long>(rmax);
            long min = TC_Common::strto<long>(rmin);
            return (long)(rand() % (max - min + 1) + min);
        }

        /**
         * @brief  ����ȡ�������
         *
         * @param v     string����
         * @param is_int    �Ƿ���������
         *
         * @return string ���ֵ����
         */
        virtual string genRandomValue(const string& v, bool is_int = true)
        {
            string::size_type l = v.find_first_of('[');
            string::size_type r = v.find_last_of(']');
            string::size_type m = v.find_first_of('-');
            string::size_type n = v.find_first_of(',');
            if (l != 0 || r != (v.size() - 1) || (m == string::npos && n == string::npos))
            {
                return v;
            }

            string nv = v.substr(l + 1, r - l);
            if (m != string::npos && is_int)
            {
                vector<string> vs = TC_Common::sepstr<string>(nv, "-");
                if (vs.size() == 2)
                {
                    _random_flag = true;
                    return TC_Common::tostr(genRandomValue(vs.at(0), vs.at(1)));
                }
                throw runtime_error("invalid randval(-)");
            }
            else if (n != string::npos)
            {
                vector<string> vs = TC_Common::sepstr<string>(nv, ",");
                if (vs.size() > 1)
                {
                    _random_flag = true;
                    return vs[(size_t)rand() % vs.size()];
                }
            }
            return nv;
        }
    protected:
        bool _random_flag;
    };
};
#endif
