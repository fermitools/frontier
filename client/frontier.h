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

#ifndef __HEADER_H_FRONTIER_H
#define __HEADER_H_FRONTIER_H

#include <sys/types.h>

#define FRONTIER_OK		0
#define FRONTIER_EIARG		-1	/*Invalid argument passed*/
#define FRONTIER_EMEM		-2	/*mem_alloc failed*/
#define FRONTIER_ECURLI		-3	/*lubcurl failed to init*/
#define FRONTIER_ECHAN		-4	/*error while creating channel*/
#define FRONTIER_EURL		-5	/*error while setting URL*/
#define FRONTIER_EUNKNOWN	-6	/*unknown error*/
#define FRONTIER_ENON200	-7	/*HTTP response code != 200*/
#define FRONTIER_XMLPARSE	-8	/*error while parsing XML response*/
#define FRONTIER_EBASE64	-9	/*base64 decode failed*/
#define FRONTIER_ENOROW		-10	/*end of RS*/
#define FRONTIER_ENORS		-11	/*no such RS*/
#define FRONTIER_ENOTINIT	-12	/*frontier was not initialized*/
#define FRONTIER_EMD5		-13	/*MD5 digest mismatch*/

#define FRONTIER_EEND		-100	/*error values less than (-100) are libcurl errors*/

#define FRONTIER_SEMPTY		0
#define FRONTIER_SRAW_DATA	1
#define FRONTIER_SERROR		2

#define FRONTIER_MAX_PAYLOADNUM	32

typedef unsigned long FrontierChannel;

struct s_FrontierRespStat
 {
  int status;
  int error;
  int http_resp_code;
  size_t raw_data_size;
  char *raw_data_buf;
 };
typedef struct s_FrontierRespStat FrontierRespStat;

struct s_FrontierRSBlob
 {
  const char *buf;
  unsigned int size;
  unsigned int pos;
  unsigned int nrec;
 };
typedef struct s_FrontierRSBlob FrontierRSBlob;


FrontierRSBlob *frontierRSBlob_get(FrontierChannel u_channel,int n,int *ec);
void frontierRSBlob_close(FrontierRSBlob *rs,int *ec);
void frontierRSBlob_rsctl(FrontierRSBlob *rs,int ctl_fn,void *data,int size,int *ec);
void frontierRSBlob_start(FrontierRSBlob *rs,int *ec);
char frontierRSBlob_getByte(FrontierRSBlob *rs,int *ec);
int frontierRSBlob_getInt(FrontierRSBlob *rs,int *ec);
long long frontierRSBlob_getLong(FrontierRSBlob *rs,int *ec);
double frontierRSBlob_getDouble(FrontierRSBlob *rs,int *ec);

int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr));

FrontierChannel frontier_createChannel(int *ec);
void frontier_closeChannel(FrontierChannel chn);
void frontier_setProxy(FrontierChannel u_channel,const char *proxy,int *ec);
int frontier_getRawData(FrontierChannel chn,const char *url);
void frontier_getRespStat(FrontierChannel chn,FrontierRespStat *stat);
const char *frontier_getHttpHeaderName(FrontierChannel c,int num);
const char *frontier_getHttpHeaderValue(FrontierChannel c,int num);
const char *frontier_error_desc(int err);

int frontier_n2h_i32(const void* p);
float frontier_n2h_f32(const void* p);
long long frontier_n2h_i64(const void* p);
double frontier_n2h_d64(const void* p);

size_t frontier_md5_get_ctx_size();
void frontier_md5_init(void *ctx);
void frontier_md5_update(void *ctx,const unsigned char *data,unsigned int len);
void frontier_md5_final(void *ctx,unsigned char *out);


#endif /*__HEADER_H_FRONTIER_H*/

