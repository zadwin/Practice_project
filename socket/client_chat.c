#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "csapp.h"

void client_chat(int clientfd)
{
    char buffer[1024]; // 这里的buffer只是为了使用它显示的取数据，还不是真正的socket缓冲区。
    int iret = 0;
    /* 开始聊天.... */
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
}