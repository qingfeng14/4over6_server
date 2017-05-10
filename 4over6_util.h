//
// Created by dalaoshe on 17-4-13.
//

#ifndef INC_4OVER6_4OVER6_UTIL_H
#define INC_4OVER6_4OVER6_UTIL_H

#include "unp.h"
#include <pthread.h>
#include <map>

#define MAX_IPV4_PAYLOAD 1024
#define MAX_COUNTER 10
#define MAX_TIME_DUR 30
#define USED 1
#define FREE 0
/*
 *       ipv6_hdr | tcp_hdr | tcp_payload[ Msg_Hdr[length | type] | Msg[ipv4_payload] ]
 */
struct Msg_Hdr {
    uint32_t length; // payload 长度,不包括type, 注意协议切割
    char type; //
};

struct Msg{
    struct Msg_Hdr hdr;
    uint8_t ipv4_payload[MAX_IPV4_PAYLOAD];
};

struct Ipv4_Request_Reply{
    struct in_addr addr_v4[5];
};

struct User_Info {
    int fd;
    int i;
    int count;
    int state; //
    unsigned long int  secs;
    struct in_addr addr_v4;// 网络序
    struct in6_addr addr_v6;// 网络序
    pthread_mutex_t mutex;
    void incCount();
    void decCount();
    bool isTimeOut();
    bool needSendKeepAlive();
    void setLatestTime();
    void setUserInfo(int fd, struct in6_addr addr_v6);
    void freeResource();
    void resetCount();
    void mutex_write_FD( char* buf, ssize_t nbyte);
};

using namespace std;
//typedef char[16] v6_byte

static bool operator<(in6_addr a,in6_addr b)
{
    for(int i = 0 ; i < 16 ; ++i) {
        if(a.__in6_u.__u6_addr8[i] < b.__in6_u.__u6_addr8[i]) {
            return true;
        }
    }
    return false;
}

struct User_Tables {
    uint32_t pool_size;
    uint32_t ipv4_used;
    map<in_addr_t , User_Info*> v4_map_info;// in_addr_t 为网络序
    map<int , User_Info*> fd_map_info;// in_addr_t 为网络序
    pthread_mutex_t fd_map_mutex;

    void init_ipv4_pool(in_addr start, in_addr end);
    User_Info* get_free_v4_addr();
    User_Info* get_user_info_by_v4(in_addr_t ipv4);
    User_Info* get_user_info_by_fd(int fd);

    void set_fd_info_map(int fd, User_Info* info);
    void release_fd_info_map(int fd);
    void free_resource_of_fd(int fd);
};

struct GlobalRecord {
    unsigned long long packet_number;
    unsigned long long GBs;
    unsigned long long MBs;
    unsigned long long KBs;
    unsigned long long Bs;
    void update() {
        if((Bs >> 10) > 0) {
            KBs += (Bs >> 10);
            Bs = Bs % 1024;
            if((KBs >> 10) > 0) {
                MBs += (KBs >> 10);
                KBs = KBs % 1024;
                if((MBs >> 10) > 0) {
                    GBs += (MBs >> 10);
                    MBs = MBs % 1024;
                }
            }
        }
    }
};

struct keep_alive_thread_argv {
    User_Tables* table;
    fd_set* allset;
    int*    client;
};

extern pthread_mutex_t allset_mutex;
void set_FD_SET(fd_set* set, int fd, pthread_mutex_t* mutex);
void clr_FD_SET(fd_set* set, int fd, pthread_mutex_t* mutex);


void do_client();
void do_server();
void* keep_alive_thread(void* argv);

//test
void process_packet(unsigned char* buffer , int size);
void print_ip_header(unsigned char* , int);
void print_tcp_packet(unsigned char* , int);
void print_udp_packet(unsigned char * , int);
void print_icmp_packet(unsigned char* , int);
void PrintData (unsigned char* , int);
void sendKeepAlive(User_Info* info);
#endif //INC_4OVER6_4OVER6_UTIL_H
