/*
 * frontier client C API header
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
/*
 * NOTEs on thread safety: 
 * frontier_init(), frontier_initdebug(), and frontier_createChannel()
 *   are always thread-safe.  The other functions might not be unless
 *   the configuration option threadsafe=yes is set on any channel or
 *   the function frontier_setThreadSafe() is called.  From then on all
 *   functions should be safe.  Parallelism is limited however, and is
 *   primarily only allowed while waiting for input and while doing
 *   the cpu-intensive unpacking of data.
 * With multiple threads, frontier_getErrorMsg() is not reliable outside
 *   of the client and may return a value from a different thread.
 */

#ifndef __HEADER_H_FRONTIER_H
#define __HEADER_H_FRONTIER_H

#include <inttypes.h>
#include <sys/types.h>
#include "frontier_config.h"
#include "frontier_log.h"
#include "frontier_error.h"

#define FRONTIER_MAX_PAYLOADNUM	32

typedef uintptr_t FrontierChannel;
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
char *frontierRSBlob_getByteArray(FrontierRSBlob *rs,unsigned int len,int *ec);
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
void frontier_setTimeToLive(FrontierChannel u_channel,int ttl); // 1=short, 2=long, 3=forever
void frontier_setReload(FrontierChannel u_channel,int reload); // deprecated: 0=long ttl, !0=short ttl
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

void frontier_setThreadSafe();

// GZip and base64URL encode
int fn_gzip_str2urlenc(const char *str,int size,char **out);

// NOTE: The contents of this struct may not change or binary compatibility
//   will break.
struct s_FrontierStatisticsNum
 {
  unsigned int min;
  unsigned int max;
  unsigned int avg;
 };
typedef struct s_FrontierStatisticsNum FrontierStatisticsNum;

// NOTE: for binary compatibility, variables should never be removed
//   from this structure, and new ones should only be added to the end.
struct s_FrontierStatistics
 {
  int num_queries;
  int num_errors; // num with no response bytes; not included in other stats
  FrontierStatisticsNum bytes_per_query;
  FrontierStatisticsNum msecs_per_query;
 };
typedef struct s_FrontierStatistics FrontierStatistics;
void frontier_statistics_start();
// return is status as defined in frontier_error.h
int frontier_statistics_get_bytes(FrontierStatistics *stats,int maxbytes);
#define frontier_statistics_get(STATS) frontier_statistics_get_bytes(STATS, sizeof(*(STATS)))
void frontier_statistics_stop();

#endif /*__HEADER_H_FRONTIER_H*/

