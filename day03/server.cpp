#include <sys/socket.h>
#include <arpa/inet.h> // 这个头文件包含了netinet/in.h，不用再次包含
#include <cstring> // 使用到了bzero函数
#include <stdio.h> // 使用到了printf函数
#include <unistd.h> // 使用到了send write函数
#include <sys/epoll.h> // 使用到了epoll
#include <fcntl.h> // 使用到了fcntl函数
#include <errno.h>
#include "util.h"

#define MAX_EVENTS 1024
#define READ_BUFFER 1024

// 设置socket为非阻塞式
void setnonblocking(int fd) {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    errif(sockfd == -1, "socket create error");

    sockaddr_in serv_addr;
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    serv_addr.sin_port = htons(8888);

    errif(bind(sockfd, (sockaddr*)&serv_addr, sizeof(serv_addr)) == -1, "socket bind error");

    errif(listen(sockfd, SOMAXCONN) == -1, "socket listen error");

    // 使用高并发的epoll做io复用
    // 1. 创建epoll文件描述符
    // 第一个参数flag：一般设置为0
    int epfd = epoll_create1(0);
    errif(epfd == -1, "epoll create error");

    // 2. 创建epoll event事件并把服务器socket加入到epoll中
    epoll_event events[MAX_EVENTS], ev;
    bzero(&events, sizeof(events));
    bzero(&ev, sizeof(ev));
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET; // 在代码中使用了ET模式，实际上在接受连接最好不要使用ET模式
    setnonblocking(sockfd); // ET模式需要搭配非阻塞socket使用
    epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev); // 将服务器socket fd添加到epoll中
    
    // 3. 等待事件发生，并处理事件
    while(true) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1); // 有nfds个fd发生事件
        errif(nfds == -1, "epoll wait error");
        for(int i = 0; i < nfds; i++) {
            if(events[i].data.fd == sockfd) { // 发生事件的是服务器socket fd表示有新客户端请求连接
                sockaddr_in clnt_addr;
                socklen_t clnt_addr_len = sizeof(clnt_addr);
                bzero(&clnt_addr, sizeof(clnt_addr));
                int clnt_sockfd = accept(sockfd, (sockaddr*)&clnt_sockfd, &clnt_addr_len);
                errif(clnt_sockfd == -1, "socket accept error");
                printf("new client fd connected: %d! IP: %s Port: %d\n", clnt_sockfd, inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

                bzero(&ev, sizeof(ev));
                ev.data.fd = clnt_sockfd;
                ev.events = EPOLLIN | EPOLLET; // 对于客户端连接，使用ET模式，可以让epoll更加高效，支持更多并发
                setnonblocking(clnt_sockfd); // ET模式需要搭配非阻塞socket使用
                epoll_ctl(epfd, EPOLL_CTL_ADD, clnt_sockfd, &ev); // 将该客户端的socket fd加入epoll
            } else if(events[i].events & EPOLLIN) { // 发生事件的是客户端，并且是可读事件
                char buf[READ_BUFFER];
                while(true) { // 由于使用了ET模式，读取客户端buffer需要一次性读取完毕，下次不会再触发可读事件
                    bzero(&buf, sizeof(buf));
                    ssize_t bytes_read = read(events[i].data.fd, buf, sizeof(buf));
                    if(bytes_read > 0) {
                        printf("message from client fd %d: %s\n", events[i].data.fd, buf);
                        write(events[i].data.fd, buf, sizeof(buf));// 将相同的数据写回客户端
                    } else if(bytes_read == -1) {
                        if(errno == EINTR) { // 客户端正常中断，继续读取
                            printf("message from client fd %d: %s", events[i].data.fd, buf);
                            write(events[i].data.fd, buf, sizeof(buf));
                        } else if(errno == EAGAIN || errno == EWOULDBLOCK) { // 非阻塞IO，这个条件表示数据全部读取完毕
                            printf("finish reading once, errno: %d\n", errno);
                            break;
                        } 
                    } else if(bytes_read == 0) { // EOF，客户端主动断开连接
                        printf("EOF, client fd %d disconnected\n", events[i].data.fd);
                        close(events[i].data.fd);
                        break;
                    }
                } 
            } else { // 其他事件，之后的版本实现
                printf("something else happened\n");
            }
        }
    }

    close(sockfd);
    return 0;
}