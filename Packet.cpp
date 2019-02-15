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

    Packet::Packet(uint8_t* senderIP, uint8_t* senderMAC, uint8_t* targetIP, uint8_t* targetMAC, Packet& request){
        struct arphdr* requestEthHeader = &request.detail.arp.ea_hdr;

        detail.arp.ea_hdr.ar_hrd = requestEthHeader->ar_hrd; 
        detail.arp.ea_hdr.ar_pro = requestEthHeader->ar_pro;
        detail.arp.ea_hdr.ar_hln = requestEthHeader->ar_hln;
        detail.arp.ea_hdr.ar_pln = requestEthHeader->ar_pln;
        detail.arp.ea_hdr.ar_op = ARPOP_REPLY;


        printf("Arp Response Construction Check: ");
        memcpy(detail.arp.arp_sha, senderMAC, 6);
        memcpy(detail.arp.arp_spa, senderIP, 4);
        memcpy(detail.arp.arp_sha, targetMAC, 6);
        memcpy(detail.arp.arp_spa, targetIP, 4);
        printf("Success\n");

        arp = true;        


        //Construct the ethernet header
        //Destination
        memcpy(ethernetHeader.ether_dhost, request.ethernetHeader.ether_shost, 6);
        
        //Host
        memcpy(ethernetHeader.ether_shost, senderMAC, 6);

        //Type
        ethernetHeader.ether_type = ARPOP_REPLY;


        //Copy the data to the data array
        memcpy(this->data, &ethernetHeader, ETHER_LEN);
        memcpy(this->data, &detail.arp, ARP_LEN);

    }

    Packet& Packet::operator=(Packet other){
        std::swap(this->detail,other.detail);
        std::swap(this->data, other.data);
        std::swap(this->ethernetHeader, other.ethernetHeader);
        std::swap(this->ipHeader, other.ipHeader);
        std::swap(this->arp, other.arp);

        return *this;
    }


    Packet Packet::constructResponseARP(struct ifaddrs* interfaceList){
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


                            Packet response(detail.arp.arp_tpa, targetMAC, senderIP, senderMAC, *this);
                            //Temp test
                            return response;
                            //Construct the response packet
                            //Target becomes the sender and the sender becomes the target.
                        }
                    }
                    break;
                }
            }
        }
        throw 2;
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

    void Packet::transferMAC(uint8_t* responseMAC, uint8_t* requestMAC){
        for(int octet = 0; octet < 6; ++octet){
            responseMAC[octet] = requestMAC[octet];
        }
    }

    void Packet::transferIP(uint8_t* responseIP, uint8_t* requestIP){
        for(int octet = 0; octet < 4; ++octet){
            responseIP[octet] = requestIP[octet];
        }
    }
}

