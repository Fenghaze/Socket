<center>
    <h1>Select模型实现并发</h1>
	<small>APUE-NOTES/01.IO操作/高级IO.md</small>
</center>

# select函数

```c
int select(int nfds, fd_set *readfds, fd_set *writefds, 
           fd_set *exceptfds, struct timeval *timeout);
```

fd_set是位图

- nfds：监视的最大文件描述符+1
- readfds：文件描述符是否可读的集合（监听集合中的文件描述符的读事件）
- writefds：文件描述符是否可写的集合
- exceptfds：文件描述符是否异常的集合
- timeout：超时设置，**设置为0时，非阻塞**

如果成功，返回监听集合中满足条件的文件描述符个数；如果失败，返回-1和errno



问题1：如何将文件描述符添加到各个集合中？

先调用`FD_ZERO`，再`FD_SET`

问题2：对于成功返回的文件描述符个数，如何区分哪个fd来自与哪个集合？

for循环，调用`FD_ISSET`来判断每个集合中是否存在fd

```c
void FD_ZERO(fd_set *set);			//将集合清空

void FD_CLR(int fd, fd_set *set);	//将一个fd从set中清除

void FD_SET(int fd, fd_set *set);	//将一个fd添加到set中

int  FD_ISSET(int fd, fd_set *set);	//判断fd是否在集合中
```



缺点：监视的文件描述符类型太少；同时监听的文件描述符有上限（1024）；

优点：可移植性好



# 【示例】 ./select

==注意以下问题：==

**当有新的客户端要请求连接服务端时，需要监听哪个`socket`，加入哪个集合？**

对于服务端而言，新的客户端加入时，监听的是服务端的`socket`，加入可读集合



**客户端向服务端发送数据，这对于服务端中使用`select`监听的socket而言，是加入哪个集合？**

客户端使用`send`或`write`对socket写入数据后（相当于发送了数据）；于服务端而言，连接到的socket中就有内容了，于是服务端监听的socket==可读==，即加入可读集合



> 【示例2】客户端使用select模型接收服务端消息，使用线程输入命令：./select/client2.cpp

