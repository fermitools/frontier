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

#define FRONTIER_MAX_SERVERN	4	// Max number of servers in FRONTIER_SERVER env. variable
#define FRONTIER_MAX_PROXYN	4	// Max number of proxies in FRONTIER_PROXY env. variable

#define FRONTIER_ENV_SERVER	"FRONTIER_SERVER"
#define FRONTIER_ENV_PROXY	"FRONTIER_PROXY"

#define FRONTIER_MAX_REQUEST_URL	4096

struct s_FrontierConfig
 {
  char *server[FRONTIER_MAX_SERVERN];
  char *proxy[FRONTIER_MAX_PROXYN];
  int server_num;
  int proxy_num;
  int server_cur;
  int proxy_cur;
  char buf[FRONTIER_MAX_REQUEST_URL];
 };
typedef struct s_FrontierConfig FrontierConfig;
FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url);
const char *frontierConfig_getServerUrl(FrontierConfig *cfg);
const char *frontierConfig_getRequestUrl(FrontierConfig *cfg,const char *uri,int *ec);
const char *frontierConfig_getProxyUrl(FrontierConfig *cfg);
int frontierConfig_nextServer(FrontierConfig *cfg);
int frontierConfig_nextProxy(FrontierConfig *cfg);
void frontierConfig_delete(FrontierConfig *cfg);


struct s_Channel
 {
  FrontierConfig *cfg;
  FrontierResponse *resp;
  FrontierMemData *md_head;
  void *curl;
  int http_resp_code;
  int status;
  int error;
  int reload;
  const char *http_status_line;
 };
typedef struct s_Channel Channel;

#endif /*__HEADER_H_FN_INTERNAL_H*/


