#include "conntrack.h"
#include "darkmagic.h"
#include <arpa/inet.h>
#include <stdio.h>

#undef uthash_nonfatal_oom
#define uthash_nonfatal_oom(elt) ut_oom_recover(elt)

static bool oom = false;
static void ut_oom_recover(void *elem)
{
	oom = true;
}

static const char *connstate_s[]={"SYN","ESTABLISHED","FIN"};

static void connswap(const t_conn *c, t_conn *c2)
{
	memset(c2,0,sizeof(*c2));
	c2->l3proto = c->l3proto;
	c2->l4proto = c->l4proto;
	c2->src = c->dst;
	c2->dst = c->src;
	c2->sport = c->dport;
	c2->dport = c->sport;
}


static void ConntrackPoolDestroyPool(t_conntrack_pool **pp)
{
	t_conntrack_pool *elem, *tmp;
	HASH_ITER(hh, *pp, elem, tmp) { HASH_DEL(*pp, elem); free(elem); }
}
void ConntrackPoolDestroy(t_conntrack *p)
{
	ConntrackPoolDestroyPool(&p->pool);
}

void ConntrackPoolInit(t_conntrack *p, time_t purge_interval, uint32_t timeout_syn, uint32_t timeout_established, uint32_t timeout_fin, uint32_t timeout_udp)
{
	p->timeout_syn = timeout_syn;
	p->timeout_established = timeout_established;
	p->timeout_fin = timeout_fin;
	p->timeout_udp= timeout_udp;
	p->t_purge_interval = purge_interval;
	time(&p->t_last_purge);
	p->pool = NULL;
}

void ConntrackExtractConn(t_conn *c, bool bReverse, const struct ip *ip, const struct ip6_hdr *ip6, const struct tcphdr *tcphdr, const struct udphdr *udphdr)
{
	memset(c,0,sizeof(*c));
	if (ip)
	{
		c->l3proto = IPPROTO_IP;
		c->dst.ip = bReverse ? ip->ip_src : ip->ip_dst;
		c->src.ip = bReverse ? ip->ip_dst : ip->ip_src;
	}
	else if (ip6)
	{
		c->l3proto = IPPROTO_IPV6;
		c->dst.ip6 = bReverse ? ip6->ip6_src : ip6->ip6_dst;
		c->src.ip6 = bReverse ? ip6->ip6_dst : ip6->ip6_src;
	}
	else
		c->l3proto = -1;
	extract_ports(tcphdr, udphdr, &c->l4proto, bReverse ? &c->dport : &c->sport, bReverse ? &c->sport : &c->dport);
}


static t_conntrack_pool *ConntrackPoolSearch(t_conntrack_pool *p, const t_conn *c)
{
	t_conntrack_pool *t;
	HASH_FIND(hh, p, c, sizeof(*c), t);
	return t;
}

static void ConntrackInitTrack(t_ctrack *t)
{
	memset(t,0,sizeof(*t));
	t->scale_orig = t->scale_reply = SCALE_NONE;
	time(&t->t_start);
}


static t_conntrack_pool *ConntrackNew(t_conntrack_pool **pp, const t_conn *c)
{
	t_conntrack_pool *new;
	if (!(new = malloc(sizeof(*new)))) return NULL;
	new->conn = *c;
	oom = false;
	HASH_ADD(hh, *pp, conn, sizeof(*c), new);
	if (oom) { free(new); return NULL; }
	ConntrackInitTrack(&new->track);
	return new;
}

// non-tcp packets are passed with tcphdr=NULL but len_payload filled
static void ConntrackFeedPacket(t_ctrack *t, bool bReverse, const struct tcphdr *tcphdr, uint32_t len_payload)
{
	uint8_t scale;

	if (bReverse)
	{
		t->pcounter_reply++;
		t->pdcounter_reply+=!!len_payload;
		
	}
	else
	{
		t->pcounter_orig++;
		t->pdcounter_orig+=!!len_payload;
	}

	if (tcphdr)
	{
		if (tcp_syn_segment(tcphdr))
		{
			if (t->state!=SYN) ConntrackInitTrack(t); // erase current entry
			t->seq0 = htonl(tcphdr->th_seq);
		}
		else if (tcp_synack_segment(tcphdr))
		{
			if (t->state!=SYN) ConntrackInitTrack(t); // erase current entry
				if (!t->seq0) t->seq0 = htonl(tcphdr->th_ack)-1;
			t->ack0 = htonl(tcphdr->th_seq);
		}
		else if (tcphdr->th_flags & (TH_FIN|TH_RST))
		{
			t->state = FIN;
		}
		else
		{
			if (t->state==SYN) 
			{
				t->state=ESTABLISHED;
				if (!bReverse && !t->ack0) t->ack0 = htonl(tcphdr->th_ack)-1;
			}
		}
		scale = tcp_find_scale_factor(tcphdr);
		if (bReverse)
		{
			t->pos_orig = t->seq_last = htonl(tcphdr->th_ack);
			t->ack_last = htonl(tcphdr->th_seq);
			t->pos_reply = t->ack_last + len_payload;
			t->winsize_reply = htons(tcphdr->th_win);
			if (scale!=SCALE_NONE) t->scale_reply = scale;
			
		}
		else
		{
			t->seq_last = htonl(tcphdr->th_seq);
			t->pos_orig = t->seq_last + len_payload;
			t->pos_reply = t->ack_last = htonl(tcphdr->th_ack);
			t->winsize_orig = htons(tcphdr->th_win);
			if (scale!=SCALE_NONE) t->scale_orig = scale;
		}
	}
	else
	{
		if (bReverse)
		{
			t->ack_last=t->pos_reply;
			t->pos_reply+=len_payload;
		}
		else
		{
			t->seq_last=t->pos_orig;
			t->pos_orig+=len_payload;
		}
	}

	time(&t->t_last);
}


static bool ConntrackPoolFeedPool(t_conntrack_pool **pp, const struct ip *ip, const struct ip6_hdr *ip6, const struct tcphdr *tcphdr, const struct udphdr *udphdr, size_t len_payload, t_ctrack **ctrack, bool *bReverse)
{
	t_conn conn, connswp;
	t_conntrack_pool *ctr;
	bool b_rev;

	ConntrackExtractConn(&conn,false,ip,ip6,tcphdr,udphdr);
	if ((ctr=ConntrackPoolSearch(*pp,&conn)))
	{
		ConntrackFeedPacket(&ctr->track, (b_rev=false), tcphdr, len_payload);
		goto ok;
	}
	else
	{
		connswap(&conn,&connswp);
		if ((ctr=ConntrackPoolSearch(*pp,&connswp)))
		{
			ConntrackFeedPacket(&ctr->track, (b_rev=true), tcphdr, len_payload);
			goto ok;
		}
	}
	b_rev = tcphdr && tcp_synack_segment(tcphdr);
	if (tcphdr && tcp_syn_segment(tcphdr) || b_rev || udphdr)
	{
		if ((ctr=ConntrackNew(pp, b_rev ? &connswp : &conn)))
		{
			ConntrackFeedPacket(&ctr->track, b_rev, tcphdr, len_payload);
			goto ok;
		}
	}
	return false;
ok:
	if (ctrack) *ctrack = &ctr->track;
	if (bReverse) *bReverse = b_rev;
	return true;
}
bool ConntrackPoolFeed(t_conntrack *p, const struct ip *ip, const struct ip6_hdr *ip6, const struct tcphdr *tcphdr, const struct udphdr *udphdr, size_t len_payload, t_ctrack **ctrack, bool *bReverse)
{
	return ConntrackPoolFeedPool(&p->pool,ip,ip6,tcphdr,udphdr,len_payload,ctrack,bReverse);
}

static bool ConntrackPoolDropPool(t_conntrack_pool **pp, const struct ip *ip, const struct ip6_hdr *ip6, const struct tcphdr *tcphdr, const struct udphdr *udphdr)
{
	t_conn conn, connswp;
	t_conntrack_pool *t;
	ConntrackExtractConn(&conn,false,ip,ip6,tcphdr,udphdr);
	if (!(t=ConntrackPoolSearch(*pp,&conn)))
	{
		connswap(&conn,&connswp);
		t=ConntrackPoolSearch(*pp,&connswp);
	}
	if (!t) return false;
	HASH_DEL(*pp, t); free(t);
	return true;
}
bool ConntrackPoolDrop(t_conntrack *p, const struct ip *ip, const struct ip6_hdr *ip6, const struct tcphdr *tcphdr, const struct udphdr *udphdr)
{
	return ConntrackPoolDropPool(&p->pool,ip,ip6,tcphdr,udphdr);
}

void ConntrackPoolPurge(t_conntrack *p)
{
	time_t tidle, tnow = time(NULL);
	t_conntrack_pool *t, *tmp;

	if ((tnow - p->t_last_purge)>=p->t_purge_interval)
	{
		HASH_ITER(hh, p->pool , t, tmp) {
			tidle = tnow - t->track.t_last;
			if (	t->track.b_cutoff ||
				t->conn.l4proto==IPPROTO_TCP && (
					t->track.state==SYN && tidle>=p->timeout_syn ||
					t->track.state==ESTABLISHED && tidle>=p->timeout_established ||
					t->track.state==FIN && tidle>=p->timeout_fin) ||
				t->conn.l4proto==IPPROTO_UDP &&
					tidle>=p->timeout_udp)
			{
				HASH_DEL(p->pool, t); free(t); 
			}
		}
		p->t_last_purge = tnow;
	}
}

static void taddr2str(uint8_t l3proto, const t_addr *a, char *buf, size_t bufsize)
{
	if (!inet_ntop(family_from_proto(l3proto), a, buf, bufsize) && bufsize) *buf=0;
}

void ConntrackPoolDump(const t_conntrack *p)
{
	t_conntrack_pool *t, *tmp;
	char sa1[40],sa2[40];
	time_t tnow = time(NULL);
	HASH_ITER(hh, p->pool, t, tmp) {
		taddr2str(t->conn.l3proto, &t->conn.src, sa1, sizeof(sa1));
		taddr2str(t->conn.l3proto, &t->conn.dst, sa2, sizeof(sa2));
		if (t->conn.l4proto==IPPROTO_TCP)
			printf("%s [%s]:%u => [%s]:%u : %s : t0=%llu last=t0+%llu now=last+%llu packets_orig=d%llu/n%llu packets_reply=d%llu/n%llu seq0=%u rseq=%u pos_orig=%u ack0=%u rack=%u pos_reply=%u wsize_orig=%u:%d wsize_reply=%u:%d",
				proto_name(t->conn.l4proto),
				sa1, t->conn.sport, sa2, t->conn.dport,
				t->conn.l4proto==IPPROTO_TCP ? connstate_s[t->track.state] : "-",
				(unsigned long long)t->track.t_start, (unsigned long long)(t->track.t_last - t->track.t_start), (unsigned long long)(tnow - t->track.t_last),
				(unsigned long long)t->track.pdcounter_orig, (unsigned long long)t->track.pcounter_orig,
				(unsigned long long)t->track.pdcounter_reply, (unsigned long long)t->track.pcounter_reply,
				t->track.seq0, t->track.seq_last - t->track.seq0, t->track.pos_orig - t->track.seq0,
				t->track.ack0, t->track.ack_last - t->track.ack0, t->track.pos_reply - t->track.ack0,
				t->track.winsize_orig, t->track.scale_orig==SCALE_NONE ? -1 : t->track.scale_orig,
				t->track.winsize_reply, t->track.scale_reply==SCALE_NONE ? -1 : t->track.scale_reply);
		else
			printf("%s [%s]:%u => [%s]:%u : %s : t0=%llu last=t0+%llu now=last+%llu packets_orig=d%llu/n%llu packets_reply=d%llu/n%llu rseq=%u pos_orig=%u rack=%u pos_reply=%u",
				proto_name(t->conn.l4proto),
				sa1, t->conn.sport, sa2, t->conn.dport,
				t->conn.l4proto==IPPROTO_TCP ? connstate_s[t->track.state] : "-",
				(unsigned long long)t->track.t_start, (unsigned long long)(t->track.t_last - t->track.t_start), (unsigned long long)(tnow - t->track.t_last),
				(unsigned long long)t->track.pdcounter_orig, (unsigned long long)t->track.pcounter_orig,
				(unsigned long long)t->track.pdcounter_reply, (unsigned long long)t->track.pcounter_reply,
				t->track.seq_last, t->track.pos_orig,
				t->track.ack_last, t->track.pos_reply);
		printf("  cutoff=%u wss_cutoff=%u d_cutoff=%u\n",
			t->track.b_cutoff, t->track.b_wssize_cutoff, t->track.b_desync_cutoff);
	};
}
