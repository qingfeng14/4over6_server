//
// Created by dalaoshe on 17-4-18.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
/*IP_QUEUE主要数据结构头文件*/
#include<linux/netfilter_ipv4/ip_queue.h>


#define VAL_LEN  1024
#define RS_OK 0
#define RS_WRONG -1

typedef unsigned int u32;
/*
 应用层发送到内核的报文的二种模式。
*/
enum {
    SET_MODE = 0,
    RETURN_IDE
};

/*
 应用层返回给内核的几种报文的处理意见
*/
enum {
    NF_DROP = 0,
    NF_ACCEPT,
    NF_STOLEN,
    NF_QUEUE,
    NF_REPEAT,
    NF_STOP,
    NF_MAX_VERDICT,
};

typedef struct ipq_hand {
    int ipqfd;
    struct sockaddr_nl local, peer;
}ipqhander;

/*
 应用层请求内核报文头部
*/
struct req_head {
    /*netlink特有报文头部*/
    struct nlmsghdr nlh;
    /*ip_queue头部*/
    ipq_peer_msg_t reqh;
};

struct ipq_hand *ipqh;

/*创建netlink套接字，并初始化*/
int create_ipqueue_hander()
{
    ipqh = (struct ipq_hand *)malloc(sizeof(struct ipq_hand));
    if (NULL == ipqh) {
        printf ("[%s:%d]malloc ipq hander failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return RS_WRONG;
    }

    memset(ipqh, 0, sizeof(struct ipq_hand));
    ipqh->ipqfd = socket(AF_NETLINK, SOCK_RAW, 0);
    if (ipqh->ipqfd == -1) {
        printf ("[%s:%d]create ip_queue socket failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return RS_WRONG;
    }

    memset(&ipqh->local,0, sizeof(ipqh->local));
    ipqh->local.nl_family = AF_NETLINK;
    ipqh->local.nl_pid = getpid();
    ipqh->local.nl_groups = 0;

    memset(&ipqh->peer, 0, sizeof(ipqh->peer));
    ipqh->peer.nl_family = AF_NETLINK;
    ipqh->peer.nl_pid = 0;
    ipqh->peer.nl_groups = 0;

    if(bind(ipqh->ipqfd, (struct sockaddr *)&(ipqh->local), sizeof (ipqh->local))) {
        printf ("[%s:%d]bind ip_queue socket failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return RS_WRONG;
    }

    return RS_OK;
}

/*向内核发送消息，包括二种，一种为模式设置，一种为返回处理意见*/
int send_msg_to_kernel(u32 msgtype, ipq_packet_msg_t *m, u32 verdict)
{
    int ret;
    struct req_head req;

    req.nlh.nlmsg_len = NLMSG_LENGTH(sizeof (req));
    req.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.nlh.nlmsg_pid = getpid();

    switch(msgtype) {
        case SET_MODE:
            printf ("now set the mode\n");
            req.nlh.nlmsg_type = IPQM_MODE;
            req.reqh.msg.mode.value = IPQ_COPY_PACKET;
            req.reqh.msg.mode.range = VAL_LEN;
            break;
        case RETURN_IDE:
            req.nlh.nlmsg_type = IPQM_VERDICT;
            req.reqh.msg.verdict.value = verdict;
            req.reqh.msg.verdict.id = m->packet_id;
            req.reqh.msg.verdict.data_len = 0;
            break;
        default:
            break;
    }

    ret = sendto(ipqh->ipqfd, &req, sizeof(req), 0, (struct sockaddr *)&ipqh->peer, sizeof(ipqh->peer));
    if (ret == -1) {
        printf ("[%s:%d]send ipq msg failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
        return RS_WRONG;
    }

    return RS_OK;
}
/*从内核接收消息*/
int recv_msg_from_kernel()
{
    int ret, status;
    char buf[VAL_LEN];
    struct nlmsghdr *nlh;
    ipq_packet_msg_t *packet;
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(ipqh->ipqfd, &read_fds);

    while (1) {
        fprintf(stderr,"wait......\n");
        ret = select(ipqh->ipqfd +1, &read_fds, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR) {
                continue;
            } else {
                printf ("[%s:%d]recvfrom msg failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
                continue;
            }
        }

        if (!FD_ISSET(ipqh->ipqfd, &read_fds))
            continue;
        memset(buf, 0, sizeof(buf));
        status = recvfrom(ipqh->ipqfd, buf, VAL_LEN, 0, NULL, NULL);
        if (status < 0) {
            printf ("[%s:%d]recvfrom msg failed, error is %s\n", __FUNCTION__, __LINE__, strerror(errno));
            continue;
        }

        if (status == 0) {
            printf ("[%s:%d]recv NULL msg\n", __FUNCTION__, __LINE__);
            continue;
        }
        nlh = (struct nlmsghdr *)buf;
        if (nlh->nlmsg_flags & MSG_TRUNC || nlh->nlmsg_len > status) {
            continue;
        }

        packet = (ipq_packet_msg*)NLMSG_DATA((struct nlmsghdr *)(buf));
        printf("recv bytes =%d, nlmsg_len=%d, indev=%s, datalen=%d, packet_id=%x\n", status, nlh->nlmsg_len,
               packet->indev_name,  packet->data_len, (u32)packet->packet_id);
        if (send_msg_to_kernel(RETURN_IDE, packet, NF_ACCEPT)) {
            printf ("[%s:%d]send_msg_to_kernel error\n", __FUNCTION__, __LINE__);
            continue;
        }
    }
    return RS_OK;
}

int main(int argc, char *argv[]) {
    if (create_ipqueue_hander()) {
        printf("[%s:%d]create_ipqueue_hander error\n", __FUNCTION__, __LINE__);
        return RS_WRONG;
    }

    if (send_msg_to_kernel(SET_MODE, NULL, 0)) {
        printf("[%s:%d]send_msg_to_kernel error\n", __FUNCTION__, __LINE__);
        return RS_WRONG;
    }
    recv_msg_from_kernel();

    return 0;
}