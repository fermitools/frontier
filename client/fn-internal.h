/*
 * frontier client internal header
 * 
 * Author: Sergey Kosyakov
 *
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 *
 */

#ifndef __HEADER_H_FN_INTERNAL_H
#define __HEADER_H_FN_INTERNAL_H

#include <http/fn-htclient.h>

#define FRONTIER_ID_SIZE 256

# define FRONTIER_SUCCESS 0
# define FRONTIER_FAILURE 1

#include "fn-base64.h"
#include "fn-md5.h"

struct s_FrontierMemBuf
 {
  struct s_FrontierMemBuf *nextbuf;
  size_t len;
  // data follows immediately after
 };
typedef struct s_FrontierMemBuf FrontierMemBuf;
struct s_FrontierMemData
 {
  FrontierMemBuf *firstbuf;
  FrontierMemBuf *lastbuf;
  size_t total;
  size_t zipped_total;
  int error;
  fn_b64a2b_context b64context;
  unsigned char md5[16];
  struct md5_ctx md5_ctx;
  int binzipped;
  unsigned char zipbuf[4096];
  int zipbuflen;
 };
typedef struct s_FrontierMemData FrontierMemData;
FrontierMemData *frontierMemData_create(int zipped);
int frontierMemData_finalize(FrontierMemData *md);
unsigned char *frontierMemData_getmd5(FrontierMemData *md);
void frontierMemData_delete(FrontierMemData *md);
int frontierMemData_b64append(FrontierMemData *md,const char *buf,int size);


struct s_FrontierPayload
 {
  int id;
  char *encoding;
  int error;
  int error_code;
  char *error_msg;
  unsigned char *blob;
  int blob_size;
  unsigned int nrec;
  long full_size;
  char md5_str[36];
  char srv_md5_str[36];
  FrontierMemData *md;
 };
typedef struct s_FrontierPayload FrontierPayload;
FrontierPayload *frontierPayload_create(const char *encoding);
void frontierPayload_delete(FrontierPayload *pl);
void frontierPayload_append(FrontierPayload *pl,const char *s,int len);
int frontierPayload_finalize(FrontierPayload *pl);

#define FNTR_WITHIN_PAYLOAD	1

struct s_FrontierResponse
 {
  int error;
  int payload_num;
  int error_payload_ind;
  int keepalives;
  void *parser;
  int p_state;
  int zipped;
  int seqnum;
  FrontierPayload *payload[FRONTIER_MAX_PAYLOADNUM];
 };
typedef struct s_FrontierResponse FrontierResponse;
FrontierResponse *frontierResponse_create(int *ec);
void frontierResponse_delete(FrontierResponse *fr);
int FrontierResponse_append(FrontierResponse *fr,char *buf,int len);
int frontierResponse_finalize(FrontierResponse *fr);


#define FRONTIER_ENV_LOG_LEVEL	"FRONTIER_LOG_LEVEL"
#define FRONTIER_ENV_LOG_FILE	"FRONTIER_LOG_FILE"

struct s_fn_hashtable;
struct s_fn_client_cache_list {
    struct s_fn_client_cache_list *next;
    struct s_fn_hashtable *table;
    char *servlet;
};
typedef struct s_fn_client_cache_list fn_client_cache_list;


struct s_Channel
 {
  FrontierConfig *cfg;
  FrontierResponse *resp;
  FrontierHttpClnt *ht_clnt;
  pid_t pid;
  int http_resp_code;
  int reload; // Current reload flag
  int user_reload; // reload flag desired by user
  int seqnum;      // sequence number for the channel
  int response_seqnum; // next sequence number for responses using this channel
  fn_client_cache_list *client_cache;
  char *client_cache_buf;
  char *ttlshort_suffix;
  char *ttllong_suffix;
  int client_cache_maxsize;
 };
typedef struct s_Channel Channel;

struct s_RSBlob
 {
  FrontierResponse *resp;
  int payload_error;
  const char *payload_msg;
  const unsigned char *buf;
  unsigned int size;
  unsigned int pos;
  unsigned int nrec;
  int respnum;
 };
typedef struct s_RSBlob RSBlob;

#endif /*__HEADER_H_FN_INTERNAL_H*/


