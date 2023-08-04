#pragma once

#include "params.h"
#include "strpool.h"
#include "conntrack.h"
#include "desync.h"

#include <sys/param.h>
#include <sys/types.h>
#include <net/if.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define Q_RCVBUF	(128*1024)	// in bytes
#define Q_SNDBUF	(64*1024)	// in bytes
#define RAW_SNDBUF	(64*1024)	// in bytes

#define Q_MAXLEN	1024		// in packets

#define BADSEQ_INCREMENT_DEFAULT 	-10000
#define BADSEQ_ACK_INCREMENT_DEFAULT 	-66000

#define IPFRAG_UDP_DEFAULT 8
#define IPFRAG_TCP_DEFAULT 32

#define UDPLEN_INCREMENT_DEFAULT 	2

struct params_s
{
	bool debug;
	uint16_t wsize,wssize;
	uint8_t wscale,wsscale;
	char wssize_cutoff_mode; // n - packets, d - data packets, s - relative sequence
	unsigned int wssize_cutoff;
#ifdef __linux__
	int qnum;
#elif defined(BSD)
	uint16_t port; // divert port
#endif
	char bind_fix4,bind_fix6;
	bool hostcase, hostnospace, domcase;
	char hostspell[4];
	enum dpi_desync_mode desync_mode0,desync_mode,desync_mode2;
	bool desync_retrans,desync_skip_nosni,desync_any_proto;
	unsigned int desync_repeats,desync_split_pos,desync_ipfrag_pos_tcp,desync_ipfrag_pos_udp;
	char desync_cutoff_mode; // n - packets, d - data packets, s - relative sequence
	unsigned int desync_cutoff;
	uint8_t desync_ttl, desync_ttl6;
	uint8_t desync_fooling_mode;
	uint32_t desync_fwmark; // unused in BSD
	uint32_t desync_badseq_increment, desync_badseq_ack_increment;
	uint8_t fake_http[1432],fake_tls[1432],fake_unknown[1432],fake_unknown_udp[1472],fake_quic[1472];
	size_t fake_http_size,fake_tls_size,fake_unknown_size,fake_unknown_udp_size,fake_quic_size;
	int udplen_increment;
	bool droproot;
	uid_t uid;
	gid_t gid;

	strpool *hostlist, *hostlist_exclude;
	struct str_list_head hostlist_files, hostlist_exclude_files;

	unsigned int ctrack_t_syn, ctrack_t_est, ctrack_t_fin, ctrack_t_udp;
	t_conntrack conntrack;
};

extern struct params_s params;

#define DLOG(format, ...) {if (params.debug) printf(format, ##__VA_ARGS__);}
