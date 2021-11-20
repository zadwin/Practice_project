#define LISTENQ 5 // 用于指定完成监听队列的长度。
#define MAXLINE 1024 // 用于接收addrinfo等的空间。
/* 1、open_clientfd描述：客户端调用open_clinetfd函数建立与服务器的连接。
            hostname参数：服务器主机（IP或域名），port参数：端口号port 。*/
int open_clientfd(char *hostname, char *port);  //返回：若成功则返回描述符，若出错则为-1。

/* 2、open_listenfd描述：调用open_listenfd函数，服务器创建一个监听描述符，准备好接收连接请求。
            port参数：端口号port。*/
int open_listenfd(char *port);  // 返回：若成功则返回描述符，若出错则为-1。

void server_chat(int connfd); // 服务端实现与connfd的socket描述进行通信。

void client_chat(int clientfd); // 客户端实现与服务器的socket描述进行通信。