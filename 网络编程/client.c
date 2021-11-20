/*
 * 程序名：client.cpp，此程序用于演示socket的客户端
 * 作者：C语言技术网(www.freecplus.net) 日期：20190525
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
    /* 运行客户端的时候需要确定给定访问服务器的IP地址和端口号，因此这里要判断是否有三个参数。 */
    if (argc != 3)
    {
        printf("Using:./client ip port\nExample:./client 127.0.0.1 5005\n\n");
        return -1;
    }

    // 第1步：创建客户端的socket。主要是向系统申请一个socket资源。这里也要防止系统资源耗尽，否则不会失败。
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    // 第2步：向服务器发起连接请求。gethostbyname可以将IP地址或域名换为hostent结构体，这个结构体里面包含主机名、多个IP、多个域名等等。这个函数只用于客户端。其实也就是有域名解析的功能。
    struct hostent *h;

    if ((h = gethostbyname(argv[1])) == 0) // 指定服务端的ip地址。
    {
        printf("gethostbyname failed.\n");
        close(sockfd);
        return -1;
    }
    // 套接字结构体。协议版本、IP地址和端口号。在一定程度上可以认为每一个属性都变成了内置类型。
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2])); // 指定服务端的通信端口。
    memcpy(&servaddr.sin_addr, h->h_addr, h->h_length);// C 库函数 void *memcpy(void *str1, const void *str2, size_t n) 从存储区 str2 复制 n 个字节到存储区str1。
    // 连接请求的时候，不仅要给对方的地址等，还要告诉对方自己的通信socket等。这个函数的返回值：0表示成功，-1表示失败。失败一般可能是服务器地址错了，或者端口号错了，或则会服务的没有启动。
    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0) // 向服务端发起连接清求。connect函数用于将参数sockfd的socket连至serv_addr指定的服务的。因为在linux中所有打开的文件都有唯一的id号（即文件描述符）。
    {
        perror("connect");
        close(sockfd);
        return -1;
    }

    char buffer[1024];

    // 第3步：与服务端通信，发送一个报文后等待回复，然后再发下一个报文。
    /* 实现实时聊天。*/
    while(1)
    {
	    int iret = 0;
	    memset(buffer, 0, sizeof(buffer));
	    printf("请输入您要发送的消息内容：");
	    scanf("%s", buffer);
	    if ((iret = send(sockfd, buffer, strlen(buffer), 0)) <= 0)
	    {
	    	perror("send");
		break;
	    }
	    memset(buffer, 0, sizeof(buffer));
	    printf("等待服务器发送消息...\n");
	    if((iret = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0)
	    {
	    	printf("iret = %d,对方已下线。\n", iret);
		break;
	    }
	    printf("接收消息：%s\n", buffer);
    }
    /* for (int ii = 0; ii < 3; ii++)
    {
        int iret;
        memset(buffer, 0, sizeof(buffer));
	// 为了测试服务端是否一直处于等待客户端接收数据的状态。
        sprintf(buffer, "客户端1，这是第%d个超级女生，编号%03d。", ii + 1, ii + 1);
        if ((iret = send(sockfd, buffer, strlen(buffer), 0)) <= 0) // 向服务端发送请求报文。
        {
            perror("send");
            break;
        }
        printf("发送：%s\n", buffer);
	// 测试服务端发送过来的数据，是否我们可以晚接收。
        memset(buffer, 0, sizeof(buffer));
        if ((iret = recv(sockfd, buffer, sizeof(buffer), 0)) <= 0) // 接收服务端的回应报文。
        {
            printf("iret=%d\n", iret);
            break;
        }
        printf("接收：%s\n", buffer);
    }
    */

    // 第4步：关闭socket，释放资源。
    close(sockfd);
}
