#include <sys/socket.h>
#include <arpa/inet.h> // 这个头文件包含了netinet/in.h，不用再次包含
#include <cstring> // 使用到了bzero函数
#include <stdio.h> // 使用到了printf函数
#include "util.h"

int main() {

    // 1. 创建socket
    // 第一个参数: IP地址类型，AF_INET表示使用IPv4，如果使用IPv6请使用AF_INET6
    // 第二个参数: 数据传输方式，SOCK_STREAM表示流式传输、面向连接、多用于TCP。
    //                           SOCK_DGRAM表示数据报格式、无连接，多用于UDP。
    // 第三个参数：协议，0表示根据前面的两个参数自动推导协议类型。设置为IPPROTO_TCP和IPPROTO_UDP，分别表示TCP和UDP。
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    // 2. 创建服务端绑定的ip地址和端口
    sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    // 设置协议族、IP地址和端口
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serv_addr.sin_port = htons(8888);

    // 3. 使用bind绑定到创建的地址信息上
    errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    // Q&A
    // 为什么定义的时候用专用的socket地址（sockaddr_in)而绑定的时候要转化为socket地址（sockaddr），以及转换ip地址（inet_addr）和端口（htons）为网络字节序等
    // 函数及其必要性，在游双《Linux高性能服务器编程》第五章第一节：socket地址API中有详细讨论。

    // 4. 使用listen监听端口，等待socket创建连接（半连接队列）
    // 第二个参数：最大监听队列长度，系统建议的最大值SOMAXCONN被定义为128
    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    // 5. 使用accept接收一个客户端的连接（从内核全连接队列取出一个连接）
    // 对每个客户端，在接收连接时也需要保存客户端的socket地址信息，于是有以下代码
    sockaddr_in clnt_addr;
    socklen_t clnt_addr_len = sizeof(clnt_addr);
    bzero(&clnt_addr, sizeof(clnt_addr));
    int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_addr, &clnt_addr_len);
    errif(clnt_sockfd == -1, "socket accept error");
    printf("new client fd connected: %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

    // 要注意的是：accept和bind的第三个参数有点区别，对于bind只需要传入serv_addr的大小即可，而accept需要写入客户端socket长度，
    // 所以需要定义一个类型为socklen_t的变量，并传入该变量的地址。
    // 另外accept函数会阻塞当前程序，知道有一个socket客户端被接收后程序才会往下执行。

    // 到这里，客户端就可以通过ip和端口连接这个服务器了。

    return 0;
}