//
// Created by dalaoshe on 17-4-14.
//

#include "4over6_util.h"

in_addr addr_v4[5];
void request_ipv4(int fd);
void do_client() {
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in6));
//    bzero(&server_addr.sin6_addr, sizeof(in6_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERV_PORT);
    server_addr.sin6_addr = in6addr_any;

    socklen_t len = sizeof(struct sockaddr_in6);


    int sockfd = Socket(AF_INET6, SOCK_STREAM, 0);

    Socket_Peer_Connect(sockfd, (SA*)&server_addr, len);

    request_ipv4(sockfd);
    while (1){
        static struct Msg msg;
        memset(&msg, 0, sizeof(struct Msg));
        ssize_t needbs = sizeof(struct Msg_Hdr);
        ssize_t n = read(sockfd, &msg, needbs);
        if(msg.hdr.type == 104) {
            fprintf(stderr," client read an 104 packet\n");
            memset(&msg, 0, sizeof(struct Msg));
            msg.hdr.type = 104;
            msg.hdr.length = 0;
            fprintf(stderr,"------- client send a keep alive to server  ----------\n");
//    ssize_t n = write(fd, &msg, sizeof(struct Msg_Hdr)+msg.hdr.length);
            Write_nByte(sockfd, (char*)&msg, sizeof(struct Msg_Hdr)+msg.hdr.length);

        }
    }
}

void dns_request() {

}

void request_ipv4(int fd) {
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    ssize_t needbs = sizeof(struct Msg_Hdr);
    msg.hdr.length = 0;
    msg.hdr.type = 100;

    ssize_t n = write(fd, &msg, needbs);
    if(n < 0) {
        fprintf(stderr,"client write 100 request error: %s\n",strerror(errno));
    }
    if(n != needbs) {
        fprintf(stderr,"client write 100 request error, need %d, write %d \n",needbs, n);
    }
    else {
        fprintf(stderr,"send 100 request success\n");
    }
    memset(&msg, 0, sizeof(struct Msg));
    n = read(fd, &msg, needbs);
    if(n < 0) {
        fprintf(stderr,"client read 101 reply hdr error: %s\n",strerror(errno));
    }
    else if(n != needbs) {
        fprintf(stderr,"client read 101 reply hdr error, need %d, write %d \n",needbs, n);
    }
    else {
        if(msg.hdr.type == 101) {
            n = read(fd, msg.ipv4_payload, msg.hdr.length);
            if(n < 0) {
                fprintf(stderr,"client read 101 reply payload error: %s\n",strerror(errno));
            }
            else if(n != msg.hdr.length) {
                fprintf(stderr,"client read 101 reply payload error, need %d, write %d \n",msg.hdr.length, n);
            }
            else {
                Ipv4_Request_Reply* reply = (Ipv4_Request_Reply*)msg.ipv4_payload;
                for(int i = 0 ; i < 5; ++i) {
                    char buf[16];
                    Inet_ntop(AF_INET, &(reply->addr_v4[i]), buf,sizeof(buf));
                    fprintf(stderr,"recev %d , ip_v4: %s \n",i, buf);
                }
                for(int i = 0 ; i < 3; ++i)
                    addr_v4[i] = reply->addr_v4[i+2];
            }
        }

    }

}
