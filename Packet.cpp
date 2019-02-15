#include "Packet.h"

#include <memory>
#include <algorithm>

#include<ifaddrs.h>
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

namespace shared{ 
    Packet::Packet(char* data){
        memcpy(this->data, data, 1500);

        //Construct ethernetheader
        memcpy(&ethernetHeader,this->data, ETHER_LEN);

        printf("Packet Type: %d\n", ntohs(ethernetHeader.ether_type));

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


    bool Packet::constructResponseARP(struct ifaddrs* interfaceList){
        //Remember that IPs only have 4 elements while MAC addresses have 6
        //The target IP we have and the MAC we are looking for.
        //TODO maybe typedef this to make it easier to read.
        uint32_t targetIP;
        memcpy(&targetIP, detail.arp.arp_tpa, 4);

        uint8_t* targetMAC;
        
        uint8_t* senderIP = detail.arp.arp_spa;
        uint8_t* senderMAC = detail.arp.arp_sha;
        
        char* targetInterface;
        struct ifaddrs* temp;
        struct ifaddrs* temp2;
    
        //Find the interface for the matching ip. 
        for(temp = interfaceList; temp != NULL; temp = temp->ifa_next){
            if(temp->ifa_addr->sa_family == AF_INET){
                //Cant be cast as a static cast
                struct sockaddr_in* foundIP = (struct sockaddr_in*) (temp->ifa_addr);
                //We found the matching ip get the interface name.
                if(foundIP->sin_addr.s_addr == targetIP){
                    targetInterface = temp->ifa_name;

                    //Find the matching MAC address 
                    for(temp2 = interfaceList; temp2 != NULL; temp2 = temp2->ifa_next){
                        //We found the MAC address of the interface we need.
                        if(strcmp(temp2->ifa_name, targetInterface) == 0 && 
                                temp2->ifa_addr->sa_family == AF_PACKET){
                            struct sockaddr_ll* targetMatch = (struct sockaddr_ll*)(temp->ifa_addr);
                            targetMAC = targetMatch->sll_addr;
                            
                            //Temp test
                            return true;
                            //Construct the response packet
                            //Target becomes the sender and the sender becomes the target.
                        }
                    }
                    break;
                }
            }
        }
        return false;
    }


    bool Packet::isARP() const{
        return this->arp;
    }

    void Packet::printARPData() {
        //Print the MAC Address? 
        struct ether_addr temp;
        for(int i = 0; i < 6; ++i){
            temp.ether_addr_octet[i] = detail.arp.arp_sha[i];
        }
        char* mac_str = ether_ntoa(&temp);
        printf("%s\n", mac_str);
        printf("\n");
    }

    //DEPRICATED
    bool Packet::equalIPs(uint8_t* rhs, uint8_t* lhs){
        for(int octet = 0; octet < 4; ++octet){
            if(rhs[octet] != lhs[octet]){
                return false;
            }
        }
        return true;
    }

    //BadPacket::BadPacket(const char* file, unsigned int line, const char* function, const char* info) :
    //file(file), line(line), function(function), info(info) {}
}

