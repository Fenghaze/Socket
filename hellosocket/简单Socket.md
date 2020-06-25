![](../assets/2020-06-25 20-34-08 的屏幕截图.png)



# TCP服务端

- 1、`socket()`建立一个socket
- 2、`bind()`**申请一个端口用于服务端的socket服务**

- 3、`listen()`监听网络端口号
- 4、`accept()`等待客户端连接
- 5、`recv()`接收客户端发送的数据
- 6、`send()`响应客户端，向客户端发送一条数据
- 7、`close()`关闭socket



# TCP客户端

- 1、`socket()`建立一个socket
- 2、`connect()`发起请求，连接服务器
- 3、`send()`连接成功后，向服务端发送数据

- 4、`recv()`接收服务端发送的数据
- 5、`close()`关闭socket