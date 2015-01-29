#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "socket.h"
#include "packet.h"
#include "config.h"

ssize_t packet_send(int sock, void *data, size_t size) {
    ssize_t ret;
    char *packet;
    size_t packet_size;
    uint32_t data_size;

    packet_size = sizeof(uint32_t) + size;

    data_size = htonl((uint32_t) size);
    packet = calloc(1, packet_size);

    printf("packet_send(): DATA = %lu bytes / Packet = %lu bytes\n", size, packet_size);

    memcpy(packet, &data_size, sizeof(uint32_t));
    memcpy((packet + sizeof(uint32_t)), data, size);

    if (socket_send(sock, packet, packet_size, 0) != packet_size)
        ret = -1;
    else
        ret = size;

    free(packet);

    return ret;
}

ssize_t packet_receive(int sock, char *data) {
    char buf[MAX_BUF];
    size_t packet_size;
    uint32_t data_size;

    if (socket_receive(sock, buf, sizeof(uint32_t), MSG_PEEK) != sizeof(uint32_t))
        return -1;

    data_size = ntohl(*(uint32_t *) buf);
    packet_size = sizeof(uint32_t) + data_size;

    printf("packet_receive(): DATA = %u bytes / Packet = %lu bytes\n", data_size, packet_size);

    if (socket_receive(sock, buf, packet_size, 0) != packet_size)
        return -1;

    memcpy(data, (buf + sizeof(uint32_t)), data_size);

    return data_size;
}