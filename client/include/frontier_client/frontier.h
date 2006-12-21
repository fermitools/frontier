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
#include "frontier_config.h"
#include "frontier_log.h"
#include "frontier_error.h"

#define FRONTIER_MAX_PAYLOADNUM	32

typedef unsigned long FrontierChannel;
typedef void FrontierRSBlob;


/*frontierRSBlob_get is deprecated, use frontierRSBlob_open instead*/
FrontierRSBlob *frontierRSBlob_get(FrontierChannel u_channel,int n,int *ec);
FrontierRSBlob *frontierRSBlob_open(FrontierChannel u_channel,FrontierRSBlob *oldrs,int n,int *ec);
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
unsigned int frontierRSBlob_getRecNum(FrontierRSBlob *rs);
unsigned int frontierRSBlob_getPos(FrontierRSBlob *rs);
unsigned int frontierRSBlob_getSize(FrontierRSBlob *rs);
int frontierRSBlob_payload_error(FrontierRSBlob *rs);
const char* frontierRSBlob_payload_msg(FrontierRSBlob *rs);

int frontier_initdebug(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr),
			const char *logfile, const char *loglevel);
int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr));

FrontierChannel frontier_createChannel(const char *srv,const char *proxy,int *ec);
FrontierChannel frontier_createChannel2(FrontierConfig* config, int *ec);
void frontier_closeChannel(FrontierChannel chn);
void frontier_setReload(FrontierChannel u_channel,int reload);
int frontier_getRawData(FrontierChannel chn,const char *uri);
int frontier_postRawData(FrontierChannel chn,const char *uri,const char *body);
int frontier_getRetrieveZipLevel(FrontierChannel chn);

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

// GZip and base64URL encode
int fn_gzip_str2urlenc(const char *str,int size,char **out);


#endif /*__HEADER_H_FRONTIER_H*/

