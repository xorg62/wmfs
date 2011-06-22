/******
 * HFTSniff simple sniffer using pcap
 * xorg62@gmail.com
 ******/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

#include <pcap.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

/* Macros */
#define LEN(x)  (sizeof(x) / sizeof(*x))

/* Using ANSI escape codes to use bold in simple text mode */
#define IP_SRCDEST(src, psrc, dest, pdest)                  \
     do {                                                   \
          printf("%s:\e[1m%d\e[0m  ==>  %s:\e[1m%d\e[0m\n", \
                    src, psrc, dest, pdest);                \
     } while(/* CONSTCOND */ 0);

#define DISP_PROTO(s)                     \
     do {                                 \
          printf("( \e[1m%s\e[0m ) ", s); \
     } while(/* CONSTCOND */ 0);

#define BYE(s, er)                           \
     do {                                    \
          fprintf(stderr,  s": (%s)\n", er); \
          exit(EXIT_FAILURE);                \
     } while(/* CONSTCOND */ 0);

#define PKT_ERROR(r, s, a)                                 \
     do {                                                  \
          fprintf(stderr, "  PKT_ERROR: %s (%u)\n", s, a); \
          return r;                                        \
     } while(/* CONSTCOND */ 0);                           \

/* Option const */
#define FILTER  "ip"
#define TIMEOUT (10)
#define SNAPLEN (SHRT_MAX)

/*
 * Ethernet packet format
 */
#define ETHERNET_SIZE (14)
#define ETHERNET_ALEN (6)

struct ethernet_header
{
     unsigned char  ether_dhost[ETHERNET_ALEN]; /* Destination ethernet address */
     unsigned char  ether_shost[ETHERNET_ALEN]; /* Source ethernet adress       */
     unsigned short ether_type;                 /* Packet type, id field        */
};

/*
 * Ip packet format (tcpdump way)
 */
#define IP_V(ip)  (((ip)->ip_vhl & 0xf0) >> 4)
#define IP_HL(ip) ((ip)->ip_vhl  & 0x0f)

#define IP_DF      (0x4000) /* Dont fragment flag        */
#define IP_MF      (0x2000) /* More fragments flag       */
#define IP_OFFMASK (0x1fff) /* Mask for fragmenting bits */

struct ip_header
{
     unsigned char  ip_vhl;                 /* Header length, version  */
     unsigned char  ip_tos;                 /* Type of service         */
     unsigned short ip_len;                 /* Total length            */
     unsigned short ip_id;                  /* Identification          */
     unsigned short ip_off;                 /* Fragment offset field   */
     unsigned char  ip_ttl;                 /* Time to live            */
     unsigned char  ip_p;                   /* Protocol                */
     unsigned short ip_sum;                 /* Checksum                */
     struct         in_addr ip_src, ip_dst; /* Source and dest address */
};

/*
 * TCP packet format
 */
#define TH_OFF(th) (((th)->th_offx2 & 0xf0) >> 4)

#define TH_FIN   (0x01)
#define TH_SYN   (0x02)
#define TH_RST   (0x04)
#define TH_PUSH  (0x08)
#define TH_ACK   (0x10)
#define TH_URG   (0x20)
#define TH_ECE   (0x40)
#define TH_CWR   (0x80)
#define TH_FLAGS (0xf7) /* TH_* */

struct tcp_header
{
     unsigned short th_sport; /* Source port            */
     unsigned short th_dport; /* Destination port       */
     unsigned int   th_seq;   /* Sequence number        */
     unsigned int   th_ack;   /* Acknowledgement number */
     unsigned char  th_offx2; /* Data offset, rsvd      */
     unsigned char  th_flags; /* Flags bitfield         */
     unsigned short th_win;   /* Window                 */
     unsigned short th_sum;   /* Checksum               */
     unsigned short th_urp;   /* Urgent pointer         */
};

/* Function protos */
int pkt_tcp_handle(void *, void *);


/* Usefull things */
const struct
{
     unsigned int p;
     int          (*f)(void *, void*);
     char         name[32];
} protos[] =
{
     { IPPROTO_IP,   NULL,           "IP"   }, /* _IP   0  */
     { IPPROTO_ICMP, NULL,           "ICMP" }, /* _ICMP 1  */
     { IPPROTO_TCP,  pkt_tcp_handle, "TCP"  }, /* _TCP  6  */
     { IPPROTO_UDP,  NULL,           "UDP"  }, /* _UDP  17 */
     { -1, NULL, ""}
};


/* As defined in man: */
char errbuf[PCAP_ERRBUF_SIZE];

/*
 * Handle TCP packets
 */
int
pkt_tcp_handle(void *pkt, void *prev_pkt)
{
     const struct tcp_header *tcp;
     const struct ip_header  *ip;
     int                      len;

     ip  = (struct ip_header*)prev_pkt;
     tcp = (struct tcp_header*)(pkt + ETHERNET_SIZE + (IP_HL(ip) << 2));

     if((len = TH_OFF(tcp) << 2) < 20)
          PKT_ERROR(0, "Invalid IP header length:", len);

     IP_SRCDEST(inet_ntoa(ip->ip_src), ntohs(tcp->th_sport),  /* src  */
                inet_ntoa(ip->ip_dst), ntohs(tcp->th_dport)); /* dest */

     return len;
}

/*
 * Handle every packet, point it to corresponding pkt_proto_handle
 */
void
pkt_handle(unsigned char *args, const struct pcap_pkthdr *header, const unsigned char *packet)
{
     struct ethernet_header *eth;
     struct ip_header       *ip;
     int                     len, plen, i = 0;
     bool                    pfound = false;

     /* Translate ethernet pkt */
     eth = (struct ethernet_header*)packet;

     /* Translate ip pkt */
     ip = (struct ip_header*)(packet + ETHERNET_SIZE);

     /* Check ipv4 */
     if((len = IP_HL(ip) << 2) < 20)
          PKT_ERROR(, "Invalid IP header length:", len);

     /* Protocol */
     for(; i < LEN(protos); ++i)
          if(ip->ip_p == protos[i].p)
          {
               DISP_PROTO(protos[i].name);

               if(protos[i].f)
                    plen = protos[i].f((void*)packet, (void*)ip);

               pfound = true;
               break;
          }

     /* Unknown protocol */
     if(!pfound)
     {
          DISP_PROTO("unknown");
          IP_SRCDEST(inet_ntoa(ip->ip_src), -1,  /* src  */
                     inet_ntoa(ip->ip_dst), -1); /* dest */

     }

     puts("\n");

     return;
}

int
main(int argc, char **argv)
{
     char               *dev;
     bpf_u_int32         mask;
     bpf_u_int32         netip;
     pcap_t             *descr;
     struct bpf_program  bpf;

     /* Find device */
       if(!(dev = pcap_lookupdev(errbuf)))
          BYE("Can't find device", errbuf);

     printf("Device ( %s )\n", dev);

     /* Find adress & mask */
     if(pcap_lookupnet(dev, &netip, &mask, errbuf) < 0)
     {
          fprintf(stderr, "Can't find Adress or Mask: (%s)", errbuf);
          netip = mask = 0;
     }

     /* Open device and reaaad */
     if(!(descr = pcap_open_live(dev, SNAPLEN, true,  TIMEOUT, errbuf)))
          BYE("Fail at opening and reading device", errbuf);

     /* Check if packet is from ethernet device */
     if(pcap_datalink(descr) != DLT_EN10MB)
          BYE("Device is not an Ethernet", dev);

     /* Parse & set pcap with option expression */
     if(pcap_compile(descr, &bpf, FILTER, 0, netip) < 0)
          BYE("Option parse error", pcap_geterr(descr));

     if(pcap_setfilter(descr, &bpf) < 0)
          BYE("Can't use option", pcap_geterr(descr));

     pcap_loop(descr, 10, pkt_handle, NULL);

     pcap_freecode(&bpf);
     pcap_close(descr);

     return 0;
}

