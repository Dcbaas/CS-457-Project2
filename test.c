#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <netinet/ip_icmp.h>

int main(int argc, char** argv){
    
    printf("%lu\n", sizeof(struct icmphdr));
    return 0;
            
}
