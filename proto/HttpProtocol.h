/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          httpProtocol.h
* Author:             linfengchen    
* Version:            1.0  
* Date:               2016.11.23
* Description:        HTTP协议的实现
* History:    
*                     2016.11.23             首次创建
**************************************************************************/
#ifndef _HTTP_PROTOCOL_H_
#define _HTTP_PROTOCOL_H_

#include "Protocol.h"

namespace bm
{
    class httpProtocol : public Protocol
    {
        DECLARE_PROTODYNCREATE(httpProtocol)
    public:
        httpProtocol() {}
        virtual ~httpProtocol() {}

        /** 
         * @brief  静态初始化函数
         * 
         * @param argc 参数个数
         * @param argv 参数内容
         * 
         * @return 0成功, 其他失败
         */
        virtual int initialize(int argc, char** argv);

        /** 
         * @brief  判断收包是否完整 
         * 
         * @param buf  数据包指针
         * @param len  网络数据包长度
         * 
         * @return int
         */
        virtual int input(const char *buf, size_t len);

        /** 
         * @brief  HTTP协议打包
         * 
         * @param req  业务数据实例
         * @param len  网络数据包长度
         * @param uniqId  全局唯一ID
         * 
         * @return 0成功, <0: 其他失败 >0: 需要的buffer长度 
         */
        virtual int encode(char *buf, int& len, int& uniqId);

        /** 
         * @brief  HTTP协议解包
         * 
         * @param req  业务数据实例
         * @param len  网络数据包长度
         * @param uniqId  全局唯一ID
         * 
         * @return 0成功, 其他失败
         */
        virtual int decode(const char *buf, int len, int& uniqId);
    private:
        string  _reqBuff;
    };
};
#endif
