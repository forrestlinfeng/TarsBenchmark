#include "ProxyImp.h"
#include "ProxyServer.h"
#include "tars_protocol.h"
#include "servant/Application.h"

using namespace std;
extern ProxyServer g_app;

#define DELETE_POINT(p) if (p != NULL) { delete p; p = NULL;}

//////////////////////////////////////////////////////
void ProxyImp::initialize()
{
}

//////////////////////////////////////////////////////
void ProxyImp::destroy()
{
    //destroy servant here:
    //...
}

int ProxyImp::startup(const BenchmarkUnit& req, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TaskStat task;
    if (req.servant.empty() || req.rpcfunc.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    if ((req.speed % req.links != 0) || (req.speed / req.links) > 1000)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_CASEMATCH, err_code, 0, err_msg, "link and speed not match")
    }

    task.conf.paralist  = TC_Common::sepstr<string>(req.para_input, "|");
    task.conf.paravals  = TC_Common::sepstr<string>(req.para_value, "<br>");
    if (task.conf.paralist.size() != task.conf.paravals.size())
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_CASEMATCH, err_code, 0, err_msg, "case para not match val")
    }

    bool has_bad_endpoint = false;
    for (size_t i = 0; i < req.endpoints.size(); i++)
    {
        try {
            TC_Endpoint ep;
            ep.parse(req.endpoints[i]);
            if (ep.getPort() == 0 || ep.getPort() > 65535)
            {
                has_bad_endpoint = true;
            }
        }
        catch (TC_Exception& e)
        {
            TLOGTARS("parse endpoint fail:" << e.what() << ", " << req.endpoints[i] << endl);
            has_bad_endpoint = true;
        }
    }

    if (has_bad_endpoint || req.endpoints.size() == 0)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_ENDPOINT, err_code, req.endpoints.size(), err_msg, "err endpoint")
    }

    BenchmarkSummary summary;
    g_app.getSummary(summary);
    
    int64_t totol_left_speed = 0;
    int64_t need_speed = req.speed * req.endpoints.size();
    for (auto &node : summary.nodes)
    {
        totol_left_speed += node.second.left_speed;
    }

    if (need_speed < 0 || totol_left_speed < need_speed)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_RESOURCE, err_code, totol_left_speed, err_msg, "need more nodes")
    }

    // 处理入参配置
    task.state = TS_IDLE;
    task.conf.speed   = req.speed;
    task.conf.links   = req.links;
    task.conf.servant = req.servant;
    task.conf.rpcfunc = req.rpcfunc;
    task.conf.endpoints = req.endpoints;
    g_app.updateTask(req.servant + "." + req.rpcfunc, task);

    PROC_TRY_END(err_msg, ret_code, BM_ERR_PARAM, BM_ERR_PARAM)

    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(req) << "|" << curr->getIp() << endl;

    return ret_code;
}

int ProxyImp::query(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (req.servant.empty() || req.rpcfunc.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    string main_key = req.servant + "." + req.rpcfunc;
    if (!g_app.getResult(main_key, stat))
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "not find interface")
    }
    
    PROC_TRY_END(err_msg, ret_code, BM_ERR_PARAM, BM_ERR_PARAM)
    
    if (ret_code != 0)
    {
        FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << logTars(req) << "|" << curr->getIp() << endl;
    }
    else
    {
        TLOGDEBUG((TNOWMS - f_start) << "|" << "|req:" << logTars(req) << "|" << logTars(stat) << "|" << curr->getIp() << endl << endl);
    }
    return ret_code;
}

int ProxyImp::shutdown(const BenchmarkUnit& req, ResultStat& stat, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (req.servant.empty() || req.rpcfunc.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_ERR_PARAM, err_code, 0, err_msg, "check param")
    }

    BenchmarkSummary summary;
    g_app.getSummary(summary);
    string main_key = req.servant + "." + req.rpcfunc;
    if (summary.task.find(main_key) == summary.task.end() || 
        summary.task[main_key].state != TS_RUNNING)
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_RUNNING, err_code, 0, err_msg, "task not running")
    }
    
    // 移交线程去执行关闭策略
    g_app.getResult(main_key, stat);
    summary.task[main_key].state = TS_FINISHED;
    g_app.updateTask(main_key, summary.task[main_key]);

    PROC_TRY_END(err_msg, ret_code, BM_ERR_PARAM, BM_ERR_PARAM)
    
    FDLOG("info") << __FUNCTION__ << "|" << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|req:" << logTars(req) << "|" << logTars(stat) << "|" << curr->getIp() << endl;

    return ret_code;
}

int ProxyImp::test(const BenchmarkUnit& req, string& rsp, string& errmsg, TarsCurrentPtr curr)
{
    int ret_code = 0;
    int err_code = 0;
    char *sendbuf = NULL;
    char *recvbuf = NULL;

    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    TaskStat task;
    if (req.servant.empty() || req.rpcfunc.empty())
    {
        PROC_TRY_EXIT(ret_code, BM_ERR_PARAM, err_code, 0, errmsg, "check param")
    }

    tarsProtocol proto;
    proto._servant  = req.servant;
    proto._function = req.rpcfunc;
    proto._paraList = TC_Common::sepstr<string>(req.para_input, "|");
    proto._paraVals = TC_Common::sepstr<string>(req.para_value, "<br>");
    vector<string> paraOut = TC_Common::sepstr<string>(req.para_output, "<br>");
    if (proto._paraList.size() != proto._paraVals.size())
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_CASEMATCH, err_code, 0, errmsg, "case para not match val")
    }

    TC_Endpoint ep;
    if (req.endpoints.size() == 0)
    {
        PROC_TRY_EXIT(ret_code, BM_NODE_ERR_ENDPOINT, err_code, req.endpoints.size(), errmsg, "err endpoint")
    }

    ep.parse(req.endpoints[0]);
    proto._timeOut  = ep.getTimeout();

    int seq = 1;
    int sendlen = 4 * 1024 * 1024;
    size_t recvlen = 8 * 1024 * 1024;
    sendbuf = new char[sendlen];
    recvbuf = new char[recvlen];
    int ret = proto.encode(sendbuf, sendlen, seq);
    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_ENCODE, err_code, ret, errmsg, "encode fail")
    }
    
    if (ep.isTcp())
    {
        TC_TCPClient client(ep.getHost(), ep.getPort(), ep.getTimeout());
        ret = client.sendRecv(sendbuf, (size_t)sendlen, recvbuf, recvlen);
    }
    else
    {
        TC_TCPClient client(ep.getHost(), ep.getPort(), ep.getTimeout());
        ret = client.sendRecv(sendbuf, (size_t)sendlen, recvbuf, recvlen);
    }

    if (ret != 0)
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_SOCKET, err_code, ret, errmsg, "socket sendrecv fail")
    }


    TarsInputStream<BufferReader> is, isf;
    is.setBuffer(recvbuf + 4, recvlen - 4);

    ResponsePacket res;
    res.readFrom(is);
    isf.setBuffer(res.sBuffer);
    ret_code = res.iRet;
    for (size_t i = 0; i < paraOut.size(); i++)
    {
        rsp += proto.decode(isf, paraOut[i], i+1, false);
    }

    PROC_TRY_END(errmsg, ret_code, BM_PROXY_ERR_DECODE, BM_PROXY_ERR_DECODE)
    DELETE_POINT(sendbuf)
    DELETE_POINT(recvbuf)
    
    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << errmsg << "|" << "|req:" << logTars(req) << "|out:" << rsp << "|" << curr->getIp() << endl;

    return ret_code;
}