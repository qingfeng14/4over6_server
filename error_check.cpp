//
// Created by dalaoshe on 17-3-22.
//

#include "unp.h"

int Socket(int __domain, int __type, int __protocol) {
    int sockfd = socket(__domain, __type, __protocol);
    if(sockfd < 0) {
        fprintf(stderr, "init socket error: %s \n", strerror(errno));
       // exit(0);
    }
    return sockfd;
}

int Socket_Peer_Connect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) {
    int result = connect(__fd, __addr, __len);
    if(result < 0) {
        fprintf(stderr, "connect error: %s \n", strerror(errno));
      //  exit(0);
    }
    return result;
}

int Bind_Socket(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) {
    if(bind(__fd, __addr, __len) < 0) {
        fprintf(stderr, "bind error: %s \n", strerror(errno));
       // exit(0);
    }
    return 0;
}

int SetSocket(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) {
    if(setsockopt(__fd, __level, __optname, __optval, __optlen) < 0) {
        fprintf(stderr, "set socket opt error: %s \n", strerror(errno));
      //  exit(0);
    }
    return 0;
}

int Inet_pton(int __af, const char *__restrict __cp, void *__restrict __buf) {
    if(inet_pton(__af, __cp, __buf) < 0 ) {
        fprintf(stderr, "set presentation to numeric error: %s \n", strerror(errno));
       // exit(0);
    }
}

int Inet_ntop(int __af, const void *__restrict __cp, char *__restrict __buf, socklen_t __len) {
    if(inet_ntop(__af, __cp, __buf, __len) == NULL) {
        fprintf(stderr, "set numeric to presenation error: %s \n", strerror(errno));
      //  exit(0);
    }
    return 0;
}

int Listen(int __fd, int __n) {
    if(listen(__fd, __n) < 0) {
        fprintf(stderr, "listen socket fd:%d  backlog: %d error: %s \n", __fd, __n, strerror(errno));
       // exit(0);
    }
    return 0;
}

int Select (int __nfds, fd_set *__restrict __readfds,
            fd_set *__restrict __writefds,
            fd_set *__restrict __exceptfds,
            struct timeval *__restrict __timeout) {
    int n = select(__nfds, __readfds, __writefds, __exceptfds, __timeout);
    if(n < 0) {
        fprintf(stderr, "select error: %s \n",strerror(errno));
       // exit(0);
    }
    return n;
}

int Accept (int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len) {
    int fd = accept (__fd,  __addr, __addr_len);
    if(fd < 0) {
        fprintf(stderr, "accept listentfd %d error: %s \n",__fd, strerror(errno));
      //  exit(0);
    }
    return fd;
}

int Close(int __fd) {
    int n;
    if((n = close(__fd)) < 0) {
        fprintf(stderr, "close fd %d error: %s \n",__fd, strerror(errno));
        //exit(0);
    }
    return n;
}

int Getpeername(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len) {
    if(getpeername(__fd,__addr,__len) < 0) {
        fprintf(stderr, "get peer name of fd %d error: %s \n",__fd, strerror(errno));
        //exit(0);
    }
}
int Getsockname(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len) {
    if(getsockname(__fd,__addr,__len) < 0) {
        fprintf(stderr, "get sock name of fd %d error: %s \n",__fd, strerror(errno));
        //exit(0);
    }
}

ssize_t Write_nByte(int fd, char* buf, ssize_t nbyte) {
    ssize_t n = write(fd, buf, nbyte);
    int count = 1;
    while(n != nbyte) {
        fprintf(stderr, "%d time write fd %d nbyte error, need %d byte, write %d byte\n",fd, count, nbyte, n);
        n += write(fd, buf + n , nbyte - n);
        if(n <= 0)
            return n;
    }
    return n;
}