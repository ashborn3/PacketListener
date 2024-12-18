#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/if_packet.h>

#define BUFFER_SIZE 65536

// Function to print MAC address
void print_mac(const unsigned char *mac) {
    for (int i = 0; i < 6; i++) {
        printf("%02x%s", mac[i], (i < 5) ? ":" : "");
    }
}

// Function to print payload in human-readable format
void print_payload(const unsigned char *payload, int size) {
    printf("Payload (ASCII):\n");
    for (int i = 0; i < size; i++) {
        if (payload[i] >= 32 && payload[i] <= 126) {
            printf("%c", payload[i]);
        } else {
            printf(".");
        }
    }
    printf("\n");
}

// Function to process packets
void process_packet(const unsigned char *buffer, int size) {
    struct ethhdr *eth = (struct ethhdr *)buffer;

    struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    struct sockaddr_in dest;
    dest.sin_addr.s_addr = ip_header->daddr;

    if (strcmp(inet_ntoa(dest.sin_addr), "127.0.0.1") == 0) {
        return;
    }

    printf("\n=== Data Link Layer ===\n");
    printf("Source MAC: ");
    print_mac(eth->h_source);
    printf("\nDestination MAC: ");
    print_mac(eth->h_dest);
    printf("\nEther Type: 0x%04x\n", ntohs(eth->h_proto));

    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));

        struct sockaddr_in src, dest;
        src.sin_addr.s_addr = ip_header->saddr;
        dest.sin_addr.s_addr = ip_header->daddr;

        printf("\n=== Network Layer ===\n");
        printf("Source IP: %s\n", inet_ntoa(src.sin_addr));
        printf("Destination IP: %s\n", inet_ntoa(dest.sin_addr));

        if (ip_header->protocol == IPPROTO_ICMP) {
            printf("\n=== Transport Layer (ICMP) ===\n");
            printf("ICMP Packet\n");
            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4));
            if (payload_size > 0) {
                print_payload(payload, payload_size);
            }
            return;
        }

        if (ip_header->protocol == IPPROTO_TCP) {
            struct tcphdr *tcp_header = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4));
            printf("\n=== Transport Layer (TCP) ===\n");
            printf("Source Port: %u\n", ntohs(tcp_header->source));
            printf("Destination Port: %u\n", ntohs(tcp_header->dest));

            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4) + (tcp_header->doff * 4);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4) + (tcp_header->doff * 4));

            if (payload_size > 0) {
                printf("\n=== Application Layer ===\n");
                print_payload(payload, payload_size);
            }

        } else if (ip_header->protocol == IPPROTO_UDP) {
            struct udphdr *udp_header = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4));
            printf("\n=== Transport Layer (UDP) ===\n");
            printf("Source Port: %u\n", ntohs(udp_header->source));
            printf("Destination Port: %u\n", ntohs(udp_header->dest));

            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4) + sizeof(struct udphdr);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4) + sizeof(struct udphdr));

            if (payload_size > 0) {
                printf("\n=== Application Layer ===\n");
                print_payload(payload, payload_size);
            }
        }
    }
}

int main() {
    int raw_socket;
    struct sockaddr saddr;
    socklen_t saddr_size;
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    // Redirect stdout to a file
    FILE *log_file = freopen("./packet_log.txt", "w", stdout);
    if (log_file == NULL) {
        perror("Failed to redirect stdout to file");
        return 1;
    }

    // Create a raw socket
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }

    printf("Starting packet capture...\n");

    while (1) {
        saddr_size = sizeof(saddr);
        int data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, &saddr, &saddr_size);
        if (data_size < 0) {
            perror("Failed to capture packets");
            break;
        }
        process_packet(buffer, data_size);
    }

    close(raw_socket);
    free(buffer);
    return 0;
}
