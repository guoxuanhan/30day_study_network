#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "src/util.h"
#include "src/Buffer.h"
#include "src/InetAddress.h"
#include "src/Socket.h"

int main() {

    Socket* sock = new Socket();
    InetAddress* addr = new InetAddress("127.0.0.1", 8888);
    sock->connect(addr);

    int sockfd = sock->getFd();

    Buffer* sendBuffer = new Buffer();
    Buffer* readBuffer = new Buffer();

    while(true) {
        sendBuffer->getline();
        ssize_t bytes_write = write(sockfd, sendBuffer->c_str(), sendBuffer->size());
        if(bytes_write == -1) {
            printf("socket already disconnected, can't write any more\n");
            break;
        }

        int already_read = 0;
        char buf[1024]; // 这个buf大小无所谓
        while (true)
        {
            bzero(&buf, sizeof(buf));
            ssize_t bytes_read = read(sockfd, buf, sizeof(buf));
            if(bytes_read > 0) {
                readBuffer->append(buf, bytes_read);
                already_read += bytes_read;
            } else if(bytes_read == 0) { // EOF
                printf("server closed!\n");
                exit(EXIT_SUCCESS);
            }

            if(already_read >= sendBuffer->size()) {
                printf("message from server: %s\n", readBuffer->c_str());
                break;
            }
        }
        readBuffer->clear();
    }

    delete addr;
    delete sock;
    return 0;
}