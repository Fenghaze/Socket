#ifndef PROTO_H__
#define PROTO_H__
#include<stdint.h>
#define SERVERPORT      4567

enum CMD
{
    CMD_LOGIN = 1,      // 登录/登出命令宏
    CMD_LOGIN_RES,
    CMD_LOGOUT,
    CMD_LOGOUT_RES,
    CMD_NEW_USER_JOIN,
    CMD_ERROR
};

// 包头
struct DataHeader
{
    DataHeader()
    {
        dataLength = sizeof(DataHeader);
        cmd = CMD_ERROR;
    }
    short   dataLength; // 数据包长度
    short   cmd;        // 命令
};

// 包体
struct Login: public DataHeader // 继承包头
{
    Login() //构造函数初始化包头
    {
        dataLength = sizeof(Login);
        cmd = CMD_LOGIN;
    }
    char UserName[32];  // 登录用户名
    char PassWord[32];  // 登录密码
    char data[932];//用于测试
};


struct Logout: public DataHeader
{
    Logout()
    {
        dataLength = sizeof(Logout);
        cmd = CMD_LOGOUT;
    }
    char UserName[32];  // 登出的用户名
};

struct LoginResult: public DataHeader
{
    LoginResult()
    {
        dataLength = sizeof(LoginResult);
        cmd = CMD_LOGIN_RES;
        result = 0;
    }   /* data */
    int result;
    int data[992];//用于测试
};

struct LogoutResult: public DataHeader
{
    LogoutResult()
    {
        dataLength = sizeof(LogoutResult);
        cmd = CMD_LOGOUT_RES;
        result = 0;
    }   /* data */
    int result;
};

struct NewUserJoin: public DataHeader
{
    NewUserJoin()
    {
        dataLength = sizeof(NewUserJoin);
        cmd = CMD_NEW_USER_JOIN;
    } 
    int sock;  /* data */
};


#endif