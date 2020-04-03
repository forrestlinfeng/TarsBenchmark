/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          Protocol.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.11
* Description:        交互协议基类
* History:    
*                     2016.11.11             首次创建
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
         * @brief  初始化函数
         * 
         * @param argc  参数
         * @param argv  参数内容
         * 
         * @return 0成功, 其他失败
         */
        virtual int initialize(int argc, char** argv) = 0;

        /** 
         * @brief  协议是否支持有序的
         * 
         * @return 0成功, 其他失败
         */
        virtual int isSupportSeq() { return false; }

        /** 
         * 验证数据包的完整性
         * 
         * @param data 数据包指针
         * @param len  网络数据包长度
         * 
         * @return 参考PACKET_XXXX;
         */
        virtual int input(const char *buf, size_t len) = 0;
    
        /** 
         * 解析/编码协议, 获取网络数据包的实例
         * 
         * @param req  业务数据实例
         * @param len  网络数据包长度
         * @param uniqId  全局唯一ID
         * 
         * @return 0成功, 其他失败
         */
        virtual int encode(char *buf, int& len, int& uniqId) = 0;
        virtual int decode(const char *buf, int len, int& uniqId) = 0;
    };
};
#endif
