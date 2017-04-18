//
// Created by dalaoshe on 17-3-20.
//

#ifndef UDP_UNP_H
#define UDP_UNP_H
#include	<sys/types.h>	/* basic system data types */
#include	<sys/socket.h>	/* basic socket definitions */
#include	<sys/time.h>	/* timeval{} for select() */
#include	<time.h>		/* timespec{} for pselect() */
#include	<netinet/in.h>	/* sockaddr_in{} and other Internet defns */
#include	<arpa/inet.h>	/* inet(3) functions */
#include	<errno.h>
#include	<fcntl.h>		/* for nonblocking */
#include	<netdb.h>
#include	<signal.h>
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include   <stdint.h>
#include	<sys/stat.h>	/* for S_xxx file mode constants */
#include	<sys/uio.h>		/* for iovec{} and readv/writev */
#include	<unistd.h>
#include	<sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <sys/types.h>
#include	<sys/un.h>		/* for Unix domain sockets */
#include    <netinet/in.h>

#include <netinet/ether.h>
#include    <netinet/ip.h>
#include    <netinet/ip6.h>
#include    <netinet/tcp.h>
#include    <netinet/udp.h>
#include <netinet/ip_icmp.h>

#include <linux/netlink.h>
/*IP_QUEUE主要数据结构头文件*/
#include<linux/netfilter_ipv4/ip_queue.h>

#define	SA	struct sockaddr
#define SERV_PORT 6666
#define CLIENT_PORT 5555
#define MAX_LEN 4096

struct unp_in_pktinfo {
    struct in_addr	ipi_addr;	/* dst IPv4 address */
    int				ipi_ifindex;/* received interface index */
};


int Socket(int __domain, int __type, int __protocol) ;

int Socket_Peer_Connect(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) ;

int Bind_Socket(int __fd, __CONST_SOCKADDR_ARG __addr, socklen_t __len) ;

int SetSocket(int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen);

int Listen(int __fd, int __n);

int Select (int __nfds, fd_set *__restrict __readfds,
                   fd_set *__restrict __writefds,
                   fd_set *__restrict __exceptfds,
                   struct timeval *__restrict __timeout);

int Accept (int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __addr_len);

int Close(int __fd);

int Getpeername(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);
int Getsockname(int __fd, __SOCKADDR_ARG __addr, socklen_t *__restrict __len);

int Inet_pton(int __af, const char *__restrict __cp, void *__restrict __buf);

int Inet_ntop(int __af, const void *__restrict __cp, char *__restrict __buf, socklen_t __len);

ssize_t Write_nByte(int fd, char* buf, ssize_t nbyte);
#endif //UDP_UNP_H
