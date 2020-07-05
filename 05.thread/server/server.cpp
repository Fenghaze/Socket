/*
使用select模型
一次接收多个客户端的请求，处理请求，发送数据给客户端
返回客户端的登录/登出状态
每当有新的客户端加入时，向所有连接的客户端发送新的客户端消息
*/

#include <stdio.h>
#include <stdlib.h>
#include<thread>
#include "EasyTcpServer.hpp"


using namespace std;

bool g_bRun = true;
void cmdThread()
{
	while (true)
	{
		char cmdBuf[256] = {};
		scanf("%s", cmdBuf);
		if (0 == strcmp(cmdBuf, "exit\n"))
		{
			g_bRun = false;
			printf("退出cmdThread线程\n");
			break;
		}
		else {
			printf("不支持的命令。\n");
		}
	}
}
int main()
{
    EasyTcpServer server;
    server.initSocket();
    server.Bind(nullptr, SERVERPORT);
    server.Listen(50);
    server.StartServers();

    //启动UI线程
	std::thread t1(cmdThread);
	t1.detach();


    while(g_bRun)
    {
        server.OnRun();
    }
    server.closeSocket();
    return 0;
}
