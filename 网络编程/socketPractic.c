#include <stdio.h>
#include <sys/types.h>
/* 函数socket和connect都在socket.h文件中。*/
#include <sys/socket.h>
int main(void)
{
    /* socket函数的使用。若成功则为非负描述符，若出错则为-1。*/
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    printf("%d\n", clientfd);
    /* connect函数的使用。*/
    return 0;
}
