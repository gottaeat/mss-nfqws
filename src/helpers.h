#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stddef.h>
#include <stdbool.h>

#include "params.h"

void hexdump_limited_dlog(const uint8_t *data, size_t size, size_t limit);
char *strncasestr(const char *s,const char *find, size_t slen);
bool load_file(const char *filename,void *buffer,size_t *buffer_size);
bool load_file_nonempty(const char *filename,void *buffer,size_t *buffer_size);
bool save_file(const char *filename, const void *buffer, size_t buffer_size);

void print_sockaddr(const struct sockaddr *sa);
void ntop46(const struct sockaddr *sa, char *str, size_t len);
void ntop46_port(const struct sockaddr *sa, char *str, size_t len);

void dbgprint_socket_buffers(int fd);
bool set_socket_buffers(int fd, int rcvbuf, int sndbuf);

uint64_t pntoh64(const void *p);
void phton64(uint8_t *p, uint64_t v);

static inline uint16_t pntoh16(const uint8_t *p) {
	return ((uint16_t)p[0] << 8) | (uint16_t)p[1];
}
static inline void phton16(uint8_t *p, uint16_t v) {
	p[0] = (uint8_t)(v >> 8);
	p[1] = v & 0xFF;
}
static inline uint32_t pntoh32(const uint8_t *p) {
	return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}
