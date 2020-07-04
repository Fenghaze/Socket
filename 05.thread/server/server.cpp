/*
使用select模型
一次接收多个客户端的请求，处理请求，发送数据给客户端
返回客户端的登录/登出状态
每当有新的客户端加入时，向所有连接的客户端发送新的客户端消息
*/

#include <stdio.h>
#include <stdlib.h>
#include "EasyTcpServer.hpp"


using namespace std;


int main()
{
    EasyTcpServer server;
    server.initSocket();
    server.Bind(nullptr, SERVERPORT);
    server.Listen(50);
    while(server.isRun())
    {
        server.OnRun();
    }
    server.closeSocket();
    return 0;
}
