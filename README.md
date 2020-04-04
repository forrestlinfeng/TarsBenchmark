## 简单介绍

eab专门为tars服务量身订做的无码压测工具，利用EPOLL和多进程，充分发掘压测机的性能，8核机器能模拟20W+/S的输出能力，具备以下特性：

 - 网络高性能：8核机器TPS支持超过20W/S;
 - 通讯扩展性：网络层支持TCP/UDP两种协议方式;
 - 协议扩展性：应用层支持HTTP,TARS压测，提供协议开放能力;
 - 完善实时的统计与监控支持。提供周期内的请求数/TPS/耗时/成功率分布;


## 使用说明

### 举个栗子：
./eab -c 600 -s 6000 -D 192.168.31.1 -P 10505 -p tars -S tars.DemoServer.DemoObj -M add -C test.txt

### -C参数说明
用例文件建议使用tars2case工具自动生成接口用例

#### TARS服务用例编写说明:
文件分为上下部分，用"#"开头行分割，上半部分为RPC参数，下半部分为RPC调用参数的值，跟参数一一对应

- **参数帮助说明**：
 1. 输入参数使用"|"符号进行分割，即jce参数中的","替换为"|"
 2. struct的表示方法是:struct<tag require|optional 字段1,字段2,字段3...>,如果tag从0开始，直接字段1
 3. vector的表示方法是:vector<类型>
 4. map的表示方法:map<key类型,value类型>
 5. 2、3、4可以嵌套使用

- **参值帮助说明**：
 1. <strong>基本类型</strong>的随机值设置：
    <strong>范围随机值</strong>用[1-100]表示，表示在1-100内随机出现, 必须是数字
    <strong>限定随机值</strong>用[1,123,100]表示，表示在1,123,100中随机出现，可以是字符串
 2. 输入参数每个参数一行,也就是jce参数列表中","的地方换成回车
 3. struct的表示方法是:<字段值1,字段值2,字段值3...>
 4. vector的表示方法是:<值1,值2,值3...>
 5. map的表示方法:[key1=val1,key2=val2,key3=val3...]
 6. 3、4、5可以嵌套使用

- **例如**：
vector<string>|struct<string, int>|map<string, string>
#######
<abc, def, tt, fbb>
<abc, 1>
[abc=def, dfd=bbb]

### 压测结果显示
![压测结果](docs/image/result.jpg)

## 新协议开发
EAB提供协议开放开发能力，如果你服务的协议不是TARS/HTTP，需要适配下服务的协议，三步就能搞定：
 - 1.参考httpProtocol继承Protocol协议类。
 - 2.实现input/encode/decode三个编解码方法。
 - 3.实现初始化函数，截取命令行需要的参数。

在开发工作准备前，你需要了解压测的目标服务属于那种模式：
**有序模式**
在模式下压测客户端会按照指定速率匀速向目标服务器发送请求，不依赖服务端的返回，在发送请求的时候，客户端会生成**全局唯一id**给服务端，服务端回包的时候会携带这个id给客户端。
**![有序模式](docs/image/ordered.jpg)**

**无序模式**
在模式下压测客户端会低于指定速率匀速向目标服务器发送请求，强依赖服务端的返回（一般服务端不回序号给客户端），在发送下一个请求前必须收到上一个请求的响应或超时。以下图为例。
![无序模式](docs/image/disordered.jpg)

## FAQ
 - eab指定速率何解？
1.速率指定单位是单目标机，如果目标机有n台，统计的TPS=速率*n
2.如果不指定压测速率，eab将全速向目标机发起冲击。

 - eab高性能是如何实现的？
首先，利用epoll避免进程的网络IO阻塞，最大程度提升CPU利用率，
其次，工具会根据CPU核数量创建数量相同的压测进程，连接数和压测速率会均分到子进程上。

 - eab压测进程之间数据同步如何同步？
通过无锁共享内存队列实现数据共享，统计结果在主进程输出
