/**************************************************************************
* Copyright (c) 2012-2022, Tencent Tech. Co., Ltd. All rights reserved.
*
* File name:          CommDefs.h
* Author:             linfengchen
* Version:            1.0
* Date:               2016.11.10
* Description:        通用头文件处理
* History:
*                     2016.11.10             首次创建
**************************************************************************/
#ifndef _COMMDEFS_H_
#define _COMMDEFS_H_

#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
//#include <linux/tcp.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <cassert>
#include <cstdio>
#include <map>
#include <list>
#include <deque>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <unordered_map>
#include "licote.h"
using namespace std;

#define CAS(ptr, old, new)({ char ret; __asm__ __volatile__("lock; cmpxchgl %2,%0; setz %1": "+m"(*ptr), "=q"(ret): "r"(new),"a"(old): "memory"); ret;})
#define WMB() __asm__ __volatile__("sfence":::"memory")
#define RMB() __asm__ __volatile__("lfence":::"memory")

enum ErrorCode
{
    BM_SUCC = 0,
    BM_UNEXPECT           = 9000,
    BM_SEQUENCE           = 9001,
    BM_INIT_PARAM         = -101,

    BM_PACKET_ERROR       = -1000,
    BM_PACKET_ENCODE      = -1001,
    BM_PACKET_DECODE      = -1002,
    BM_PACKET_PARAM       = -1003,
    BM_PACKET_OVERFLOW    = -1004,

    BM_SOCK_ERROR         = -2000,
    BM_SOCK_INVALID       = -2001,
    BM_SOCK_NCONNECTED    = -2002,
    BM_SOCK_CONN_ERROR    = -2003,
    BM_SOCK_CONN_TIMEOUT  = -2004,
    BM_SOCK_SEND_ERROR    = -2005,
    BM_SOCK_RECV_ERROR    = -2006,
    BM_SOCK_RECV_TIMEOUT  = -2007,

    BM_SHM_ERR_GET        = -3000,
    BM_SHM_ERR_ATTACH,
    BM_SHM_ERR_INIT,
    BM_SHM_ERR_CLEAR,
    BM_SHM_ERR_LOCK,
    BM_SHM_ERR_UNLOCK,
};

enum ConnectStatus
{
    eUnconnected,
    eConnecting,
    eConnected,
};

enum ConnectBuffer
{
    MAX_RECVBUF_SIZE = 64*1024,
    MAX_SENDBUF_SIZE = 4*1024*1024,
};

enum
{
    STA_FLAG = 0x5,
    END_FLAG = 0xa,
    MAX_STEP_COST = 10,
};

/**
 * @brief  去掉头部以及尾部的字符或字符串
 *
 * @param sStr  原始字符
 * @param s     去掉的字符
 *
 * @return string
 */
inline string trim(const string &sStr, const string &s = " \r\n\t")
{
    if (sStr.empty())
    {
        return sStr;
    }

    string::size_type pos = sStr.length();
    while (pos != 0)
    {
        if (s.find_first_of(sStr[pos - 1]) == string::npos)
        {
            break;
        }
        pos--;
    }

    if (pos == sStr.length())
    {
        return sStr;
    }
    return sStr.substr(0, pos);
}

/**
* @brief  解析字符串,用分隔符号分隔,保存在vector里
*
* @param sStr      输入字符串
* @param sSep      分隔字符串(每个字符都算为分隔符)
* @return          解析后的字符vector
*/
inline vector<string> sepstr(const string &sStr, const string &sSep)
{
    vector<string> vt;
    string::size_type pos = 0;
    string::size_type pos1 = 0;

    while(true)
    {
        string s;
        pos1 = sStr.find_first_of(sSep, pos);
        if(pos1 == string::npos)
        {
            if(pos + 1 <= sStr.length())
            {
                s = sStr.substr(pos);
            }
        }
        else if(pos1 == pos)
        {
            s = "";
        }
        else
        {
            s = sStr.substr(pos, pos1 - pos);
            pos = pos1;
        }

        if(!s.empty())
        {
            vt.push_back(s);
        }

        if(pos1 == string::npos)
        {
            break;
        }
        pos++;
    }

    return vt;
}

/**
 * @brief  string转换map
 *
 * @param sStr  业务数据实例
 *
 * @return map<string, string>
 */
inline map<int, int> str2map(const string &sStr)
{
    int flag = 0;
    map<int, int> mapVal;
    char ch1 = '=', ch2 = ';';
    string sName, sValue, sBuffer;
    for (string::size_type pos = 0; pos < sStr.length(); pos++)
    {
        //中间分隔符,前面读入是name
        if (sStr[pos] == ch1)
        {
            flag = 1;
            sName = sBuffer;
            sBuffer = "";
        }
        else if (sStr[pos] == ch2 || (pos + 1) == sStr.length())  //结束符,读入的是值
        {
            sValue  = sBuffer;
            sBuffer = "";
            if ((pos + 1) == sStr.length())
            {
                sValue += sStr[pos];
            }

            if (sName.length() > 0 && flag)
            {
                mapVal[atoi(sName.c_str())] = atoi(sValue.c_str());
                flag = 0;
            }
        }
        else
        {
            sBuffer += sStr[pos];
        }
    }
    return mapVal;
}

/**
 * @brief  时间转换成字符串
 *
 * @param tm  时间戳
 *
 * @return char
 */
inline char* tm2str(time_t tm)
{
    struct tm tmm;
    static char stm[1024];
    memset(&tmm, 0, sizeof(tmm) );
    localtime_r((time_t *)&tm, &tmm);
    snprintf(stm, sizeof(stm), "[%04d-%02d-%02d %02d:%02d:%02d]",
        tmm.tm_year + 1900, tmm.tm_mon + 1, tmm.tm_mday,
        tmm.tm_hour, tmm.tm_min, tmm.tm_sec);
    return stm;
}

/**
 * @brief  map转换string
 *
 * @param mMap 业务数据实例
 *
 * @return string
 */
inline string map2str(const map<int, int>& mMap)
{
    ostringstream os;
    for (map<int, int>::const_iterator itm = mMap.begin(); itm != mMap.end(); itm++)
    {
        os << itm->first << "=" << itm->second << ";";
    }
    return trim(os.str(), ";");
}

/**
 * @brief T型转换成字符串
 *
 * @param @param t 要转换的数据
 *
 * @return 转换后的字符串
 */
template<typename T>
inline string tostr(const T &t)
{
    ostringstream sBuffer;
    sBuffer << t;
    return sBuffer.str();
}

/**
 * @brief 函数注释
 *
 * @param sFullFileName 文件名称
 *
 * @return int
 */
inline string loadfile(const string &sFullFileName)
{
    ifstream ifs(sFullFileName.c_str());
    return string(istreambuf_iterator<char>(ifs), istreambuf_iterator<char>());
}

/**
 * @brief  获取当前时间
 *
 * @return int64_t
 */
inline int64_t getNow(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * (int64_t)1000000 + tv.tv_usec;
}

/**
 * @brief 获取当前CPU数量
 *
 * @return int
 */
inline int64_t getProcNum(void)
{
    static int procNum = 0;
    if (procNum == 0)
    {
        procNum = sysconf(_SC_NPROCESSORS_ONLN);
    }
    return procNum;
}

// Percentile finds the relative standing in a slice of floats
inline double percentile(const vector<double>& input, double percent)
{
    size_t len = input.size();
    if (len == 0)
    {
        return -1;
    }

    if (percent <= 0 || percent > 100)
    {
        return -2;
    }

    // Multiply percent by length of input
    double index = (percent / 100) * (double)len;

    // Check if the index is a whole number
    if (index == (double)((int64_t)index)) {
        return input[(int)index - 1];
    } else if (index > 1) {
        // Find the average of the index and following values
        return (input[(int)index-1] + input[(int)index]) / 2;
    } else {
        return -3;
    }
}

#define MAX_FD  50001
#define TNOWMS  (getNow() / 1000)
#define LICODE_GETINT(x, v) (licote_option_exist(x) ? atoi(licote_option_get(x)) : v)
#define LICODE_GETSTR(x, v) (licote_option_exist(x) ? trim(licote_option_get(x)) : v)
#endif // _COMMDEFS_H_

