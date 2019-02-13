#include "Packet.h"

#include <algorithm>

#include <netinet/if_ether.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netpacket/packet.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <net/ethernet.h>

namespace shared{ Packet::Packet(char* data){
        memcpy(this->data, data, 5000);

        //Construct ethernetheader
        memcpy(&ethernetHeader,this->data, ETHER_LEN);

        //Check the type
        if(ntohs(ethernetHeader.ether_type) == ARP_CODE){
            //Arp stuff
            memcpy(&detail.arp, &data[ETHER_LEN], ARP_LEN);
            arp = true;
        }
        else if(ntohs(ethernetHeader.ether_type) == ICMP_CODE){
            //ICMP stuff
            memcpy(&ipHeader, &data[ETHER_LEN], IP_LEN);
            memcpy(&detail.icmp, &data[ETHER_LEN + IP_LEN], ICMP_LEN);
            arp = false;
        }
        else{
            throw 1;
        }
    }
    Packet::Packet(){}
    Packet& Packet::operator=(Packet other){
        std::swap(this->detail,other.detail);
        std::swap(this->data, other.data);
        std::swap(this->ethernetHeader, other.ethernetHeader);
        std::swap(this->ipHeader, other.ipHeader);
        std::swap(this->arp, other.arp);

        return *this;
    }

    bool Packet::isARP() const{
        return this->arp;
    }
}

