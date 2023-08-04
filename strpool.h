#pragma once

#include <stdbool.h>
#include <ctype.h>
#include <sys/queue.h>

//#define HASH_BLOOM 20
#define HASH_NONFATAL_OOM 1
#define HASH_FUNCTION HASH_BER
#include "uthash.h"

typedef struct strpool {
    char *str;           /* key */
    UT_hash_handle hh;         /* makes this structure hashable */
} strpool;

void StrPoolDestroy(strpool **p);
bool StrPoolAddStr(strpool **pp,const char *s);
bool StrPoolAddStrLen(strpool **pp,const char *s,size_t slen);
bool StrPoolCheckStr(strpool *p,const char *s);

struct str_list {
	char *str;
	LIST_ENTRY(str_list) next;
};
LIST_HEAD(str_list_head, str_list);

bool strlist_add(struct str_list_head *head, const char *filename);
void strlist_destroy(struct str_list_head *head);
