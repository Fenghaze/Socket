/*
建立10000个客户端连接，使用4个线程
给每个线程分配2500个客户端，用于处理客户端的操作
目前情况：客户端的操作仅为向服务器循环发送数据
*/

#include <stdio.h>
#include <stdlib.h>

#include <thread>
#include "EasyTcpClient.hpp"
using namespace std;
#define TNUM   4 
#define CLIENTS 1000 
bool flag = true;
EasyTcpClient *client[CLIENTS];
static void cmdThread()
{
    while (1)
    {
        char sendbuf[BUFSIZ];
        // 从终端输入请求命令
        fgets(sendbuf, sizeof(sendbuf), stdin);
        // 处理请求
        if (strcmp(sendbuf, "exit\n") == 0)
        {
            flag = false;
            break;
        }
        else
            printf("type error!\n");
    }
}

// 为每个线程分配平均的客户端
void sendThread(int tid)
{
    int c = CLIENTS / TNUM;
    int begin = (tid-1) *c;
    int end = tid*c;
    Login login;
    strcpy(login.UserName, "xiaohong");
    strcpy(login.PassWord, "ferwr");
    int flag_num = 0;
    while (flag)
    {
        for (int i = begin; i < end; i++)
        {
            if (client[i]->OnRun() == false)
            {
                flag_num++;
                if (flag_num == CLIENTS)
                {
                    flag = false;
                    break;
                }
                else
                    continue;
            }
            client[i]->sendData(&login);
        }
    }
    for (int i = begin; i < end; i++)
    {
        client[i]->closeSocket();
    }
}

int main()
{
    // 启动输入线程
    thread t1(cmdThread);
    t1.detach();

    for (int i = 0; i < CLIENTS; i++)
    {
        client[i] = new EasyTcpClient();
        client[i]->initSocket();
        client[i]->Connect("127.0.0.1", SERVERPORT);
    }
    // 启动发送线程
    for (int i = 0; i < TNUM; i++)
    {
        thread t2(sendThread, i + 1);
        t2.join(); 
    }
    
    return 0;
}
