#ifndef PROTO_H__
#define PROTO_H__
#include<stdint.h>
#define SERVERPORT      4567

enum CMD
{
    CMD_LOGIN = 1,      // 登录/登出命令宏
    CMD_LOGOUT,
    CMD_ERROR
};

// 包头
struct DataHeader
{
    short   dataLength; // 数据包长度
    short   cmd;        // 命令
};

// 包体
struct Login
{
    char UserName[32];  // 登录用户名
    char PassWord[32];  // 登录密码
};

struct LoginResult
{
    int result;         // 登录成功返回的结果
};

struct Logout
{
    char UserName[32];  // 登出的用户名
};

struct LogoutResult
{
    int result;         // 登出返回的结果
};


#endif