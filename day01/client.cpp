#include <sys/socket.h>
#include <arpa/inet.h>
#include <cstring>

int main() {

    // 1. 创建tcp socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. 创建要连接的服务器地址信息
    sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(8888);

    // 3. 使用connect连接到服务器
    connect(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr));


    return 0;
}