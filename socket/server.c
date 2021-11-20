#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h> // 包含：close函数
#include <arpa/inet.h>
#include "csapp.h"

int main(int argc, char **argv){
    int listenfd, connfd;
    // struct sockaddr_storage clientaddr;
    if(argc != 2)
    {
        fprintf(stderr, "usage: %s <port> \n", argv[0]);
        exit(0);
    }
    listenfd = open_listenfd(argv[1]);
    printf("开始接受监听...\n"); // 当服务器开始运行时，会等待客服端的连接。

    /* 开始监听。*/
    int clientfd;                             // 客户端的socket。
    int socklen = sizeof(struct sockaddr_in); // struct sockaddr_in的大小
    struct sockaddr_in clientaddr;            // 客户端的地址信息。
    clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
    printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));

    // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
    char buffer[1024]; // 这里的buffer只是为了使用它显示的取数据，还不是真正的socket缓冲区。
    while (1)          // 服务端虽然这里是一个循环，但是由于后面的if语句中的recv会判断是否客户端还连接。如果不连接就会终止该程序。
    {
        int iret;
        memset(buffer, 0, sizeof(buffer));
        // 开始接收客户端的报文。
        printf("等待客户端发送消息报文...\n");                       // 一定要特别注意c语言的输出缓冲区问题。如果没有\n那么有可能就不会立即输出。
        if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) // 接收客户端的请求报文。
        {
            printf("iret=%d\n", iret);
            break;
        }
        printf("接收消息：%s\n", buffer);
        if (strcmp(buffer, "Bye") == 0)
        {
            printf("聊天结束...\n");
            break;
        }
        // strcpy(buffer, "ok"); // 该函数的功能是，把一个字符串复制到另一个字符串中。
        memset(buffer, 0, sizeof(buffer));
        printf("请输入您要发送的消息：");
        scanf("%s", buffer);
        if ((iret = send(clientfd, buffer, strlen(buffer), 0)) <= 0) // 向客户端发送响应结果。
        {
            perror("send");
            break;
        }
    }
    // 第6步：关闭socket，释放资源。
    close(listenfd);
    close(clientfd);
    return 0;
}