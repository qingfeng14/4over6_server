//
// Created by dalaoshe on 17-4-14.
//

#include "4over6_util.h"
#define IPV4_ADDR "59.66.134.90"
static void printmessage(unsigned char *buf);

static unsigned char *printnamestring(unsigned char *p,unsigned char *buf);



#define GETWORD(__w,__p) do{__w=*(__p++)<<8;__w|=*(p++);}while(0)

#define GETLONG(__l,__p) do{__l=*(__p++)<<24;__l|=*(__p++)<<16;__l|=*(__p++)<<8;__l|=*(p++);}while(0)
in_addr addr_v4[5];
static void request_ipv4(int fd);
int get_dns_request(char *dns_name, unsigned char* dns_buf, ssize_t *dns_size);
uint16_t
in_cksum(uint16_t *addr, int len)
{
    int				nleft = len;
    uint32_t		sum = 0;
    uint16_t		*w = addr;
    uint16_t		answer = 0;

    /*
     * Our algorithm is simple, using a 32 bit accumulator (sum), we add
     * sequential 16 bit words to it, and at the end, fold back all the
     * carry bits from the top 16 bits into the lower 16 bits.
     */
    while (nleft > 1)  {
        sum += *w++;
        nleft -= 2;
    }

    /* 4mop up an odd byte, if necessary */
    if (nleft == 1) {
        *(unsigned char *)(&answer) = *(unsigned char *)w ;
        sum += answer;
    }

    /* 4add back carry outs from top 16 bits to low 16 bits */
    sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
    sum += (sum >> 16);			/* add carry */
    answer = ~sum;				/* truncate to 16 bits */
    return(answer);
}
uint16_t udp_checksum(unsigned long saddr,
             unsigned long daddr,
             unsigned short *buffer,
             int size)
{
    unsigned char rawBuf[1024];
    struct pseudo_hdr
    {
        struct in_addr  src;
        struct in_addr  dst;
        uint8_t         mbz;
        uint8_t         proto;
        uint16_t        len;
    } __attribute__((__packed__));
    unsigned long sum = 0;
    struct pseudo_hdr *ph;
    int ph_len = sizeof(struct pseudo_hdr);
    ph = (struct pseudo_hdr *)rawBuf;
    ph->src.s_addr = saddr;
    ph->dst.s_addr = daddr;
    ph->mbz = 0;
    ph->proto = IPPROTO_UDP;
    ph->len = htons(size); //这里的长度为udp header + payload 的总和
    //buffer = udpheader + payload,  size = sizeof(udpheader + payload)
    memcpy(rawBuf + ph_len, buffer, size);
    //ph_len + size 是虚假头长＋UDP长＋payload长。来计算checksum
    sum = in_cksum((unsigned short*)rawBuf,ph_len+size);
    return(sum);
}

void do_client() {

    unsigned char buf[512];
    unsigned short iphdrlen, ip_len;

    struct ip *iph = (struct ip *)buf;
//    Inet_pton(AF_INET, "166.111.8.68",&iph->ip_dst);
    Inet_pton(AF_INET, IPV4_ADDR,&iph->ip_dst);
    Inet_pton(AF_INET, "10.0.0.3",&iph->ip_src);

    struct udphdr *udph = (struct udphdr*)(buf + sizeof(*iph));
    unsigned char* payload = buf + sizeof(struct iphdr) + sizeof(struct udphdr);

    ssize_t dns_len;
    char s[] = "www.baidu.com";
    get_dns_request(s, payload, &dns_len);
    udph->dest = htons(6666);
    udph->source = htons(33675);
    udph->len = htons(dns_len + 8);
    udph->check = udp_checksum(iph->ip_dst.s_addr,iph->ip_src.s_addr,(uint16_t*)udph,8+dns_len);
//dns&&(ip.src==59.66.134.64||ip.src==166.111.8.28)&&(udp.srcport==38500||udp.dstport==38500)
    //ip hdr
    iph->ip_v = IPVERSION;
    iph->ip_hl = sizeof(*iph) >> 2;
    iph->ip_tos = 0;
    iph->ip_len = htons(20+8+dns_len);
    iph->ip_id = htons(0);
    iph->ip_off = htons(IP_DF);
    iph->ip_ttl = 64;
    iph->ip_p = IPPROTO_UDP;
//    iph->protocol = 17;
//    iph->check = 0;
    //


    struct sockaddr_in dest;
    Inet_pton(AF_INET, IPV4_ADDR,&dest.sin_addr.s_addr);
    dest.sin_port = htons(53);
    dest.sin_family = AF_INET;

    int raw_socket = Socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
    int on = 1;
    SetSocket(raw_socket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));

   // ssize_t n = sendto(raw_socket, buf, 20+8+dns_len , 0, (SA*)&dest, sizeof(struct sockaddr_in));
//        if(n < 0) {
//            printf("\n\nsend size:%d error %s\n\n",20+8+dns_len, strerror(errno));
//        }
    printf("send over\n");
    struct sockaddr addr;
    int datasize;
    socklen_t saddr_size =  sizeof(addr);
    int fd =   socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in cli;
    cli.sin_family = AF_INET;
    cli.sin_port = htons(38500);
    Inet_pton(AF_INET, "20.0.0.3",&cli.sin_addr.s_addr);

//    Bind_Socket(fd, (SA*)&cli, sizeof(cli));
//    unsigned  char* buffer = (unsigned char*)malloc(65536);
//        datasize = recvfrom(fd, buffer, 65536,0 ,&addr,&saddr_size );
//        if(datasize < 0)
//        {
//            printf("Recvfrom error, failed to get packets\n");
//            return ;
//        }
//    process_packet(buffer,datasize);

    //return;
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(struct sockaddr_in6));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(SERV_PORT);
    server_addr.sin6_addr = in6addr_any;

    socklen_t len = sizeof(struct sockaddr_in6);


    int sockfd = Socket(AF_INET6, SOCK_STREAM, 0);

    Socket_Peer_Connect(sockfd, (SA*)&server_addr, len);

    request_ipv4(sockfd);
    while (1){
        static struct Msg msg;
        memset(&msg, 0, sizeof(struct Msg));
        ssize_t needbs = sizeof(struct Msg_Hdr);
        ssize_t n = read(sockfd, &msg, needbs);
        if(msg.hdr.type == 104) {
            fprintf(stderr," client read an 104 packet\n");
            memset(&msg, 0, sizeof(struct Msg));
            msg.hdr.type = 104;
            msg.hdr.length = 0;
            fprintf(stderr,"------- client send a keep alive to server  ----------\n");
            Write_nByte(sockfd, (char*)&msg, sizeof(struct Msg_Hdr)+msg.hdr.length);

        }
        else if(msg.hdr.type == 103) {
            int n = sizeof(struct udphdr);
            read(sockfd, msg.ipv4_payload, msg.hdr.length);
            fprintf(stderr,"recv an resp_packet: %s \n",((char*)msg.ipv4_payload+28));
        }
        msg.hdr.type = 102;
        msg.hdr.length = 20+8+dns_len;
        memcpy(msg.ipv4_payload, buf, 20+8+dns_len);
        n = Write_nByte(sockfd, (char*)&msg, sizeof(struct Msg_Hdr)+msg.hdr.length);
        if(n < 0) {
            fprintf(stderr, "send buf error \n", strerror(errno));
        }
    }
}



int get_dns_request(char *dns_name, unsigned char* dns_buf, ssize_t *dns_size) {
    time_t ident;

    int fd;

    int rc;

    socklen_t serveraddrlent;

    char *q;

    unsigned char *p;

    unsigned char *countp;

    unsigned char reqBuf[512] = {0};

    unsigned char rplBuf[512] = {0};

    struct sockaddr_in serveraddr;


    //udp

    fd = socket(AF_INET, SOCK_DGRAM, 0);

    if(fd == -1)

    {

        perror("error create udp socket");

        return -1;

    }
    time(&ident);

    //copy

    p = dns_buf;

    //Transaction ID

    *(p++) = 0x66;

    *(p++) = 0x81;

    //Header section

    //flag word = 0x0100

    *(p++) = 0x01;

    *(p++) = 0x00;

    //Questions = 0x0001

    //just one query

    *(p++) = 0x00;

    *(p++) = 0x01;

    //Answer RRs = 0x0000

    //no answers in this message

    *(p++) = 0x00;

    *(p++) = 0x00;

    //Authority RRs = 0x0000

    *(p++) = 0x00;

    *(p++) = 0x00;

    //Additional RRs = 0x0000

    *(p++) = 0x00;

    *(p++) = 0x00;

    //Query section

    countp = p;

    *(p++) = 0;

    for(q=dns_name; *q!=0; q++)

    {

        if(*q != '.')

        {

            (*countp)++;

            *(p++) = *q;

        }

        else if(*countp != 0)

        {

            countp = p;

            *(p++) = 0;

        }

    }

    if(*countp != 0)

        *(p++) = 0;



    //Type=1(A):host address

    *(p++)=0;

    *(p++)=1;

    //Class=1(IN):internet

    *(p++)=0;

    *(p++)=1;



    printf("\nRequest:\n");

    printmessage(reqBuf);



    //fill

    bzero(&serveraddr, sizeof(serveraddr));

    serveraddr.sin_family = AF_INET;

    serveraddr.sin_port = htons(53);

    serveraddr.sin_addr.s_addr = inet_addr("166.111.8.28");



    //send to DNS Serv

    *dns_size = p - dns_buf;
    return 0;
    if(sendto(fd,dns_buf,p-dns_buf,0,(SA *)&serveraddr,sizeof(serveraddr)) < 0)

    {

        perror("error sending request");

        return -1;

    }
    return 1;


    //recev the reply

    bzero(&serveraddr,sizeof(serveraddr));

    serveraddrlent = sizeof(serveraddr);

    rc = recvfrom(fd,&rplBuf,sizeof(rplBuf),0,(SA *)&serveraddr,&serveraddrlent);

    if(rc < 0)

    {

        perror("error receiving request\n");

        return -1;

    }



    //print out results

    printf("\nReply:\n");

    printmessage(rplBuf);



    //exit

    printf("Program Exit\n");

    return 0;
}

void request_ipv4(int fd) {
    static struct Msg msg;
    memset(&msg, 0, sizeof(struct Msg));
    ssize_t needbs = sizeof(struct Msg_Hdr);
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
    memset(&msg, 0, sizeof(struct Msg));
    n = read(fd, &msg, needbs);
    if(n < 0) {
        fprintf(stderr,"client read 101 reply hdr error: %s\n",strerror(errno));
    }
    else if(n != needbs) {
        fprintf(stderr,"client read 101 reply hdr error, need %d, write %d \n",needbs, n);
    }
    else {
        if(msg.hdr.type == 101) {
            n = read(fd, msg.ipv4_payload, msg.hdr.length);
            if(n < 0) {
                fprintf(stderr,"client read 101 reply payload error: %s\n",strerror(errno));
            }
            else if(n != msg.hdr.length) {
                fprintf(stderr,"client read 101 reply payload error, need %d, write %d \n",msg.hdr.length, n);
            }
            else {
                Ipv4_Request_Reply* reply = (Ipv4_Request_Reply*)msg.ipv4_payload;
                for(int i = 0 ; i < 5; ++i) {
                    char buf[16];
                    Inet_ntop(AF_INET, &(reply->addr_v4[i]), buf,sizeof(buf));
                    fprintf(stderr,"recev %d , ip_v4: %s \n",i, buf);
                }
                for(int i = 0 ; i < 3; ++i)
                    addr_v4[i] = reply->addr_v4[i+2];
            }
        }

    }

}



static void printmessage(unsigned char *buf)

{

    unsigned char *p;

    unsigned int ident,flags,qdcount,ancount,nscount,arcount;

    unsigned int i,j,type,cclass,ttl,rdlength;



    p = buf;

    GETWORD(ident,p);

    printf("ident=%#x\n",ident);



    GETWORD(flags,p);

    printf("flags=%#x\n",flags);

    //printf("qr=%u\n",(flags>>15)&1);

    printf("qr=%u\n",flags>>15);



    printf("opcode=%u\n",(flags>>11)&15);

    printf("aa=%u\n",(flags>>10)&1);

    printf("tc=%u\n",(flags>>9)&1);

    printf("rd=%u\n",(flags>>8)&1);

    printf("ra=%u\n",(flags>>7)&1);

    printf("z=%u\n",(flags>>4)&7);

    printf("rcode=%u\n",flags&15);



    GETWORD(qdcount,p);

    printf("qdcount=%u\n",qdcount);



    GETWORD(ancount,p);

    printf("ancount=%u\n",ancount);



    GETWORD(nscount,p);

    printf("nscount=%u\n",nscount);



    GETWORD(arcount,p);

    printf("arcount=%u\n",arcount);



    for(i=0; i<qdcount; i++)

    {

        printf("qd[%u]:\n",i);

        while(*p!=0)

        {

            p = printnamestring(p,buf);

            if(*p != 0)

                printf(".");

        }

        p++;

        printf("\n");

        GETWORD(type,p);

        printf("type=%u\n",type);

        GETWORD(cclass,p);

        printf("class=%u\n",cclass);

    }



    for(i=0; i<ancount; i++)

    {

        printf("an[%u]:\n",i);

        p = printnamestring(p,buf);

        printf("\n");

        GETWORD(type,p);

        printf("type=%u\n",type);

        GETWORD(cclass,p);

        printf("class=%u\n",cclass);

        GETLONG(ttl,p);

        printf("ttl=%u\n",ttl);

        GETWORD(rdlength,p);

        printf("rdlength=%u\n",rdlength);

        printf("rd=");

        for(j=0; j<rdlength; j++)

        {

            printf("%2.2x(%u)",*p,*p);

            p++;

        }

        printf("\n");

    }

}



static unsigned char *printnamestring(unsigned char *p,unsigned char *buf)

{

    unsigned int nchars,offset;



    nchars = *(p++);

    if((nchars & 0xc0) == 0xc0)

    {

        offset = (nchars & 0x3f) << 8;

        offset |= *(p++);

        nchars = buf[offset++];

        printf("%*.*s",nchars,nchars,buf+offset);

    }

    else

    {

        printf("%*.*s",nchars,nchars,p);

        p += nchars;

    }



    return (p);

}
