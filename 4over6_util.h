//
// Created by dalaoshe on 17-4-13.
//

#ifndef INC_4OVER6_4OVER6_UTIL_H
#define INC_4OVER6_4OVER6_UTIL_H

#include <cstdint>
#include "unp.h"
#include <map>

#define MAX_IPV4_PAYLOAD

/*
 *       ipv6_hdr | tcp_hdr | tcp_payload[ Msg_Hdr[length | type] | Msg[ipv4_payload] ]
 */
struct Msg_Hdr {
    uint32_t length; // payload 长度,不包括type, 注意协议切割
    char type; //
};

struct Msg{
    struct Msg_Hdr hdr;
    char ipv4_payload[MAX_IPV4_PAYLOAD];
};

struct Ipv4_Request_Reply{
    struct in_addr addr_v4[5];
};

struct User_Info {
    int fd;
    int count;
    unsigned long int  secs;
    struct in_addr addr_v4;// 网络序
    struct in6_addr addr_v6;// 网络序
};

using namespace std;
struct User_Tables {
    uint32_t pool_size;
    uint32_t ipv4_used;
    map<in_addr_t , User_Info*> v4_map_info;// in_addr_t 为网络序
    void init_ipv4_pool(in_addr start, in_addr end);
    User_Info* get_free_v4_addr();
    User_Info* get_user_info_by_v4(in_addr_t ipv4);
};

#endif //INC_4OVER6_4OVER6_UTIL_H
