/*
使用线程执行：输入命令来发送请求给服务器
使用select模型接收服务端的数据
*/

#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include "EasyTcpClient.hpp"

using namespace std;

static void cmdThread(EasyTcpClient *client)
{
    while (1)
    {
        char sendbuf[BUFSIZ];
        // 从终端输入请求命令
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 处理请求
        if (strcmp(sendbuf, "exit\n") == 0)
        {
            client->closeSocket();
            break;
        }
        else if (strcmp(sendbuf, "login\n") == 0)
        {
            // 向服务端发送登录信息
            Login login;
            strcpy(login.UserName, "xiaohong");
            strcpy(login.PassWord, "ferwr");
            client->sendData(&login);
        }
        else if (strcmp(sendbuf, "logout\n") == 0)
        {
            Logout logout;
            strcpy(logout.UserName, "xiaohong");
            client->sendData(&logout);
        }
        else
            printf("type error!\n");
    }
}

int main()
{
    EasyTcpClient client;
    client.initSocket();
    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (client.Connect("127.0.0.1", 4567) < 0)
        exit(1);

    // 启动输入命令的线程
    thread t1(cmdThread, &client);
    t1.detach(); //线程分离

    Login login;
    strcpy(login.UserName, "xiaohong");
    strcpy(login.PassWord, "ferwr");
    

    while (client.isRun())
    {
        if(client.OnRun()==false)
            break;
        client.sendData(&login);
    }

    client.closeSocket();

    return 0;
}
