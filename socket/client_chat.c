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
        // 同样也会存在和recv相同的结果，因为不能一次性写入这么多内容，可能是因为缓冲区的问题。
        if ((iret = send(clientfd, buffer, strlen(buffer), 0)) <= 0)
        {
            perror("send");
            break;
        }
        memset(buffer, 0, sizeof(buffer));
        printf("等待服务器发送消息...\n");
        // recv函数因为要指定接收数据的大小，因此如果一个包过大，需要用循环的方式提取。不能完整的读取数据。
        // 因此，可以自己构造一个函数，用以去接收完整的数据。
        if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0)
        {
            printf("iret = %d,对方已下线。\n", iret);
            break;
        }
        printf("接收消息：%s\n", buffer);
    }
}