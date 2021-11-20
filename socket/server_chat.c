#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include "csapp.h"

void server_chat(int connfd)
{
    char buffer[1024]; // 这里的buffer只是为了使用它显示的取数据，还不是真正的socket缓冲区。
    int iret = 0;
    while (1)
    {
        memset(buffer, 0, sizeof(buffer));
        // 开始接收客户端的报文。
        printf("\n\n.......等待客户端发送消息......\n");           // 一定要特别注意c语言的输出缓冲区问题。如果没有\n那么有可能>就不会立即输出。
        if ((iret = recv(connfd, buffer, sizeof(buffer), 0)) <= 0) // 接收客户端的请求报文。
        {
            printf("iret=%d\n", iret);
            break;
        }
        printf("接收消息：%s\n", buffer);
        if (strcmp(buffer, "Bye") == 0)
        {
            printf("..............聊天结束............\n");
            break;
        }
        // strcpy(buffer, "ok"); // 该函数的功能是，把一个字符串复制到另一个字符串中。
        memset(buffer, 0, sizeof(buffer));
        printf("请输入您要发送的消息：");
        scanf("%s", buffer);
        if ((iret = send(connfd, buffer, strlen(buffer), 0)) <= 0) // 向客户端发送响应结果。
        {
            perror("send");
            break;
        }
    }
}