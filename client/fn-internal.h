/*
 * FroNTier client API
 * 
 * Author: Sergey Kosyakov
 *
 * $Header$
 *
 * $Id$
 *
 */

#ifndef __HEADER_H_FN_INTERNAL_H
#define __HEADER_H_FN_INTERNAL_H


struct s_FrontierMemData
 {
  size_t size;
  size_t len;
  int status;
  char *buf;
 };
typedef struct s_FrontierMemData FrontierMemData;
FrontierMemData *frontierMemData_create();
void frontierMemData_delete(FrontierMemData *md);
int frontierMemData_append(FrontierMemData *md,const char *buf,size_t size);


struct s_FrontierPayload
 {
  int id;
  int encoding;
  int error_code;
  char *error_msg;
  char *blob;
  int blob_size;
  unsigned int nrec;
  unsigned char md5[16];
  char md5_str[36];
  char srv_md5_str[36];
  FrontierMemData *md;
 };
typedef struct s_FrontierPayload FrontierPayload;
FrontierPayload *frontierPayload_create();
void frontierPayload_delete(FrontierPayload *pl);
void frontierPayload_append(FrontierPayload *pl,const char *s,int len);
int frontierPayload_finalize(FrontierPayload *pl);

#define FNTR_WITHIN_PAYLOAD	1

struct s_FrontierResponse
 {
  int payload_num;
  int error;
  int error_code;
  int error_payload_ind;
  void *parser;
  int p_state;
  FrontierPayload *payload[FRONTIER_MAX_PAYLOADNUM];
 };
typedef struct s_FrontierResponse FrontierResponse;
FrontierResponse *frontierResponse_create();
void frontierResponse_delete(FrontierResponse *fr);
int FrontierResponse_append(FrontierResponse *fr,char *buf,int len);
int frontierResponse_finalize(FrontierResponse *fr);


struct s_Channel
 {
  FrontierResponse *resp;
  FrontierMemData *md_head;
  void *curl;
  int http_resp_code;
  int status;
  int error;
  const char *proxy_url;
  int reload;
 };
typedef struct s_Channel Channel;

#endif /*__HEADER_H_FN_INTERNAL_H*/


