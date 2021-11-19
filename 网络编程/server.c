/*
 * 程序名：server.cpp，此程序用于演示socket通信的服务端
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
    if (argc != 2)
    {
        printf("Using:./server port\nExample:./server 5005\n\n");
        return -1;
    }

    // 第1步：创建服务端的socket。
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return -1;
    }
    // 这个语句没有输出，很多时候是因为缓冲设置的问题。需要加上\n才能立即输出内容。
    printf("中间过程...\n");
    // 第2步：把服务端用于通信的地址和端口绑定到socket上。服务端才需要将自己的Ip端口等进行绑定。因为它是等待其他客户的连接。
    struct sockaddr_in servaddr; // 服务端地址信息的数据结构。
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;                // 协议族，在socket编程中只能是AF_INET。
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // 任意ip地址。
    // servaddr.sin_addr.s_addr = inet_addr("192.168.190.134"); // 指定ip地址。
    servaddr.sin_port = htons(atoi(argv[1])); // 指定通信端口。
    /* 服务端把用于通信的地址和端口号绑定到socket上。成功返回0，错误返回-1。如果绑定的地址错误、或端口已被占用，则会报错，否则一般不会返回错误。*/
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
    {
        perror("bind");
        close(listenfd);
        return -1;
    }

    printf("监听模式开始...\n");
    // 第3步：把socket设置为监听模式。listen函数把主动连接socket变为被动连接的socket，使得这个socket可以接受其他socket连接请求，从而成为一个服务端的socket。socket是已经被bind绑定过的socket，socket函数返回的socket是一个主动连接的socket，在服务端的编程中，程序员希望这个socket可以接收外来请求，也就是被动等待客户端来连接。由于系统默认认为一个socket是主动连接的，所以需要通过某种方式来告诉系统，通过调用listen来完成这件事。第二个参数backlog代表等待队列的最大长度。
    if (listen(listenfd, 5) != 0)
    {
        perror("listen");
        close(listenfd);
        return -1;
    }
    printf("监听模式结束。\n");

    // 第4步：接受客户端的连接。
    int clientfd;                             // 客户端的socket。
    int socklen = sizeof(struct sockaddr_in); // struct sockaddr_in的大小
    struct sockaddr_in clientaddr;            // 客户端的地址信息。
    printf("开始接受监听...\n");  // 当服务器开始运行时，会等待客服端的连接。
    // sleep(10);
    // accept函数用来等待来自客户端的连接请求,虽然出与监听状态时可能已经接收了客户端的请求
    // 	但是还是要等到accpet函数接收了之后，客户端才会发送请求过来。这也应该是写并发程序的切入点。
    /* 这里的代码是为了测试服务器用来接收客户端请求的时候是以何种形式。 */
    /*int count = 2;
    while(count > 0)
    {
    	clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
	printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));
  	count--;
    }*/
    // 等待接收。
    clientfd = accept(listenfd, (struct sockaddr *)&clientaddr, (socklen_t *)&socklen);
    printf("客户端（%s）已连接。\n", inet_ntoa(clientaddr.sin_addr));

    // 第5步：与客户端通信，接收客户端发过来的报文后，回复ok。
    char buffer[1024]; // 这里的buffer只是为了使用它显示的取数据，还不是真正的socket缓冲区。
    while (1)  // 服务端虽然这里是一个循环，但是由于后面的if语句中的recv会判断是否客户端还连接。如果不连接就会终止该程序。
    {
        int iret;
        memset(buffer, 0, sizeof(buffer));
        // 开始接收客户端的报文。
	printf("等待客户端发送消息报文...\n");// 一定要特别注意c语言的输出缓冲区问题。如果没有\n那么有可能就不会立即输出。
	if ((iret = recv(clientfd, buffer, sizeof(buffer), 0)) <= 0) // 接收客户端的请求报文。
        {
            printf("iret=%d\n", iret);
            break;
        }
        printf("接收消息：%s\n", buffer);
	if(strcmp(buffer, "Bye") == 0 )
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
    // 第6步：关闭s
    // ocket，释放资源。
    close(listenfd);
    close(clientfd);
}
