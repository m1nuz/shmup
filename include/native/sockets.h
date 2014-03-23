#pragma once

#include <stdint.h>

#define NET_MAX_PACKET_SIZE 1024

enum
{
    NET_TYPE_UNKNOWN = 0,
    NET_TYPE_IPv4 = 1,
    NET_TYPE_IPv6 = 2
};

typedef struct
{
    uint32_t type;
    uint8_t ip[16];
    uint16_t port;
} NET_ADDRESS;

int net_init();
int net_cleanup();

int net_errno();
const char* net_error();

int udp_create(NET_ADDRESS *_addr);
int udp_open();
int udp_close(int _sock);
int udp_send(int _sock, NET_ADDRESS *_addr, const void *_data, size_t _size);
int udp_recv(int _sock, NET_ADDRESS *_addr, void *_data, size_t _max_size);

int tcp_create(NET_ADDRESS *_addr);
int tcp_listen(int _sock, int _backlog);
int tcp_accept(int _sock, int *_new_sock, NET_ADDRESS *_addr);
int tcp_connect(int _sock, NET_ADDRESS *_addr);
int tcp_send(int _sock, const void *_datajn, int _size);
int tcp_recv(int _sock, void *_data, int _max_size);
int tcp_close(int sock);
