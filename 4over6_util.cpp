//
// Created by dalaoshe on 17-4-13.
//
#include "4over6_util.h"

void User_Tables::init_ipv4_pool(in_addr start, in_addr end) {
    uint32_t s = ntohl(start.s_addr), e = ntohl(end.s_addr);
    for(uint32_t i = s; i <= e ; ++i) {
        User_Info* info = new User_Info();
        info->addr_v4.s_addr = htonl(i);
        info->count = -1;
        info->fd = -1;
        info->secs = 0;
        this->v4_map_info.insert(pair<in_addr_t ,User_Info*>(htonl(i), info));
    }
    this->pool_size = e - s + 1;
    this->ipv4_used = 0;
}

User_Info* User_Tables::get_free_v4_addr() {
    map<in_addr_t ,User_Info*>::iterator it;
    it = this->v4_map_info.begin();
    for(it ; it != v4_map_info.end(); ++it) {
        if(it->second->count == -1) return it->second;
    }
    return NULL;
}

User_Info* User_Tables::get_user_info_by_v4(in_addr ipv4) {
    map<in_addr_t ,User_Info*>::iterator it;
    it = this->v4_map_info.find(ipv4.s_addr);
    if(it != this->v4_map_info.end()) {
        return it->second;
    }
    return NULL;
}