#include <iostream>
#include "4over6_util.h"

 FILE* logfile;
 int sockfd;
 int tcp=0,udp=0,icmp=0,others=0,igmp=0,total=0,i,j;
 struct sockaddr_in source,dest;
 void process_packet(unsigned char* buffer , int size);
 void print_ip_header(unsigned char* , int);
 void print_tcp_packet(unsigned char* , int);
 void print_udp_packet(unsigned char * , int);
 void print_icmp_packet(unsigned char* , int);
 void PrintData (unsigned char* , int);
void test() {
    std::cout << "Hello, World!" << std::endl;
//    logfile = fopen("log.txt","w");
//    if(logfile == NULL)  printf("unable to create file\n");
//    printf("Starting.....\n");
//    struct sockaddr addr;
//    int datasize;
//    socklen_t saddr_size =  sizeof(addr);
//    unsigned  char* buffer = (unsigned char*)malloc(65536);
//
//    int raw_socket = Socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
//    std::cout << "Hello, World!" << std::endl;
//    int on = 1;
//    SetSocket(raw_socket, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on));
//
//
//    while(1)
//    {
//        datasize = recvfrom(raw_socket, buffer, 65536,0 ,&addr,&saddr_size );
//        if(datasize < 0)
//        {
//            printf("Recvfrom error, failed to get packets\n");
//            return -1;
//        }
//        process_packet(buffer,datasize);
//        //not work??????????????
//        //fprintf(logfile,"%s\n",buffer);
//        ssize_t n = sendto(raw_socket, buffer, datasize, 0, (SA*)&dest, sizeof(struct sockaddr_in));
//        if(n < 0) {
//            printf("\n\nsend size:%d error %s\n\n",sizeof(buffer), strerror(errno));
//        }
//        process_packet(buffer,datasize);
//    }
//
//    close(sockfd);
//    fclose(logfile);
//    printf("Finished");
}
 int main(int argc, char** argv) {
    if(strcmp(argv[1],"1") == 0) {
        do_server();
    }
    else {
        do_client();
    }
    return 0;
}


 void process_packet(unsigned char* buffer , int size)
 {
     struct iphdr *iph = (struct iphdr*)buffer;
     total++;
     //printf("\n1234\n");
     switch(iph->protocol)
     {
         case 1:
             ++icmp;
             //print_icmp_packet(buffer,size);
             break;

         case 2:
             ++igmp;
             break;
         case 6:
             ++tcp;
             print_tcp_packet(buffer , size);
             break;
         case 17:
             ++udp;
             //print_udp_packet(buffer , size);
             break;

         default:
             ++others;
             break;
     }
     fprintf(stderr,"TCP : %d   UDP : %d   ICMP : %d   IGMP : %d   Others : %d   Total : %d\r \n",
            tcp,udp,icmp,igmp,others,total);

 }

 void print_ip_header(unsigned char* Buffer, int  size)
 {
     unsigned short iphdrlen;
     struct iphdr *iph = (struct iphdr *)Buffer;
     iphdrlen = iph->ihl*4;
     memset(&source, 0, sizeof(source));
     source.sin_addr.s_addr = iph->saddr;


     memset(&dest, 0, sizeof(dest));
     dest.sin_addr.s_addr = iph->daddr;

     fprintf(logfile, "\n");
     fprintf(logfile, "IP Header\n");
     fprintf(logfile, "   |-IP Version        : %d\n",
             (unsigned int) iph->version);
     fprintf(logfile, "   |-IP Header Length  : %d DWORDS or %d Bytes\n",
             (unsigned int) iph->ihl, ((unsigned int) (iph->ihl)) * 4);
     fprintf(logfile, "   |-Type Of Service   : %d\n", (unsigned int) iph->tos);
     fprintf(logfile, "   |-IP Total Length   : %d  Bytes(Size of Packet)\n",
             ntohs(iph->tot_len));
     fprintf(logfile, "   |-Identification    : %d\n", ntohs(iph->id));
     //fprintf(logfile,"   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
     //fprintf(logfile,"   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
     //fprintf(logfile,"   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
     fprintf(logfile, "   |-TTL      : %d\n", (unsigned int) iph->ttl);
     fprintf(logfile, "   |-Protocol : %d\n", (unsigned int) iph->protocol);
     fprintf(logfile, "   |-Checksum : %d\n", ntohs(iph->check));

     char src_ip[60],dst_ip[60];
     Inet_ntop(AF_INET, &iph->saddr, src_ip, sizeof(src_ip));
     Inet_ntop(AF_INET, &iph->daddr, dst_ip, sizeof(dst_ip));
     fprintf(logfile, "   |-Source IP        : %s\n",
             src_ip);

     fprintf(logfile, "   |-Destination IP   : %s\n", dst_ip);
     iph->saddr = dest.sin_addr.s_addr;
     iph->daddr = source.sin_addr.s_addr;
     dest.sin_addr.s_addr =  iph->daddr;
   //  iph->daddr = iph->saddr;
 }

 void print_tcp_packet(unsigned char* Buffer, int Size)
 {
     unsigned short iphdrlen;

     struct iphdr *iph = (struct iphdr *)Buffer;
     iphdrlen = iph->ihl*4;

     struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen);

     fprintf(logfile,"\n\n***********************TCP Packet*************************\n");

     print_ip_header(Buffer,Size);

     fprintf(logfile,"\n");
     fprintf(logfile,"TCP Header\n");
     fprintf(logfile,"   |-Source Port      : %u\n",ntohs(tcph->source));
     fprintf(logfile,"   |-Destination Port : %u\n",ntohs(tcph->dest));
     fprintf(logfile,"   |-Sequence Number    : %u\n",ntohl(tcph->seq));
     fprintf(logfile,"   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
     fprintf(logfile,"   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
     //fprintf(logfile,"   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
     //fprintf(logfile,"   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
     fprintf(logfile,"   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
     fprintf(logfile,"   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
     fprintf(logfile,"   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
     fprintf(logfile,"   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
     fprintf(logfile,"   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
     fprintf(logfile,"   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
     fprintf(logfile,"   |-Window         : %d\n",ntohs(tcph->window));
     fprintf(logfile,"   |-Checksum       : %d\n",ntohs(tcph->check));
     fprintf(logfile,"   |-Urgent Pointer : %d\n",tcph->urg_ptr);
     fprintf(logfile,"\n");
     fprintf(logfile,"                        DATA Dump                         ");
     fprintf(logfile,"\n");

     fprintf(logfile,"IP Header\n");
     PrintData(Buffer,iphdrlen);

     fprintf(logfile,"TCP Header\n");
     PrintData(Buffer+iphdrlen,tcph->doff*4);

     fprintf(logfile,"Data Payload\n");
     PrintData(Buffer + iphdrlen + tcph->doff*4 , (Size - tcph->doff*4-iph->ihl*4) );

     fprintf(logfile,"\n###########################################################");
    // __uint16_t sp = tcph->source;
     tcph->source = tcph->dest;
     tcph->dest = htons(6666);
     dest.sin_port = htons(6666);

 }

 void print_udp_packet(unsigned char *Buffer , int Size)
 {

     unsigned short iphdrlen;

     struct iphdr *iph = (struct iphdr *)Buffer;
     iphdrlen = iph->ihl*4;

     struct udphdr *udph = (struct udphdr*)(Buffer + iphdrlen);

     fprintf(logfile,"\n\n***********************UDP Packet*************************\n");

     print_ip_header(Buffer,Size);

     fprintf(logfile,"\nUDP Header\n");
     fprintf(logfile,"   |-Source Port      : %d\n" , ntohs(udph->source));
     fprintf(logfile,"   |-Destination Port : %d\n" , ntohs(udph->dest));
     fprintf(logfile,"   |-UDP Length       : %d\n" , ntohs(udph->len));
     fprintf(logfile,"   |-UDP Checksum     : %d\n" , ntohs(udph->check));

     fprintf(logfile,"\n");
     fprintf(logfile,"IP Header\n");
     PrintData(Buffer , iphdrlen);

     fprintf(logfile,"UDP Header\n");
     PrintData(Buffer+iphdrlen , sizeof udph);

     fprintf(logfile,"Data Payload\n");
     PrintData(Buffer + iphdrlen + sizeof udph ,( Size - sizeof udph - iph->ihl * 4 ));

     fprintf(logfile,"\n###########################################################");
 }

 void print_icmp_packet(unsigned char* Buffer , int Size)
 {
     unsigned short iphdrlen;

     struct iphdr *iph = (struct iphdr *)Buffer;
     iphdrlen = iph->ihl*4;

     struct icmphdr *icmph = (struct icmphdr *)(Buffer + iphdrlen);

     fprintf(logfile,"\n\n***********************ICMP Packet*************************\n");

     print_ip_header(Buffer , Size);

     fprintf(logfile,"\n");

     fprintf(logfile,"ICMP Header\n");
     fprintf(logfile,"   |-Type : %d",(unsigned int)(icmph->type));

     if((unsigned int)(icmph->type) == 11)
         fprintf(logfile,"  (TTL Expired)\n");
     else if((unsigned int)(icmph->type) == ICMP_ECHOREPLY)
         fprintf(logfile,"  (ICMP Echo Reply)\n");
     fprintf(logfile,"   |-Code : %d\n",(unsigned int)(icmph->code));
     fprintf(logfile,"   |-Checksum : %d\n",ntohs(icmph->checksum));
     //fprintf(logfile,"   |-ID       : %d\n",ntohs(icmph->id));
     //fprintf(logfile,"   |-Sequence : %d\n",ntohs(icmph->sequence));
     fprintf(logfile,"\n");

     fprintf(logfile,"IP Header\n");
     PrintData(Buffer,iphdrlen);

     fprintf(logfile,"UDP Header\n");
     PrintData(Buffer + iphdrlen , sizeof icmph);

     fprintf(logfile,"Data Payload\n");
     PrintData(Buffer + iphdrlen + sizeof icmph , (Size - sizeof icmph - iph->ihl * 4));

     fprintf(logfile,"\n###########################################################");
 }

 void PrintData (unsigned char* data , int Size)
 {

     for(i=0 ; i < Size ; i++)
     {
         if( i!=0 && i%16==0)   //if one line of hex printing is complete...
         {
             fprintf(logfile,"         ");
             for(j=i-16 ; j<i ; j++)
             {
                 if(data[j]>=32 && data[j]<=128)
                     fprintf(logfile,"%c",(unsigned char)data[j]); //if its a number or alphabet

                 else fprintf(logfile,"."); //otherwise print a dot
             }
             fprintf(logfile,"\n");
         }

         if(i%16==0) fprintf(logfile,"   ");
         fprintf(logfile," %02X",(unsigned int)data[i]);

         if( i==Size-1)  //print the last spaces
         {
             for(j=0;j<15-i%16;j++) fprintf(logfile,"   "); //extra spaces

             fprintf(logfile,"         ");

             for(j=i-i%16 ; j<=i ; j++)
             {
                 if(data[j]>=32 && data[j]<=128) fprintf(logfile,"%c",(unsigned char)data[j]);
                 else fprintf(logfile,".");
             }
             fprintf(logfile,"\n");
         }
     }
 }