#include <iostream>
#include <unistd.h>
#include <string.h>
#include <functional>
#include "src/util.h"
#include "src/Buffer.h"
#include "src/InetAddress.h"
#include "src/Socket.h"
#include "src/ThreadPool.h"

void oneClient(int msgs, int wait) {
    Socket* sock = new Socket();
    InetAddress* addr = new InetAddress("127.0.0.1", 8888);
    sock->connect(addr);

    int sockfd = sock->getFd();

    Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();

    sleep(wait);
    int count = 0;
    while(count < msgs) {
        sendBuffer->setBuf("I'm client!");
        ssize_t bytes_write = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if(bytes_write == -1) {
            printf("socket already disconnected, can't write any more!\n");
            break;
        }
        int already_read = 0;
        char buf[1024];
        while(true) {
            bzero(&buf, sizeof(buf));
            ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
            if(bytes_read > 0) {
                readBuffer->append(buf, bytes_read);
                already_read += bytes_read;
            } else if(bytes_read == 0) {
                printf("server disconnected!\n");
                exit(EXIT_SUCCESS);
            }

            if(already_read >= sendBuffer->size()) {
                printf("count: %d, message from server: %s\n", count++, readBuffer->c_str());
                break;
            }
        }
        readBuffer->clear();
    }
    delete addr;
    delete sock;
}

int main(int argc, char* argv[]) {
    int threads = 100;
    int msgs = 100;
    int wait = 0;
    int o;
    const char* optstring = "t:m:w:";

    while((o = getopt(argc, argv, optstring)) != -1) {
        switch (o)
        {
        case 't':
            threads = std::stoi(optarg);
            break;
        case 'm':
            msgs = std::stoi(optarg);
            break;
        case 'w':
            wait = std::stoi(optarg);
            break;
        case '?':
            printf("error optopt: %c\n", optopt);
            break;
        }
    }

    ThreadPool* pool = new ThreadPool(threads);
    std::function<void()> func = std::bind(oneClient, msgs, wait);
    for(int i = 0; i < threads; i++) {
        pool->add(func);
    }
    
    delete pool;
    return 0;
}