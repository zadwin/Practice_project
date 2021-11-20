#include <stdio.h>
#include <stdlib.h> // 包含：exit函数
#include <string.h> // 包含：memset函数
#include <unistd.h> // 包含：close函数
#include <netdb.h>  // 包含：struct addrinfo结构体、getaddrinfo函数、freeaddrinfo函数
#include <sys/socket.h> // 包含：setsockopt函数、scoket函数
#include "csapp.h" // 如果是引入了头文件，一般很多时候在编写的时候就直接会被引用进来。

/**
 * @brief   客户端调用open_clientfd建立与服务器的连接。
 *
 * @param hostname   服务端主机IP或域名
 * @param port  服务端主机端口或服务
 * @return int    返回sockfd描述符。-1表示打开失败。
 */
int open_clientfd(char *hostname, char *port){
    int clinetfd = 0; // 声明一个用于接收系统返回的套接字描述符。
    struct addrinfo hints, *listp = NULL, *p = NULL; // hints用于设定一些参数。listp用于接收返回的套接字结构列表。p和listp相同，但是p是用来访问的。

    /* 1、获得可能的server 地址列表*/
    memset(&hints, 0, sizeof(struct addrinfo)); // 初始化hints.
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_NUMERICSERV; // 使用一个数值型的端口参数。参数service默认可以是服务名或端口号，这个标志强制参数service为端口号。
    hints.ai_flags |= AI_ADDRCONFIG; // 推荐连接的标志。
    int rc = 0; // 这个参数用来判断将主机名、主机地址和端口号等转化成套接字地址结构是否成功。
    if ( (rc = getaddrinfo(hostname, port, &hints, &listp) != 0)){ // 调用函数获得server地址列表。
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(rc)); // gai_seterror函数可以通过getaddrinfo函数的返回标志来判断发生的错误是什么。
        exit(1);
    }

    /* 2、遍历listp,直到我们可以成功连接的目标服务器。*/
    for (p = listp; p != NULL; p = p->ai_next)
    {
        /* 2.1 创建一个socket描述符。*/
        if ( (clinetfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0){
            continue; /* 创建socket失败，尝试下一个。 */
        }
        /* 2.2 创建成功，然后开始连接服务器。*/
        if( connect(clinetfd, p->ai_addr, p->ai_addrlen) != -1){
            printf("连接成功>>>>>\n");
            break; /* 连接成功。*/
        }
        close(clinetfd); // 连接失败，尝试下一个。
    }

    /* 3. 清除listp内存空间。*/
    freeaddrinfo(listp);
    if(!p){
        return -1; // 表示所有的连接都失败了。
    }else{
        return clinetfd;
    }
}

/**
 * @brief 调用open_listenfd函数，服务器创建一个监听描述符，准备好接收连接请求。
 *
 * @param port  服务器用于监听的端口号。
 * @return int  返回监听描述符。-1则表示创建失败。
 */
int open_listenfd(char *port){
    struct addrinfo hints, *listp = NULL, *p = NULL;
    int listenfd, optval = 1; // listen用于接收监听系统返回的监听描述符。

    /* 1、获得一系列潜在的server地址。*/
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG; // AI_PASSIVE表示返回的套接字地址可能被服务器用作监听套接字，在这种情况下，参数host应该为NULL。获得任意的IP地址。
    hints.ai_flags |= AI_NUMERICSERV; // 端口号使用数值。
    int rc = 0;
    if( (rc = getaddrinfo(NULL, port, &hints, &listp)) != 0) // 调用函数获得潜在的server地址。
    {
        fprintf(stderr, "getaddrinfo error：%s\n", gai_strerror(rc));
        exit(1);
    }

    /* 2、遍历listp地址列表，直到寻找到一个我们可以bind。*/
    for (p = listp; p != NULL; p = p->ai_next)
    {
        /* 2.1 创建一个socket描述符*/
        if( ( listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
        {
            continue;
        }

        /* 2.2 消除来自bind 返回的“address already in use”的错误。使得服务器能够被终止、重启和立即开始接收连接请求。 */
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

        /* 2.3 调用bind函数，将该地址到描述符。*/
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
        {
            break; // 绑定成功。
        }
        close(listenfd); // 绑定失败，尝试下一个。
    }

    /* 3、清理listp内存。*/
    freeaddrinfo(listp);
    if(!p)
    {
        return -1;  // 表示没有地址可以用于监听。
    }
    /* 4、制作一个监听socket，准备给accept连接请求。listen函数把主动连接socket变为被动连接的socket，使得这个socket可以接受其他socket连接请求，从而成为一个服务端的socket。socket是已经被bind绑定过的socket，
            socket函数返回的socket是一个主动连接的socket，在服务端的编程中，程序员希望这个socket可以接收外来请求，也就是被动等待客户端来连接。由于系统默认认为一个socket是主动连接的，所以需要通过某种方式来告诉系统，
            通过调用listen来完成这件事。第二个参数backlog代表等待队列的最大长度。*/
    if(listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}