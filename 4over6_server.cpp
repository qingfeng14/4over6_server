//
// Created by dalaoshe on 17-4-13.
//
#include "4over6_util.h"
#define MAX_BACKLOG 20
static struct User_Tables user_tables;

int do_response(int fd, int rawfd, int i, struct sockaddr_in6 *client_addr, socklen_t *len);
void reply_ipv4_request(int fd, sockaddr_in6* client_addr, socklen_t* clientlen);
void do_ipv4_packet_request(int fd, int rawfd, struct Msg* msg);
void do_keep_alive(int fd);

static struct sockaddr_in6 server_addr, client_addr;
static int i, maxi, maxfd, connfd, sockfd, raw_udp_fd, raw_tcp_fd, raw_out_fd;
static int nready, client[FD_SETSIZE];
static fd_set rset, allset;
pthread_mutex_t allset_mutex;

void process_packet(char* buffer , uint32_t size)
{
    static struct Msg msg;
    struct iphdr *iph = (struct iphdr*)buffer;
    User_Info* info = user_tables.get_user_info_by_v4(iph->daddr);
    char buf[16];
    Inet_ntop(AF_INET, &iph->daddr, buf,sizeof(buf));
    if(iph->protocol == 6)
        fprintf(stderr,"recev tcp packet should sent to ip_v4: %s \n", buf);
    else
        fprintf(stderr,"recev udp packet should sent to ip_v4: %s \n", buf);
    if(info == NULL || info->state == FREE)return;




    memset(&msg, 0, sizeof(struct Msg));
    msg.hdr.type = 103;
    msg.hdr.length = size;
    int fd = info->fd;
    if(fd != -1)
        Write_nByte(fd, (char*)&msg, sizeof(struct Msg_Hdr) + msg.hdr.length);

}

void do_server() {


    memset(&server_addr, 0, sizeof(struct sockaddr_in6));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERV_PORT);
    server_addr.sin6_addr = in6addr_any;

    socklen_t client_addr_len = sizeof(struct sockaddr_in6);


    int listenfd_6 = Socket(AF_INET6, SOCK_STREAM, 0);


    int on = 1;
    SetSocket(listenfd_6, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    Bind_Socket(listenfd_6, (SA*)&server_addr, sizeof(struct sockaddr_in6));
    Listen(listenfd_6, MAX_BACKLOG);

    int raw_tcp_fd = Socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    int raw_udp_fd = Socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    int raw_out_fd = Socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    SetSocket(raw_udp_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    SetSocket(raw_tcp_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    SetSocket(raw_out_fd, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
    struct sockaddr addr;
    char buf[65536];
    int datasize;
    socklen_t saddr_size =  sizeof(addr);


    maxfd = listenfd_6;
    maxi = -1;

    for(i = 0; i < FD_SETSIZE; ++i) client[i] = -1;
    FD_ZERO(&allset);
    FD_SET(listenfd_6, &allset);
    FD_SET(raw_tcp_fd, &allset);
    FD_SET(raw_udp_fd, &allset);

    in_addr star;
    in_addr end;
    Inet_pton(AF_INET,"192.168.111.2",&star);
    Inet_pton(AF_INET,"192.168.111.254",&end);
    user_tables.init_ipv4_pool(star, end);

    keep_alive_thread_argv argv;
    argv.allset = &allset;
    argv.client = client;
    argv.table = &user_tables;
    pthread_t keep_alive;
    pthread_create(&keep_alive, NULL, &keep_alive_thread, (void*)&argv);


    while (1) {
        rset = allset;
        nready = Select(maxfd+1, &rset, NULL, NULL, NULL);
        if(FD_ISSET(listenfd_6, &rset)) {// 接收一个新的连接
            connfd = Accept(listenfd_6, (SA*)&client_addr, &client_addr_len);
            char ipv6[512];
            Inet_ntop(AF_INET6, &client_addr.sin6_addr, ipv6, sizeof(client_addr.sin6_addr) + 1 );
            fprintf(stderr, "a client from IP:%s,port %d,socket %d\n",ipv6,client_addr.sin6_port,connfd);
            for(i = 0; i < FD_SETSIZE; ++i) {
                if(client[i] == -1) {
                    client[i] = connfd;
                    break;
                }
            }
            if(i == FD_SETSIZE) {
                fprintf(stderr, "too many\n");
            }

            set_FD_SET(&allset, connfd, &allset_mutex);

            if(connfd > maxfd) maxfd = connfd;
            if(i > maxi) maxi = i;
            if(--nready <= 0)continue;
        }

        if(FD_ISSET(raw_tcp_fd, &rset)) {
            memset(buf, 0, sizeof(buf));
            datasize = recvfrom(raw_tcp_fd, buf, 65536,0 ,&addr,&saddr_size );
            process_packet(buf, datasize);
        }
        if(FD_ISSET(raw_udp_fd, &rset)) {
            memset(buf, 0, sizeof(buf));
            datasize = recvfrom(raw_udp_fd, buf, 65536,0 ,&addr,&saddr_size );
            process_packet(buf, datasize);
        }

        for(i = 0 ; i <= maxi; ++i) {

            if((sockfd = client[i]) < 0) {
                continue;
            }
            if(FD_ISSET(sockfd, &rset)) { // 该socket有客户请求到达
                fprintf(stderr, "recv a request from fd %d\n", sockfd);
                memset(&client_addr, 0, sizeof(client_addr));
                socklen_t  len = sizeof(client_addr);
                Getpeername(sockfd, (SA*)&client_addr, &len);
                int result = do_response(sockfd, raw_out_fd, i, &client_addr, &len);

                char ipv6[512];

                Inet_ntop(AF_INET6, &client_addr.sin6_addr, ipv6, sizeof(client_addr.sin6_addr) + 1 );
                fprintf(stderr, "recv a request from  IP:%s,port %d,socket %d\n",ipv6,client_addr.sin6_port,connfd);

                if(result < 0) {//
                    clr_FD_SET(&allset, sockfd, &allset_mutex);
                    client[i] = -1;
                    fprintf(stderr, "close client \n");

                    user_tables.free_resource_of_fd(sockfd);
                    fprintf(stderr,"close ok\n");
                }
                if(--nready <= 0)break;
            }
        }
    }
}

int do_response(int fd, int rawfd, int i, struct sockaddr_in6 *client_addr, socklen_t *len) {
    ssize_t n;
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    ssize_t needbs = sizeof(struct Msg_Hdr);

    //n = recvfrom(fd, &msg, sizeof(struct Msg_Hdr),0, (SA*)client_addr, len);
    n = read(fd, &msg, sizeof(struct Msg_Hdr));
    if(n < 0) {
        fprintf(stderr, "read sockfd %d error: %s \n",fd,strerror(errno));
        close(fd);
        return -1;
    }
    else if(n == 0) {
        fprintf(stderr, "close sockfd %d \n",fd);
        Close(fd);
        return -1;
    }
    else if(n == sizeof(struct Msg_Hdr)){
        process_payload:
        char* ipv4_payload = msg.ipv4_payload;

        if(msg.hdr.type != 100 && msg.hdr.type != 104) {
            n = read(fd, ipv4_payload, msg.hdr.length);
            if(n != msg.hdr.length) {
                fprintf(stderr, "read payload error, need %d byte, read %d byte\n",msg.hdr.length, n);
                if(n <= 0) {
                    Close(fd);
                    return -1;
                }
            }
            while(n < msg.hdr.length)
                n += read(fd, ipv4_payload+n, msg.hdr.length-n);
        }

        switch(msg.hdr.type){
            case 100:
                fprintf(stderr, "recv an 100 reqeust\n");
                reply_ipv4_request(fd, client_addr, len);
                fprintf(stderr, "reply an 100 reqeust over\n");
                break;
            case 101:
                break;
            case 102:
                do_ipv4_packet_request(fd, rawfd, &msg);
                break;
            case 103:
                break;
            case 104:
                do_keep_alive(fd);
                break;
        }
    }
    else {// 读到长度小于头长度说明可能出错(也有可能粘包,继续读取)
        while (n < sizeof(struct Msg_Hdr))
            n += read(fd, (char*)&msg + n , sizeof(struct Msg_Hdr)-n);
        goto process_payload;
    }
    return 0;
}

void reply_ipv4_request(int fd, sockaddr_in6* client_addr, socklen_t* clientlen) {
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    User_Info* info = user_tables.get_free_v4_addr();
    if(info == NULL) {
        return;
    }
    fprintf(stderr,"set user info\n");
    //设置连接信息
    info->setUserInfo(fd,client_addr->sin6_addr);
    fprintf(stderr,"set user info over\n");
    user_tables.set_fd_info_map(fd,info);
    fprintf(stderr,"set user table info over\n");

    msg.hdr.type = 101;
    Ipv4_Request_Reply *payload = (Ipv4_Request_Reply*)msg.ipv4_payload;
    payload->addr_v4[0] = info->addr_v4;
    Inet_pton(AF_INET,"0.0.0.0",&(payload->addr_v4[1])); // 注意得到的字节序
    Inet_pton(AF_INET,"8.8.8.8",&(payload->addr_v4[2]));
    Inet_pton(AF_INET,"202.38.120.242",&(payload->addr_v4[3]));
    Inet_pton(AF_INET,"202.106.0.20",&(payload->addr_v4[4]));
    msg.hdr.length = sizeof(struct Ipv4_Request_Reply);

    ssize_t n = write(fd, &msg, sizeof(struct Msg_Hdr)+msg.hdr.length);

    if(n != sizeof(struct Msg_Hdr)+msg.hdr.length) {
        fprintf(stderr, "write reply error, need %d byte, write %d byte\n",sizeof(struct Msg_Hdr)+msg.hdr.length, n);
        if(n <= 0)
            return;
    }
}

void do_ipv4_packet_request(int fd, int rawfd, struct Msg* c_msg) {
    iphdr* ipv4hdr = (iphdr*)(c_msg->ipv4_payload);
    User_Info* info = user_tables.get_user_info_by_fd(fd);
    if(info == NULL) {
        return;
    }
    //
    info->setLatestTime();
    fprintf(stderr,"get a ipv4 request\n");
    //获取目的地址

    struct sockaddr_in dstaddr;
    dstaddr.sin_addr.s_addr = ipv4hdr->daddr;
    dstaddr.sin_family = AF_INET;
    socklen_t addr_len = sizeof(struct sockaddr_in);


    ssize_t n = sendto(rawfd, c_msg->ipv4_payload, c_msg->hdr.length, 0, (SA*)&dstaddr, addr_len);

    if(n != c_msg->hdr.length) {
        fprintf(stderr, "write reply error, need %d byte, write %d byte\n",c_msg->hdr.length, n);
        if(n <= 0)
            return;
    }
}

void do_keep_alive(int fd) {
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    User_Info* info = user_tables.get_user_info_by_fd(fd);
    if(info == NULL) {
        return;
    }
    info->setLatestTime();
    return;
}