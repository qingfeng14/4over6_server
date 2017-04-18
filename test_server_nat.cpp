//
// Created by dalaoshe on 17-4-18.
//

#include "test_server_nat.h"
void do_test_server() {
    struct sockaddr_in server_addr, client_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(33675);
    Inet_pton(AF_INET, "59.66.134.64", &server_addr.sin_addr.s_addr);

    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(6666);
    Inet_pton(AF_INET, "59.66.134.64", &client_addr.sin_addr.s_addr);

    socklen_t len = sizeof(struct sockaddr_in);


    int sockfd = Socket(AF_INET, SOCK_DGRAM, 0);
    Bind_Socket(sockfd, (SA*)&client_addr, len);

    char buf[] = "this is an test\n";
    //Socket_Peer_Connect(sockfd, (SA*)&server_addr, len);
    int n =sendto(sockfd, buf, sizeof(buf), 0, (SA*)&server_addr, len);
    if (n < 0) {
        fprintf(stderr, "error : %s\n",strerror(errno));
    }
    fprintf(stderr, "write byte %d\n",n);
//    Write_nByte(sockfd, buf, sizeof(buf));
}