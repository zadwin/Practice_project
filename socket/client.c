#include <stdio.h> // 包含：fprintf函数、stderr
#include <stdlib.h> // 包含：exit函数
#include <string.h> // 包含：memset函数。
#include <unistd.h> // 包含：close函数
#include <sys/socket.h> // 包含：socket函数、send函数、recv函数
#include "csapp.h"

int main(int argc, char **argv){
    int clientfd = 0;
    char *host, *port;
    if(argc != 3)
    {
        fprintf(stderr, "usage：%s <host> <post> \n", argv[0]);
        exit(1);
    }
    host = argv[1];
    port = argv[2];
    clientfd = open_clientfd(host, port);

    /* 开始聊天.... */
    char buffer[1024];
    // 与服务端通信，发送一个报文后等待回复，然后再发下一个报文。
    /* 实现实时聊天。*/
    while (1)
    {
        int iret = 0;
        memset(buffer, 0, sizeof(buffer));
        printf("请输入您要发送的消息内容：");
        scanf("%s", buffer);
        if ((iret = send(clientfd, buffer, strlen(buffer), 0)) <= 0)
        {
            perror("send");
            break;
        }
        memset(buffer, 0, sizeof(buffer));
        printf("等待服务器发送消息...\n");
        if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0)
        {
            printf("iret = %d,对方已下线。\n", iret);
            break;
        }
        printf("接收消息：%s\n", buffer);
    }
    close(clientfd);
    return 0;
}