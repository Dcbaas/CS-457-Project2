#ifndef                     PACKET_H
#define                     PACKET_H

#include <exception>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <string>

namespace shared{

    typedef union HeaderDetail{
        struct ether_arp arp;
        struct icmphdr icmp;
    } HeaderDetail;

    class Packet{
    public:
        //Won't take a constexpr below?
        char data[1500];
        Packet(char* data);
        Packet();
        //Construct an ARP request header
        //Packet(...);

        //Construct an ICMP Request header
        //Packet(...);

        Packet constructResponseARP(const Packet& request);
        Packet constructResponseICMP(const Packet& request);
        Packet& operator=(Packet other);

        void printARPData();

        //        ~Packet();
        bool isARP() const;

    private:
        HeaderDetail detail;
        struct ether_header ethernetHeader;
        struct iphdr ipHeader;
        bool arp{false};
        static constexpr unsigned long ARP_CODE = 0x0806;
        static constexpr unsigned long ICMP_CODE = 0x0800;
        static constexpr unsigned char ETHER_LEN = 14;
        static constexpr unsigned char IP_LEN = 20;
        static constexpr unsigned char ARP_LEN = 28;
        static constexpr unsigned char ICMP_LEN = 8;

    };
}

#endif

