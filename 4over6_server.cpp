//
// Created by dalaoshe on 17-4-13.
//
#include "4over6_util.h"
#define MAX_BACKLOG 20

void do_response(int fd);
void do_server() {
    struct sockaddr_in6 server_addr, client_addr;
    int i, maxi, maxfd, connfd, sockfd;
    int nready, client[FD_SETSIZE];
    fd_set rset, allset;

    socklen_t client_addr_len = sizeof(struct sockaddr_in6);

    int listenfd_6 = Socket(AF_INET6, SOCK_STREAM, 0);


    int on = 1;
    SetSocket(listenfd_6, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(listenfd_6, (SA*)&server_addr, sizeof(struct sockaddr_in6));
    Listen(listenfd_6, MAX_BACKLOG);

    maxfd = listenfd_6;
    maxi = -1;

    for(i = 0; i < FD_SETSIZE; ++i) client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd_6, &allset);

    while (1) {
        rset = allset;
        nready = Select(maxfd+1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(listenfd_6, &rset)) {// 接收一个新的连接
            connfd = Accept(listenfd_6, (SA*)&client_addr, &client_addr_len);
            for(i = 0; i < FD_SETSIZE; ++i) {
                if(client[i] == -1) {
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE) {
                fprintf(stderr, "too many\n");
            }
            FD_SET(connfd, &allset);
            if(connfd > maxfd) maxfd = connfd;
            if(i > maxi) maxi = i;
            if(--nready <= 0)continue;
        }

        for(i = 0 ; i <= maxi; ++i) {
            if((sockfd = client[i]) < 0) {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) { // 该socket有客户请求到达
                do_response(sockfd);
                if(--nready <= 0)break;
            }
        }
    }
}

void do_response(int fd) {
    ssize_t n;
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    ssize_t needbs = sizeof(struct Msg_Hdr);
    n = read(fd, &msg, sizeof(struct Msg_Hdr));
    if(n < 0) {
        fprintf(stderr, "read sockfd %d error: %s \n",fd,strerror(errno));
    }
    if(n == 0) {
        Close(fd);
    }
    else if(n == sizeof(struct Msg_Hdr)){
        process_payload:
        char* ipv4_payload = msg.ipv4_payload;

        if(msg.hdr.type != 100 && msg.hdr.type != 104) {
            n = read(fd, ipv4_payload, msg.hdr.length);
            if(n != msg.hdr.length) {
                fprintf(stderr, "read payload error, need %d byte, read %d byte\n",msg.hdr.length, n);
                if(n <= 0)
                   return;
            }
            while(n < msg.hdr.length)
                n += read(fd, ipv4_payload+n, msg.hdr.length-n);
        }

        switch(msg.hdr.type){
            case 100:
                break;
            case 101:
                break;
            case 102:
                break;
            case 103:
                break;
            case 104:
                break;
        }
    }
    else {// 读到长度小于头长度说明可能出错(也有可能粘包,继续读取)
        while (n < sizeof(struct Msg_Hdr))
            n += read(fd, (char*)&msg + n , sizeof(struct Msg_Hdr)-n);
        goto process_payload;
    }
}