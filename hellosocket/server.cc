#include<stdlib.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>

#define CLPORT      1089

int main()
{
    int sd;
    struct sockaddr_in addr;:
    // 1、建立一个socket
    sd = socket(AF_INET, SOCK_STREAM, IPPROT_TCP);
    
    // 2、绑定客户端的网络端口号
    addr.sin_family = AF_INET;
    addr.sin_port = CLPORT;
    addr.sin_
    bind(sd, (void *)&addr, sizeof(addr));


    return 0;
}