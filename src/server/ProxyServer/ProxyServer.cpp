﻿#include "ProxyServer.h"
#include "ProxyImp.h"

using namespace std;

ProxyServer g_app;

/////////////////////////////////////////////////////////////////
void ProxyServer::initialize()
{
    //initialize application here:
    //...
    addServant<ProxyImp>(ServerConfig::Application + "." + ServerConfig::ServerName + ".ProxyObj");

    // 启动一个watch线程，防止压测线程长时间空跑
    _runflag = true;
    auto fw = std::bind(&ProxyServer::daemon, this);
    _watchdog.init(1);
    _watchdog.start();
    _watchdog.exec(fw);
}
/////////////////////////////////////////////////////////////////
void ProxyServer::destroyApp()
{
    //destroy application here:
    //...

    _runflag = false;
    _watchdog.stop();
    _watchdog.waitForAllDone();
}

inline ResultStat& operator+=(ResultStat& l, ResultStat& r)
{
    uint64_t total_request = l.total_request + r.total_request;
    if (total_request > 0)
    {
        l.p90_time  = (l.p90_time*total_request + r.p90_time*r.total_request) / total_request;
        l.p99_time  = (l.p99_time*total_request + r.p99_time*r.total_request) / total_request;
        l.p999_time = (l.p999_time*total_request + r.p999_time*r.total_request) / total_request;
    }

    for (auto & it : r.ret_map)
    {
        l.ret_map[it.first] += it.second;
    }

    for (auto & it : r.cost_map)
    {
        l.cost_map[it.first] += it.second;
    }

    l.total_request += r.total_request;
    l.succ_request  += r.succ_request;
    l.fail_request  += r.fail_request;
    l.total_time    += r.total_time;
    l.send_bytes    += r.send_bytes;
    l.recv_bytes    += r.recv_bytes;
    l.max_time = std::max(l.max_time, r.max_time);
    l.min_time = std::min(l.min_time, r.min_time);
    return l;
}

/////////////////////////////////////////////////////////////////
void ProxyServer::daemon()
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");

    while (_runflag)
    {
        Int64 f_start = TNOWMS;
        PROC_TRY_BEGIN

        scanActiveNode(f_start);

        // 扫描任务列表
        for (auto &it : _summary.task)
        {
            switch (it.second.state)
            {
                case TS_IDLE: 
                {
                    map<string, int> speed_quota;
                    TaskConf tconf = it.second.conf;
                    Int32 left_speed  = tconf.speed * tconf.endpoints.size();
                    Int32 totol_links = tconf.links * tconf.endpoints.size();
                    Int32 totol_speed = tconf.speed * tconf.endpoints.size();
                    for(auto itt = _summary.nodes.begin(); itt != _summary.nodes.end() && left_speed > 0; itt++)
                    {
                        Int32 cost_speed = std::min(itt->second.left_speed, left_speed);
                        tconf.links = totol_links * cost_speed / totol_speed;
                        tconf.speed = cost_speed / tconf.endpoints.size();
                        if (tconf.speed > 0 && tconf.links > 0)
                        {
                            startupNodeTask(itt->first, tconf);
                            left_speed -= cost_speed;
                            itt->second.left_speed -= cost_speed;
                            speed_quota[itt->first] = cost_speed;
                        }
                    }

                    Lock lock(*this);
                    it.second.state = TS_RUNNING;
                    it.second.start_time = f_start / 1000;
                    it.second.fetch_time = f_start / 1000;
                    _summary.result[it.first].time_stamp = f_start / 1000;
                    _summary.total_result[it.first] .time_stamp = f_start / 1000;
                    break;
                }
                case TS_RUNNING: 
                {
                    // 超过5分钟没有过来采集数据需要关闭
                    if (abs(f_start/1000 - it.second.fetch_time) > 300)
                    {
                        Lock lock(*this);
                        it.second.state = TS_FINISHED;
                    }

                    // 采集任务状态
                    for (auto & item : it.second.speed_quota)
                    {
                        ResultStat stat;
                        int ret = queryNodeTask(item.first, it.second.conf, stat);
                        if (ret == 0)
                        {
                            Lock lock(*this);
                            _summary.result[it.first] += stat;
                            _summary.total_result[it.first] += stat;
                        }
                    }
                    break;
                }
                default: 
                {
                    // 执行关闭逻辑
                    shutdownNodeTask(it.first, it.second.conf);
                    Lock lock(*this);
                    _summary.task.erase(it.first);
                    break;
                }
            }
        }

        PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)
        
        if (ret_code != 0)
        {
            FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|"  << endl;
        }

        if ((TNOWMS - f_start) < 1000)
        {
            usleep(1000 * (TNOWMS - f_start));
        }
    }

    FDLOG(__FUNCTION__) << "thread exit" << endl;
}

void ProxyServer::scanActiveNode(int64_t cur_time)
{
    try
    {
        static int64_t last_time = 0;
        if (abs(cur_time - last_time) > 60000)
        {
            last_time = cur_time;
            map<string, NodePrx> nodeprx;
            map<string, NodeStat> nodestat;
            TC_Config &conf = Application::getConfig();
            string node_obj = conf.get("/benchmark<nodeObj>", "benchmark.NodeServer.NodeObj");
            vector<TC_Endpoint> eps = Application::getCommunicator()->getEndpoint4All(node_obj);
            for (size_t i = 0; i < eps.size(); i++)
            {
                string obj_name = node_obj + "&" + eps[i].toString();
                nodeprx[eps[i].getHost()] = Application::getCommunicator()->stringToProxy<NodePrx>(obj_name);
            }

            for (auto& prx : nodeprx)
            {
                NodeStat stat;
                prx.second->capacity(stat);
                nodestat[prx.first] = stat;
            }

            Lock lock(*this);
            _nodeprx.swap(nodeprx);
            _summary.nodes = nodestat;
        }

        // 清除非法运行的压测实例
        for (auto &it : _summary.nodes)
        {
            for(auto ite = it.second.executors.begin(); ite != it.second.executors.end();)
            {
                string main_key = ite->servant + ":" + ite->rpcfunc;
                if (_summary.task.find(main_key) == _summary.task.end())
                {
                    // 关闭配置
                    TaskConf task;
                    task.servant = ite->servant;
                    task.rpcfunc = ite->rpcfunc;
                    shutdownNodeTask(it.first, task); 

                    Lock lock(*this);
                    ite = it.second.executors.erase(ite);
                    continue;
                }
                ++ite;
            }
        }

    }
    catch (TC_Exception& e)
    {
        FDLOG("error") << "exception:" << e.what() << endl;
        throw runtime_error("taf::TC_Exception");
    }
    catch (...)
    {
        FDLOG("error") << "unknown exception" << endl;
    }
}

////////////////////////////////////////////////////////////////
int ProxyServer::startupNodeTask(const string& ipaddr, const TaskConf& task)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    PROC_NE_EXIT(_nodeprx[ipaddr]->startup(task), 0, ret_code, BM_PROXY_ERR_STARTUP)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|" << logTars(task) << endl;

    return ret_code;
}

////////////////////////////////////////////////////////////////
int ProxyServer::shutdownNodeTask(const string& ipaddr, const TaskConf& task)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    QueryRsp rsp;
    PROC_NE_EXIT(_nodeprx[ipaddr]->shutdown(task, rsp), 0, ret_code, BM_PROXY_ERR_SHUTDOWN)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)

    FDLOG(__FUNCTION__) << (TNOWMS - f_start) << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|" << logTars(task) << endl;

    return ret_code;
}

////////////////////////////////////////////////////////////////
int ProxyServer::queryNodeTask(const string& ipaddr, const TaskConf& task, ResultStat& stat)
{
    int ret_code = 0;
    int err_code = 0;
    string err_msg("");
    QueryRsp rsp;
    
    Int64 f_start = TNOWMS;
    PROC_TRY_BEGIN

    if (_nodeprx.find(ipaddr) == _nodeprx.end())
    {
        PROC_TRY_EXIT(ret_code, BM_PROXY_ERR_NOTFIND, err_code, 0, err_msg, "prx not find")
    }

    PROC_NE_EXIT(_nodeprx[ipaddr]->query(task, rsp), 0, ret_code, BM_PROXY_ERR_SHUTDOWN)
    PROC_TRY_END(err_msg, ret_code, BM_EXCEPTION, BM_EXCEPTION)
    stat = rsp.stat;

    TLOGDEBUG((TNOWMS - f_start)  << "|" << ret_code << "|" << err_code << "|" << err_msg << "|" << ipaddr << "|task:" << logTars(task) << "|stat:" << logTars(stat) << endl);

    return ret_code;
}

////////////////////////////////////////////////////////////////
void ProxyServer::updateTask(const string &key, const TaskStat& task)
{
    Lock lock(*this);
    _summary.task[key] = task;
}

void ProxyServer::getSummary(BenchmarkSummary& summary)
{
    Lock lock(*this);
    summary = _summary;
}

bool ProxyServer::getResult(const string &key, ResultStat& stat)
{
    Lock lock(*this);
    auto task = _summary.task.find(key);
    auto result = _summary.result.find(key);
    if (task != _summary.task.end() && result != _summary.result.end())
    {
        stat = result->second;
        result->second.resetDefautlt();
        result->second.time_stamp = TNOW;
        task->second.fetch_time = TNOW;
        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    try
    {
        g_app.main(argc, argv);
        g_app.waitForShutdown();
    }
    catch (std::exception& e)
    {
        cerr << "std::exception:" << e.what() << std::endl;
    }
    catch (...)
    {
        cerr << "unknown exception." << std::endl;
    }
    return -1;
}
/////////////////////////////////////////////////////////////////
