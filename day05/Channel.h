#pragma once
#include <sys/epoll.h>

class Epoll;
class Channel {
private:
    Epoll* ep;  // 每个socket fd会被分发到一个Epoll类，用ep指向
    int fd; // Channel类负责的fd
    uint32_t events; // 表示希望监听这个fd的哪些事件，不同的事件处理方式不同
    uint32_t revents;// 表示在epoll返回该Channel时fd正在发生的事件
    bool inEpoll; // 表示当前Channel是否已经在epoll红黑树中，为了注册Channel时方便区分EPOLL_CTL_ADD还是EPOLL_CTL_MOD
public:
    Channel(Epoll* _ep, int _fd);
    ~Channel();

    void enableReading();
    int getFd();
    uint32_t getEvents();
    uint32_t getRevents();
    bool getInEpoll();
    void setInEpoll();

    void setRevents(uint32_t);
};