#include "Monitor.h"
#include "Transport.h"
#include "signal.h"
#include <sys/wait.h>
using namespace bm;

// LICOTE命令声明
LICOTE_OPTION_BEGIN
LICOTE_OPTION_DECL("-c", 	NULL, 	"连接数量");
LICOTE_OPTION_DECL("-D", 	NULL, 	"目标服务器IP");
LICOTE_OPTION_DECL("-P", 	NULL, 	"网络传输端口");
LICOTE_OPTION_DECL("-T", 	"o", 	"网络传输协议(tcp|udp)");
LICOTE_OPTION_DECL("-I", 	"o", 	"压测总时长");
LICOTE_OPTION_DECL("-t", 	"o", 	"超时时间");
LICOTE_OPTION_DECL("-s", 	"o", 	"指定速率");
LICOTE_OPTION_DECL("-i", 	"o", 	"查看间隔时间");
LICOTE_OPTION_DECL("-n", 	"o", 	"制定最大进程限制");
LICOTE_OPTION_DECL("-p", 	"o", 	"接口通信协议(tars|http)");
LICOTE_OPTION_DECL("-h", 	"h", 	"帮助信息");

LICOTE_SET_LANGUAGE(CN);
LICOTE_SET_VERSION("GPL");
LICOTE_SET_EXAMPLE("./eab -c 2000 -s 8000 -D 192.168.16.1 -P 10505 -p tars -i 10;"
                   "./eab -c 2000 -s 8000 -t 6000 -D 127.0.0.1,192.168.32.1 -P 80 -u http://www.qq.com");
LICOTE_SET_DESCRIPTION("增强型压测工具");
LICOTE_OPTION_END

/**
*  全局变量定义
*/
size_t              gSpeed;
size_t              gConsNum;
size_t              gInterval;
int64_t             gRunCores;
string              gProtoName;
bool                gRunFlag;
IntfStat            gStatInf;
vector<Endpoint>    gEps;
int64_t             gStartTime;


/**
*  初始化函数
*/
int initialize(int argc, char* argv[])
{
    try
    {
        // LICOTE初始化
        if (licote_option_init(argc, argv) != 0)
        {
            return BM_INIT_PARAM;
        }

        // 获取压测核数
        gRunCores = std::max(LICODE_GETINT("-n", getProcNum()), (int64_t)1);

        // EndPoint初始化
        Endpoint ep("", LICODE_GETINT("-P", 0), LICODE_GETINT("-t", 3000));
        ep.setTcp(LICODE_GETSTR("-T", "tcp") != string("udp"));
        vector<string> vd = sepstr(LICODE_GETSTR("-D", ""), ",");
        for (size_t ii = 0; ii < vd.size() && !vd[ii].empty(); ii++)
        {
            ep.setHost(vd[ii]);
            gEps.push_back(ep);
        }

        if (ep.getTimeout() == 0 || ep.getPort() == 0 || gEps.size() == 0)
        {
            licote_option_help("参数格式不正确: 目标服务器配置错误\n");
        }

        // 压测参数初始化
        gSpeed    = (LICODE_GETINT("-s", 0) + gRunCores - 1) / gRunCores;
        gConsNum  = (LICODE_GETINT("-c", 0) + gRunCores - 1) / gRunCores;
        if (gConsNum == 0  || LICODE_GETINT("-i", 10) == 0)
        {
            licote_option_help("参数格式不正确: -i|-c 不能为0\n");
        }
        else if (gConsNum * gEps.size() > MAX_FD)
        {
            licote_option_help("参数格式不正确: 连接数超过系统限制\n");
        }

        ProtoFactory protoFactory;
        gProtoName = LICODE_GETSTR("-p", "http") + "Protocol";
        if (protoFactory.get(gProtoName, argc, argv) == NULL)
        {
            licote_option_help("参数格式不正确: 协议不存在\n");
        }

        gInterval = gSpeed == 0 ? 0 : (1000000 * gConsNum) / gSpeed;
        memset(&gStatInf, 0, sizeof(IntfStat));
        Monitor::getInstance()->initialize();
        gRunFlag = true;
    }
    catch (exception& e)
    {
        licote_option_help("系统异常: %s\n", e.what());
    }
    return 0;
}

/**
*  压测主程序run
*/

int run(int seqNum, int argc, char* argv[])
{
    // epoll初始化
    EPollLoop    eLoop;
    eLoop.initialize(MAX_FD);

    // 创建连接
    vector<Transport*> vCons;
    for (size_t i = 0; i < gConsNum; i++)
    {
        for (size_t ii = 0; ii < gEps.size(); ii++)
        {
            if (gEps[ii].isTcp())
            {
                Transport* pConn = new TCPTransport(gEps[ii], &eLoop);
                pConn->initialize(gProtoName, argc, argv);
                vCons.push_back(pConn);
            }
            else
            {
                Transport* pConn = new UDPTransport(gEps[ii], &eLoop);
                pConn->initialize(gProtoName, argc, argv);
                vCons.push_back(pConn);
            }
        }
    }

    int64_t requestId = seqNum;
    int64_t nextSendTime = 0;
    while (gRunFlag)
    {
        int64_t curSendTime = getNow();
        if (curSendTime >= nextSendTime)
        {
            nextSendTime = curSendTime + gInterval;
            for (size_t i = 0; i < vCons.size(); i++)
            {
                vCons[i]->trySend(requestId);
                vCons[i]->checkTimeOut(curSendTime/1000);
                requestId += gRunCores;
            }
        }

        Monitor::getInstance()->syncStat(curSendTime/1000);
        eLoop.loop(1);
    }

    // 析构链接
    eLoop.loop(LICODE_GETINT("-t", 3000));
    Monitor::getInstance()->syncStat(0);
    for (size_t i = 0; i < vCons.size(); i++)
    {
        delete vCons[i];
    }
    vCons.clear();
    return 0;
}

/**
*  处理信号退出
*/
void procSignal(int signo)
{
    if (SIGINT == signo || SIGUSR1 == signo || SIGTERM == signo || SIGKILL == signo)
    {
        gRunFlag = false;
    }
}

/**
*  输出压测结果
*/
void printCost(const IntfStat& statInf)
{
    double dTotalCount = 0.0;
    double dMegerCount = 0.0;
    static int arrCost[MAX_STEP_COST] = {0, 10, 30, 50, 100, 500, 3000, 5000, 99999, 0};
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        dTotalCount += statInf.costTimes[i];
    }

    dTotalCount = max(1.0, dTotalCount);
    for (size_t i = 0; i < MAX_STEP_COST && arrCost[i + 1] != 0; i++)
    {
        dMegerCount += statInf.costTimes[i];
        printf("[%5d - %5d ms] %7d\t%2.2f%%\t\t%2.2f%%\n", arrCost[i], arrCost[i+1], statInf.costTimes[i],
            100 * (double)statInf.costTimes[i] / dTotalCount, 100 * dMegerCount / dTotalCount);
    }
}

void printPeriod(int intvlTime)
{
    IntfStat statInf;
    map<int, int> mRetFinal;
    vector<IntfStat> vStatList;
    if (Monitor::getInstance()->fetch(vStatList))
    {
        for (size_t ii = 0; ii < vStatList.size(); ii++)
        {
            map<int, int> mRet = str2map(string((char *)vStatList[ii].retCount));
            for (map<int, int>::iterator itm = mRet.begin(); itm != mRet.end(); itm++)
            {
                mRetFinal[itm->first] += itm->second;
            }
            statInf += vStatList[ii];
        }
        gStatInf += statInf;
    }

    if (intvlTime > 0)
    {
        double totalDecimal = std::max(1.00, (double)gStatInf.totalCount);
        double failRate = min(statInf.failCount, statInf.totalCount) / totalDecimal;
        printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
        printf("Time\t\t\tTotal\tSucc\tFail\tRate\tMax(ms)\tMin(ms)\tAvg(ms)\tP90(ms)\tP99(ms)\tP999(ms)\tTPS\n");
        printf("%s\t%-6d\t%-6d\t%-6d\t%0.2f%%\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t%0.2f\t\t%d",
            tm2str((time_t)TNOWMS/1000), statInf.totalCount, statInf.succCount, statInf.failCount, (1 - failRate) * 100,
            statInf.maxTime, statInf.minTime, statInf.totalTime/totalDecimal,
            statInf.p90Time, statInf.p99Time, statInf.p999Time,
            statInf.totalCount/intvlTime);

        printf("\n\n\nCall Result: [%s]\n", map2str(mRetFinal).c_str());
        printf("--------------------------------------------------------------------------------------------------------------------\n");
        printCost(statInf);
    }
}

void printFinal(int intvlTime)
{
    printPeriod(0);
    string sEndPoint = gEps[0].toString();
    for (size_t ii = 1; ii < gEps.size(); ii++)
    {
        sEndPoint.append("\n                        ").append(gEps[ii].toString());
    }

    double totalDecimal = std::max(1.00, (double)gStatInf.totalCount);
    double failRate = min((double)gStatInf.failCount, (double)gStatInf.totalCount) / totalDecimal;

    int realIntvlTime = intvlTime - LICODE_GETINT("-t", 3000) / 3000;
    printf("\n\n--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("----------------------------------   Finish Pressure Test   --------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("--------------------------------------------------------------------------------------------------------------------\n");
    printf("Server Numbers:         %ld\n", gEps.size());
    printf("Server Endpoint:        %s\n",  sEndPoint.c_str());
    printf("Server Protocol:        %s\n\n\n", LICODE_GETSTR("-p", "").c_str());

    printf("Concurrency Procesor:     %ld\n", gRunCores);
    printf("Concurrency Connections:  %ld\n", gConsNum * gRunCores);
    printf("Connections per Procesor: %ld\n", gConsNum);
    printf("Success requests:         %d\n", gStatInf.succCount);
    printf("Success rate:             %.2f%%\n", (1 - failRate) * 100);
    printf("Failed requests:          %d\n", gStatInf.failCount);
    printf("Total requests:           %d\n", gStatInf.totalCount);
    printf("Total duration:           %d[sec]\n", realIntvlTime);
    printf("Transfer rate:            %.2f[Kbytes/sec]\n", (double)gStatInf.totalSendBytes/(realIntvlTime*800));
    printf("Requests per second:      %d[#/sec](mean)\n", gStatInf.totalCount/realIntvlTime);
    printf("Request size(Avg):        %ld\n", gStatInf.totalSendBytes/gStatInf.totalCount);
    printf("Response size(Avg):       %ld\n", gStatInf.totalRecvBytes/gStatInf.totalCount);
    printf("Latency time(Avg):        %2.2f[ms]\n", gStatInf.totalTime/totalDecimal);
    printf("Latency time(P90):        %2.2f[ms]\n", gStatInf.p90Time);
    printf("Latency time(P99):        %2.2f[ms]\n", gStatInf.p99Time);
    printf("Latency time(P999):       %2.2f[ms]\n", gStatInf.p999Time);
    printf("\n\n");

    printf("Percentage of the requests served within a certain time\n");
    printCost(gStatInf);
}

// 函数入口
int main(int argc, char* argv[])
{
    // 环境初始化
    vector<int> vPids;
    if (initialize(argc, argv) != 0)
    {
        return 0;
    }

    // 注册信号
    signal(SIGINT,  procSignal);
    signal(SIGKILL, procSignal);
    signal(SIGUSR1, procSignal);
    signal(SIGTERM, procSignal);

    // 创建子进程进行压测
    gStartTime = TNOWMS;
    for (int ip = 0; ip < gRunCores; ip++)
    {
        int pid = fork();
        if (pid == 0)
        {
            signal(SIGPIPE, SIG_IGN);
            return run(ip, argc, argv);
        }
        vPids.push_back(pid);
    }

    // 主进程外显
    printPeriod(0);
    int64_t tCurTime = TNOWMS;
    int iBenIntvl  = LICODE_GETINT("-I", 3600); // 默认1小时
    int iViewIntvl = LICODE_GETINT("-i", 5);
    while (iBenIntvl > 0 && gRunFlag)
    {
        sleep(1);
        iBenIntvl -= 1;
        if (abs(TNOWMS - tCurTime) > iViewIntvl * 1000)
        {
            tCurTime = TNOWMS;
            printPeriod(iViewIntvl);
        }
    }

    // 先杀死进程，然后打印统计
    for (size_t ii = 0; ii < vPids.size(); ii++)
    {
        kill(vPids[ii], SIGUSR1);
    }

    int status;
    while (wait(&status) > 0);
    printFinal((TNOWMS - gStartTime) / 1000);
    return 0;
}

