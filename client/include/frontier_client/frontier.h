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

#define FRONTIER_LOGLEVEL_DEBUG		0
#define FRONTIER_LOGLEVEL_WARNING	1
#define FRONTIER_LOGLEVEL_INFO		FRONTIER_LOGLEVEL_WARNING
#define FRONTIER_LOGLEVEL_ERROR		2
#define FRONTIER_LOGLEVEL_NOLOG		3

#define FRONTIER_OK		0
#define FRONTIER_EIARG		-1	/*Invalid argument passed*/
#define FRONTIER_EMEM		-2	/*mem_alloc failed*/
#define FRONTIER_ECFG		-3	/*config error*/
#define FRONTIER_ESYS		-4	/*system error*/
#define FRONTIER_EUNKNOWN	-5	/*unknown error*/
#define FRONTIER_ENETWORK	-6	/*error while communicating over network*/
#define FRONTIER_EPROTO		-7	/*protocol level error (e.g. wrong response)*/

#define FRONTIER_MAX_PAYLOADNUM	32

void frontier_setErrorMsg(const char *file, int line,const char *fmt,...);
const char *frontier_get_err_desc(int err_num);
const char *frontier_getErrorMsg();
void frontier_log(int level,const char *file,int line,const char *fmt,...);

#define FRONTIER_MSG(e) do{frontier_setErrorMsg(__FILE__,__LINE__,"error %d: %s",(e),frontier_get_err_desc(e));}while(0)

typedef unsigned long FrontierChannel;


struct s_FrontierRSBlob
 {
  int payload_error;
  const char *payload_msg;
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
char frontierRSBlob_checkByte(FrontierRSBlob *rs,int *ec); // Returns next byte but do not change RS pointer
int frontierRSBlob_getInt(FrontierRSBlob *rs,int *ec);
long long frontierRSBlob_getLong(FrontierRSBlob *rs,int *ec);
double frontierRSBlob_getDouble(FrontierRSBlob *rs,int *ec);
float frontierRSBlob_getFloat(FrontierRSBlob *rs,int *ec);
void frontierRSBlob_getArea(FrontierRSBlob *rs,char *p,unsigned int len,int *ec);

int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr));

FrontierChannel frontier_createChannel(const char *srv,const char *proxy,int *ec);
void frontier_closeChannel(FrontierChannel chn);
void frontier_setReload(FrontierChannel u_channel,int reload);
int frontier_getRawData(FrontierChannel chn,const char *uri);
int frontier_postRawData(FrontierChannel chn,const char *uri,const char *body);

int frontier_n2h_i32(const void* p);
float frontier_n2h_f32(const void* p);
long long frontier_n2h_i64(const void* p);
double frontier_n2h_d64(const void* p);

size_t frontier_md5_get_ctx_size();
void frontier_md5_init(void *ctx);
void frontier_md5_update(void *ctx,const unsigned char *data,unsigned int len);
void frontier_md5_final(void *ctx,unsigned char *out);

char *frontier_str_ncopy(const char *str, size_t len);
char *frontier_str_copy(const char *str);

void *frontier_malloc(size_t size);
void frontier_free(void *ptr);


#endif /*__HEADER_H_FRONTIER_H*/

