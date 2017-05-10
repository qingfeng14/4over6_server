//
// Created by dalaoshe on 17-4-13.
//
#include "4over6_util.h"
void User_Info::decCount() {
    pthread_mutex_lock(&this->mutex);
    this->count --;
    pthread_mutex_unlock(&this->mutex);
}
void User_Info::incCount() {
    pthread_mutex_lock(&this->mutex);
    this->count ++;
    pthread_mutex_unlock(&this->mutex);
}

void User_Info::freeResource() {
    pthread_mutex_lock(&this->mutex);
    this->count = MAX_COUNTER;
    this->state = FREE;
    this->secs = 0;
    this->fd = -1;
    memset(&this->addr_v6,0,sizeof(this->addr_v6));
    pthread_mutex_unlock(&this->mutex);
}

bool User_Info::needSendKeepAlive() {
    pthread_mutex_lock(&this->mutex);
    bool need = false;
    fprintf(stderr, "now count is %d\n",this->count);
    if(this->count == 0)
        need = true;
    pthread_mutex_unlock(&this->mutex);
    return need;
}

void User_Info::resetCount() {
    pthread_mutex_lock(&this->mutex);
    this->count = MAX_COUNTER;
    pthread_mutex_unlock(&this->mutex);
}

void User_Info::setUserInfo(int fd, struct in6_addr addr_v6) {
    pthread_mutex_lock(&this->mutex);
    this->fd = fd;
    this->addr_v6 = addr_v6;
    this->state = USED;
    this->count = MAX_COUNTER;
    this->secs = time(NULL);
    pthread_mutex_unlock(&this->mutex);
}

bool User_Info::isTimeOut() {
    pthread_mutex_lock(&this->mutex);
    bool need = false;
    time_t now = time(NULL);
    if(now - this->secs >= MAX_TIME_DUR)
        need = true;
    pthread_mutex_unlock(&this->mutex);
    return need;
}

void User_Info::setLatestTime() {
    pthread_mutex_lock(&this->mutex);
    this->secs = time(NULL);
    pthread_mutex_unlock(&this->mutex);

}

void User_Tables::init_ipv4_pool(in_addr start, in_addr end) {
    uint32_t s = ntohl(start.s_addr), e = ntohl(end.s_addr);
    for(uint32_t i = s; i <= e ; ++i) {
        User_Info* info = new User_Info();
        info->addr_v4.s_addr = htonl(i);
        info->count = MAX_COUNTER;
        info->fd = -1;
        info->secs = 0;
        info->state = FREE;
        info->mutex = PTHREAD_MUTEX_INITIALIZER;
        this->v4_map_info.insert(pair<in_addr_t ,User_Info*>(htonl(i), info));
    }
    this->fd_map_mutex = PTHREAD_MUTEX_INITIALIZER;
    this->pool_size = e - s + 1;
    this->ipv4_used = 0;
}

User_Info* User_Tables::get_free_v4_addr() {
    pthread_mutex_lock(&this->fd_map_mutex);

    map<in_addr_t ,User_Info*>::iterator it;
    it = this->v4_map_info.begin();
    for(it ; it != v4_map_info.end(); ++it) {
        if(it->second->state == FREE) {
            pthread_mutex_unlock(&this->fd_map_mutex);
            return it->second;
        }
    }
    pthread_mutex_unlock(&this->fd_map_mutex);

    return NULL;
}

User_Info* User_Tables::get_user_info_by_v4(in_addr_t ipv4) {
    pthread_mutex_lock(&this->fd_map_mutex);

    map<in_addr_t ,User_Info*>::iterator it;
    it = this->v4_map_info.find(ipv4);
    if(it != this->v4_map_info.end()) {
        pthread_mutex_unlock(&this->fd_map_mutex);
        return it->second;
    }

    pthread_mutex_unlock(&this->fd_map_mutex);
    return NULL;
}

User_Info* User_Tables::get_user_info_by_fd(int fd) {
    pthread_mutex_lock(&this->fd_map_mutex);

    map<int ,User_Info*>::iterator it;
    it = this->fd_map_info.find(fd);
    if(it != this->fd_map_info.end()) {
        pthread_mutex_unlock(&this->fd_map_mutex);
        return it->second;
    }

    pthread_mutex_unlock(&this->fd_map_mutex);
    return NULL;
}

void User_Tables::set_fd_info_map(int fd, User_Info * info) {
    pthread_mutex_lock(&this->fd_map_mutex);

    this->fd_map_info.insert(pair<int, User_Info*>(fd,info));

    pthread_mutex_unlock(&this->fd_map_mutex);
}

void User_Tables::release_fd_info_map(int fd) {

    map<int ,User_Info*>::iterator it;
    it = this->fd_map_info.find(fd);
    if(it != this->fd_map_info.end()) {
        this->fd_map_info.erase(it);
    }


    return;
}

void User_Info::mutex_write_FD( char* buf, ssize_t nbyte) {
    pthread_mutex_lock(&this->mutex);
    Write_nByte(this->fd,buf,nbyte);
    pthread_mutex_unlock(&this->mutex);
}

void User_Tables::free_resource_of_fd(int fd) {
    User_Info* info = this->get_user_info_by_fd(fd);
    pthread_mutex_lock(&this->fd_map_mutex);
    if(info == NULL || info->fd == -1) {
        pthread_mutex_unlock(&this->fd_map_mutex);
        return;
    }
    char buf[512];
    Inet_ntop(AF_INET6, &info->addr_v6, buf, sizeof(info->addr_v6) + 1 );

    info->freeResource();

    this->release_fd_info_map(fd);
    fprintf(stderr,"release source of %s , fd %d ",buf, info->fd);
    pthread_mutex_unlock(&this->fd_map_mutex);
}

void set_FD_SET(fd_set* set, int fd, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    FD_SET(fd, set);
    pthread_mutex_unlock(mutex);
}
void clr_FD_SET(fd_set* set, int fd, pthread_mutex_t* mutex) {
    pthread_mutex_lock(mutex);
    FD_CLR(fd,set);
    pthread_mutex_unlock(mutex);
}