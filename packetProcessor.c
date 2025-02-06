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

#include "packetProcessor.h"

#include "sqlite/sqlite3.h"

// #define BUFFER_SIZE 65536

// struct record {
//     char* dest_ip;
//     char* source_ip;
//     char* source_mac;
//     char* dest_mac;
//     int source_port;
//     int dest_port;
//     char* protocol;
//     char* payload;
// };

// typedef struct record Record;

Record records[BUFFER_SIZE];

// Function to print MAC address
void print_mac(const unsigned char *mac) {
    for (int i = 0; i < 6; i++) {
        printf("%02x%s", mac[i], (i < 5) ? ":" : "");
    }
}

char* get_mac_address(char *if_name) {
    struct ifreq ifr;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, if_name, IF_NAMESIZE-1);

    ioctl(fd, SIOCGIFHWADDR, &ifr);
    close(fd);

    unsigned char *mac = (unsigned char *)ifr.ifr_hwaddr.sa_data;
    char *mac_str = (char *)malloc(18);
    sprintf(mac_str, "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return mac_str;
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

char* get_payload(const unsigned char *payload, int size) {
    char *payload_str = (char *)malloc(size * 2 + 1); // Allocate enough space
    if (!payload_str) {
        perror("Failed to allocate memory for payload");
        return NULL;
    }
    for (int i = 0; i < size; i++) {
        if (payload[i] >= 32 && payload[i] <= 126) {
            sprintf(payload_str + strlen(payload_str), "%c", payload[i]);
        } else {
            sprintf(payload_str + strlen(payload_str), ".");
        }
    }
    return payload_str;
}

char* get_protocol(int protocol) {
    switch (protocol) {
        case IPPROTO_TCP:
            return "TCP";
        case IPPROTO_UDP:
            return "UDP";
        case IPPROTO_ICMP:
            return "ICMP";
        default:
            return "UNKNOWN";
    }
}

int initSqliteDb() {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open("packet_log.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char *sql = "CREATE TABLE IF NOT EXISTS records ("
                "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                "source_ip TEXT,"
                "dest_ip TEXT,"
                "source_mac TEXT,"
                "dest_mac TEXT,"    
                "source_port TEXT,"
                "dest_port TEXT,"
                "protocol TEXT,"
                "payload TEXT);";
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);

    return 0;
}

int insertRecord(Record* record) {
    sqlite3 *db;
    char *err_msg = 0;

    int rc = sqlite3_open("packet_log.db", &db);

    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    char sourceIp[128];
    char destIp[128];
    char sourceMac[128];
    char destMac[128];
    char protocol[128];
    char payloadBuffer[BUFFER_SIZE];

    strcpy(sourceIp, record->source_ip);
    strcpy(destIp, record->dest_ip);
    strcpy(sourceMac, record->source_mac);
    strcpy(destMac, record->dest_mac);
    strcpy(protocol, record->protocol);
    strcpy(payloadBuffer, record->payload);

    char sql[BUFFER_SIZE];
    sprintf(sql, "INSERT INTO records (source_ip, dest_ip, source_mac, dest_mac, source_port, dest_port, protocol, payload) "
                 "VALUES ('%s', '%s', '%s', '%s', '%d', '%d', '%s', '%s');",
                 sourceIp, destIp, sourceMac, destMac, record->source_port, record->dest_port, protocol, payloadBuffer);
    
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);

    if (rc != SQLITE_OK) {
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }

    sqlite3_close(db);

    return 0;
}

// Function to process packets
void process_packet(const unsigned char *buffer, int size) {
    struct ethhdr *eth = (struct ethhdr *)buffer;

    Record* record = (Record *)malloc(sizeof(Record));

    struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));
    struct sockaddr_in dest;
    dest.sin_addr.s_addr = ip_header->daddr;

    // if (strcmp(inet_ntoa(dest.sin_addr), "127.0.0.1") == 0) {
    //     return;
    // }

    // printf("\n=== Data Link Layer ===\n");
    // printf("Source MAC: ");
    // print_mac(eth->h_source);
    // printf("\nDestination MAC: ");
    // print_mac(eth->h_dest);
    // printf("\nEther Type: 0x%04x\n", ntohs(eth->h_proto));

    record->source_mac = get_mac_address(eth->h_source);
    record->dest_mac = get_mac_address(eth->h_dest);

    if (ntohs(eth->h_proto) == ETH_P_IP) {
        struct iphdr *ip_header = (struct iphdr *)(buffer + sizeof(struct ethhdr));

        struct sockaddr_in src, dest;
        src.sin_addr.s_addr = ip_header->saddr;
        dest.sin_addr.s_addr = ip_header->daddr;

        // printf("\n=== Network Layer ===\n");
        // printf("Source IP: %s\n", inet_ntoa(src.sin_addr));
        // printf("Destination IP: %s\n", inet_ntoa(dest.sin_addr));

        record->source_ip = inet_ntoa(src.sin_addr);
        record->dest_ip = inet_ntoa(dest.sin_addr);

        if (ip_header->protocol == IPPROTO_ICMP) {
            // printf("\n=== Transport Layer (ICMP) ===\n");
            // printf("ICMP Packet\n");
            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4));
            // if (payload_size > 0) {
            //     print_payload(payload, payload_size);
            // }

            char* icmp = "ICMP";

            record->protocol = icmp;
            record->payload = get_payload(payload, payload_size);

            insertRecord(record);
            
            free(record->payload);
            free(record->source_mac);
            free(record->dest_mac);
            free(record);

            return;
        }

        if (ip_header->protocol == IPPROTO_TCP) {
            struct tcphdr *tcp_header = (struct tcphdr *)(buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4));
            // printf("\n=== Transport Layer (TCP) ===\n");
            // printf("Source Port: %u\n", ntohs(tcp_header->source));
            // printf("Destination Port: %u\n", ntohs(tcp_header->dest));

            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4) + (tcp_header->doff * 4);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4) + (tcp_header->doff * 4));

            // if (payload_size > 0) {
            //     printf("\n=== Application Layer ===\n");
            //     print_payload(payload, payload_size);
            // }

            char* tcp = "TCP";

            record->source_port = ntohs(tcp_header->source);
            record->dest_port = ntohs(tcp_header->dest);
            record->protocol = tcp;
            record->payload = get_payload(payload, payload_size);

            insertRecord(record);
            
            free(record->payload);
            free(record->source_mac);
            free(record->dest_mac);
            free(record);

        } else if (ip_header->protocol == IPPROTO_UDP) {
            struct udphdr *udp_header = (struct udphdr *)(buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4));
            // printf("\n=== Transport Layer (UDP) ===\n");
            // printf("Source Port: %u\n", ntohs(udp_header->source));
            // printf("Destination Port: %u\n", ntohs(udp_header->dest));

            const unsigned char *payload = buffer + sizeof(struct ethhdr) + (ip_header->ihl * 4) + sizeof(struct udphdr);
            int payload_size = size - (sizeof(struct ethhdr) + (ip_header->ihl * 4) + sizeof(struct udphdr));

            // if (payload_size > 0) {
            //     printf("\n=== Application Layer ===\n");
            //     print_payload(payload, payload_size);
            // }

            char* udp = "UDP";

            record->source_port = ntohs(udp_header->source);
            record->dest_port = ntohs(udp_header->dest);
            record->protocol = udp;
            record->payload = get_payload(payload, payload_size);

            insertRecord(record);
            
            free(record->payload);
            free(record->source_mac);
            free(record->dest_mac);
            free(record);
        }
    }
}

int initMain() {
    int raw_socket;
    struct sockaddr saddr;
    socklen_t saddr_size;
    unsigned char *buffer = (unsigned char *)malloc(BUFFER_SIZE);

    if (initSqliteDb("packet_log.db") != 0) {
        return 1;
    }

    // Redirect stdout to a file
    // FILE *log_file = freopen("./packet_log.txt", "w", stdout);
    // if (log_file == NULL) {
    //     perror("Failed to redirect stdout to file");
    //     return 1;
    // }

    // Create a raw socket
    raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (raw_socket < 0) {
        perror("Socket creation failed");
        return 1;
    }
    
    while (1) {
        saddr_size = sizeof(saddr);
        int data_size = recvfrom(raw_socket, buffer, BUFFER_SIZE, 0, &saddr, &saddr_size);
        // printf("\nRecieved Data of size: %d\n", data_size);
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