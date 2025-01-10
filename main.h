#ifndef MAIN_H
#define MAIN_H

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
#include "sqlite/sqlite3.h"

#define BUFFER_SIZE 65536

struct record {
    char* dest_ip;
    char* source_ip;
    char* source_mac;
    char* dest_mac;
    int source_port;
    int dest_port;
    char* protocol;
    char* payload;
};

typedef struct record Record;

void print_mac(const unsigned char *mac);
char* get_mac_address(char *if_name);
void print_payload(const unsigned char *payload, int size);
char* get_payload(const unsigned char *payload, int size);
char* get_protocol(int protocol);
int initSqliteDb(const char *db_name);
int insertRecord(Record* record);
void process_packet(const unsigned char *buffer, int size);

#endif // MAIN_H