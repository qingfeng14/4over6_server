//
// Created by dalaoshe on 17-4-17.
//

#include "4over6_util.h"

static int  tun_fd;
static in_addr addr_v4[5];
static User_Info* info;
struct in6_addr addr_v6;
int pipe_write_fd, pipe_read_fd;

GlobalRecord globalRecord;
int flag; //

void request_ipv4(int fd);
void connect_server();
void recv_ipv4_addr(Msg* msg);
void do_keep_alive();
void recv_ipv4_packet(Msg* msg);



void init_my_info() {
    info = new User_Info();
    info->addr_v4.s_addr = 0;
    memset(&info->addr_v6, 0, sizeof(info->addr_v6));
    info->count = MAX_COUNTER;
    info->fd = -1;
    info->secs = 0;
    info->state = FREE;
    info->mutex = PTHREAD_MUTEX_INITIALIZER;
}

void create_pipe() {
    const char *fifo_write_name = "/tmp/my_write_fifo";
    const char *fifo_read_name = "/tmp/my_read_fifo";
    int res = 0;
    if(access(fifo_write_name, F_OK) == -1)
    {
        //管道文件不存在
        //创建命名管道
        res = mkfifo(fifo_write_name, 0777);
        if(res != 0)
        {
            fprintf(stderr, "Could not create fifo %s\n", fifo_write_name);
            exit(EXIT_FAILURE);
        }
    }
    if(access(fifo_read_name, F_OK) == -1)
    {
        res = mkfifo(fifo_read_name, 0777);
        if(res != 0)
        {
            fprintf(stderr, "Could not create fifo %s\n", fifo_read_name);
            exit(EXIT_FAILURE);
        }
    }
    //以只写阻塞方式打开FIFO文件，以只读方式打开数据文件
    pipe_write_fd = open(fifo_write_name, O_WRONLY);
    pipe_read_fd = open(fifo_read_name, O_RDONLY);
    printf("Process %d, open write_fd %d, read_fd %d \n", getpid(), pipe_write_fd, pipe_read_fd);
}

void connect_server() {
    struct sockaddr_in6 server_addr, client_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in6));

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERV_PORT);
    Inet_pton(AF_INET6, ":::::::1", &server_addr.sin6_addr);

    client_addr.sin6_family = AF_INET6;
    client_addr.sin6_port = htons(CLIENT_PORT);
    Inet_pton(AF_INET6, ":::::::1", &server_addr.sin6_addr);

    socklen_t len = sizeof(struct sockaddr_in6);

    info->fd = Socket(AF_INET6, SOCK_STREAM, 0);

    Socket_Peer_Connect(info->fd, (SA*)&server_addr, len);

    Bind_Socket(info->fd, (SA*)&client_addr, len);

}

int recv_server() {

    static struct Msg msg;
    static ssize_t n;
    memset(&msg, 0, sizeof(struct Msg));
    n = 0;

    size_t needbs = sizeof(struct Msg_Hdr);
    int fd = info->fd;

    n = read(fd, &msg, needbs);
    if(n < 0) {
        fprintf(stderr, "read sockfd %d error: %s \n",fd,strerror(errno));
        Close(fd);
        return -1;
    }
    else if(n == 0) {
        fprintf(stderr, "recv 0 byte from server, close sockfd %d \n",fd);
        Close(fd);
        return -1;
    }
    else if(n == needbs){
        process_payload:
        uint8_t * ipv4_payload = msg.ipv4_payload;

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
                n += read(fd, ipv4_payload + n, msg.hdr.length-n);
        }

        switch(msg.hdr.type){
            case 101:
                recv_ipv4_addr(&msg);
                break;
            case 103:
                recv_ipv4_packet(&msg);
                break;
            case 104:
                do_keep_alive();
                break;
            default:
                fprintf(stderr, "recv unknown type %d\n",msg.hdr.type);
                return -1;
        }
    }
    else {// 读到长度小于头长度说明可能出错(也有可能粘包,继续读取)
        while (n < needbs)
            n += read(fd, ((char*)&msg) + n , needbs-n);
        goto process_payload;
    }
    return 0;
}


void establish_tun(Msg* msg) {
    fprintf(stderr, "write recv ip info to pipi_write\n");
    Write_nByte(pipe_write_fd, (char*)msg->ipv4_payload, msg->hdr.length);
    fprintf(stderr, "read pipe_read to get a tun_fd\n");
    ssize_t n = read(pipe_read_fd, &tun_fd, sizeof(int));
    if(n == 4)
        fprintf(stderr, "establish tun_fd: %d ok \n",tun_fd);
    if(n < 0) {
        fprintf(stderr, "read pipe_read fail, error: %s\n",strerror(errno));
    }
}
void* client_keep_alive_thread(void* argv) {
    pthread_detach(pthread_self());
    while (flag) {
        sleep(1);
        if(info->state == FREE) continue;
        info->decCount();
        bool needSendKeep = info->needSendKeepAlive();
        if(needSendKeep) {
            sendKeepAlive(info);
            info->resetCount();
        }
        bool timeout = info->isTimeOut();
        if(timeout) {
            int fd = info->fd;
            info->freeResource();
            flag = 0;
            Close(fd);
        }
    }
    return NULL;
}
void recv_ipv4_addr(Msg* msg) {
    Ipv4_Request_Reply* reply = (Ipv4_Request_Reply*)msg->ipv4_payload;
    for(int i = 0 ; i < 5; ++i)
        addr_v4[i] = reply->addr_v4[i];
    for(int i = 0 ; i < 5; ++i) {
        char buf[16];
        Inet_ntop(AF_INET, &(addr_v4[i]), buf,sizeof(buf));
        fprintf(stderr,"recev %d , ip_v4: %s \n",i, buf);
    }
    info->addr_v4 = addr_v4[0];
    info->setUserInfo(info->fd, addr_v6);
    //try to get tun_fd
    establish_tun(msg);
    flag = 1;
    //get ok,
    pthread_t keep_alive;
    pthread_create(&keep_alive, NULL, &client_keep_alive_thread, NULL);
}

void request_ipv4(int fd) {
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    size_t needbs = sizeof(struct Msg_Hdr);
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
}

void do_keep_alive() {
    info->setLatestTime();
    return;
}

void write_tun(char* payload, uint32_t len) {
    if(!flag) {
        fprintf(stderr, "tun is not established, or connect to server is closed\n");
    }
    Write_nByte(tun_fd, payload, len);
}

void recv_ipv4_packet(Msg* msg) {
    globalRecord.packet_number ++;
    globalRecord.Bs += msg->hdr.length;
    globalRecord.update();
    write_tun((char*)msg->ipv4_payload, msg->hdr.length);
}

void do_android_client() {
    create_pipe();
    init_my_info();
    connect_server();
    request_ipv4(info->fd);
    while (1) {
        if(recv_server() < 0 || flag == 0) {
            flag = 0;
            fprintf(stderr,"android back over\n");
            break;
        }
    }
}