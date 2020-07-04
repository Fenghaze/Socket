**具体实现详见：./object_oriented**

实现客户端和服务端的封装

并解决了粘包、少包问题

使用C++提供的计时器来计算每秒收到所有客户端发送过来的包的个数



- client.cpp：实现单个客户端向服务端发送消息
- multi_client.cpp：实现多个客户端向服务端发送消息
- server.cpp：接收客户端的消息

# 封装客户端 

> EasyTCPClient.hpp

```c++
#ifndef EASYTCPCLIENT_H__
#define EASYTCPCLIENT_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "proto.h"
#include <unistd.h>
#include <string.h>

#define INVALID_SOCKET -1
class EasyTcpClient
{
private:
    int cfd;

public:
    EasyTcpClient(){}

    virtual ~EasyTcpClient(){}

    // 创建socket
    int initSocket(){}

    // 连接服务端
    int Connect(char *ip, short port){}

    // 关闭socket
    void closeSocket(){}
    
    // 监听socket
    bool OnRun(){}

    // 接收数据
    int recvData(int cfd){}

    // 处理接收到的数据
    void OnNetMsg(DataHeader *header){}

    // 发送数据
    int sendData(DataHeader *header){}

    // 判断socket是否在工作
    bool isRun(){}
};

#endif
```



# 封装服务端

> EasyTcpServer.hpp

```c++
#ifndef EASYTCPSERVER_H__
#define EASYTCPSERVER_H__

#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "proto.h"
#include <unistd.h>
#include <string.h>
#include <string>
#include <vector>

#define INVALID_SOCKET -1
#define IPSIZE 40
class EasyTcpServer
{
private:
    int lfd;
    struct sockaddr_in laddr, raddr;
    fd_set readfds, writefds;
    socklen_t raddr_len;
    std::vector<int> g_clients;

public:
    EasyTcpServer(){}
    ~EasyTcpServer(){}
    
    // 创建socket
    int initSocket(){}

    // 关闭socket
    void closeSocket(){}

    // 绑定ip、port
    int Bind(const char *ip, unsigned short port){}
   
    // 监听
    int Listen(int num){}

    // 接受新客户端连接请求，并将该客户端加入动态数组
    int Accept(){}

    // 监听socket
    bool OnRun(){}

    // 发送数据给指定客户端
    int sendData(DataHeader *header, int cfd){}

    // 发送数据给所有客户端
    void sendData2All(DataHeader *header){}

    // 接收数据
    int recvData(int cfd){}

    // 处理数据
    void OnNetMsg(DataHeader *header, int cfd){}

    // 判断socket是否在工作
    bool isRun(){}
};
#endif
```



