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
#include <frontier_client/frontier.h>
#include <endian.h>
#include <sys/types.h>
#include <stdlib.h>
#include <strings.h>
#include "fn-internal.h"

#define TYPED_BLOB

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *p);

union u_Buf32 { int v; float f; char b[4]; };
union u_Buf64 { long long v; double d; char b[8]; };

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline int n2h_i32(const char *p) 
 {
  union u_Buf32 u; 
  u.b[3]=p[0]; 
  u.b[2]=p[1]; 
  u.b[1]=p[2]; 
  u.b[0]=p[3];  
  return u.v;
 }
static inline float n2h_f32(const char *p) 
 {
  union u_Buf32 u; 
  u.b[3]=p[0]; 
  u.b[2]=p[1]; 
  u.b[1]=p[2]; 
  u.b[0]=p[3];  
  return u.f;
 }
static inline long long n2h_i64(const char *p) 
 {
  union u_Buf64 u; 
  u.b[7]=p[0]; 
  u.b[6]=p[1]; 
  u.b[5]=p[2]; 
  u.b[4]=p[3];  
  u.b[3]=p[4]; 
  u.b[2]=p[5]; 
  u.b[1]=p[6]; 
  u.b[0]=p[7];  
  return u.v;
 }
static inline double n2h_d64(const char *p) 
 {
  union u_Buf64 u; 
  u.b[7]=p[0]; 
  u.b[6]=p[1]; 
  u.b[5]=p[2]; 
  u.b[4]=p[3];  
  u.b[3]=p[4]; 
  u.b[2]=p[5]; 
  u.b[1]=p[6]; 
  u.b[0]=p[7];  
  return u.d;
 }
#else
#warning Big endian order
#include <strings.h>
static inline int n2h_i32(const char *p) 
 {
  return *((int*)p);
 }
static inline float n2h_f32(const char *p) 
 {
  return *((float*)p);
 }
static inline long long n2h_i64(const char *p) 
 {
  return *((long long*)p);
 }
static inline double n2h_d64(const char *p) 
 {
  return *((double*)p);
 }
#endif /*__BYTE_ORDER*/

int frontier_n2h_i32(const void* p){return n2h_i32(p);}
float frontier_n2h_f32(const void* p){return n2h_f32(p);}
long long frontier_n2h_i64(const void* p){return n2h_i64(p);}
double frontier_n2h_d64(const void* p){return n2h_d64(p);}

FrontierRSBlob *frontierRSBlob_get(FrontierChannel u_channel,int n,int *ec)
 {
  Channel *chn=(Channel*)u_channel;
  FrontierResponse *resp;
  FrontierPayload *fp;
  FrontierRSBlob *rs=(void*)0;

  resp=chn->resp;
  
  if(n>resp->payload_num)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no such payload - total %d, requested %d",resp->payload_num,n);
    return rs;
   }

  fp=resp->payload[n-1];

  rs=frontier_mem_alloc(sizeof(FrontierRSBlob));
  if(!rs)
   {
    *ec=FRONTIER_EMEM;
    return rs;
   }

  rs->buf=fp->blob;
  rs->size=fp->blob_size;
  rs->pos=0;
  rs->nrec=fp->nrec;
  
  rs->payload_error=fp->error_code;
  rs->payload_msg=fp->error_msg;

  *ec=FRONTIER_OK;
  return rs;
 }

 
 
void frontierRSBlob_close(FrontierRSBlob *rs,int *ec)
 {
  *ec=FRONTIER_OK;

  if(!rs) return;
  frontier_mem_free(rs);
 }


void frontierRSBlob_rsctl(FrontierRSBlob *rs,int ctl_fn,void *data,int size,int *ec)
 {
  *ec=FRONTIER_OK;
 }


void frontierRSBlob_start(FrontierRSBlob *rs,int *ec)
 {
  *ec=FRONTIER_OK;
  rs->pos=0;
 }

 
 
char frontierRSBlob_getByte(FrontierRSBlob *rs,int *ec)
 {
  char ret;

  if(rs->pos>=rs->size)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return (char)-1;
   }
  ret=rs->buf[rs->pos];
  rs->pos++;
  *ec=FRONTIER_OK;
  return ret;
 }
 
 
char frontierRSBlob_checkByte(FrontierRSBlob *rs,int *ec)
 {
  char ret;

  if(rs->pos>=rs->size)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return (char)-1;
   }
  ret=rs->buf[rs->pos];
  *ec=FRONTIER_OK;
  return ret;
 }
 
 
void frontierRSBlob_getArea(FrontierRSBlob *rs,char *p,unsigned int len,int *ec)
 {
  const char *buf;
  
  if(rs->pos>=rs->size-(len-1))
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return;
   }
  buf=rs->buf+rs->pos;
  bcopy(buf,p,len);
  rs->pos+=len;
  *ec=FRONTIER_OK;
  return; 
 } 


int frontierRSBlob_getInt(FrontierRSBlob *rs,int *ec)
 {
  int ret;

  if(rs->pos>=rs->size-3)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return -1;
   }
  ret=n2h_i32(rs->buf+rs->pos);
  rs->pos+=4;
  *ec=FRONTIER_OK;
  return ret;
 }


long long frontierRSBlob_getLong(FrontierRSBlob *rs,int *ec)
 {
  long long ret;

  if(rs->pos>=rs->size-7)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return -1;
   }
  ret=n2h_i64(rs->buf+rs->pos);
  rs->pos+=8;
  *ec=FRONTIER_OK;
  return ret;
 }


double frontierRSBlob_getDouble(FrontierRSBlob *rs,int *ec)
 {
  double ret;

  if(rs->pos>=rs->size-7)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return -1;
   }
  ret=n2h_d64(rs->buf+rs->pos);
  rs->pos+=8;
  *ec=FRONTIER_OK;
  return ret;
 }
 

float frontierRSBlob_getFloat(FrontierRSBlob *rs,int *ec)
 {
  float ret;

  if(rs->pos>=rs->size-3)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return -1;
   }
  ret=n2h_f32(rs->buf+rs->pos);
  rs->pos+=4;
  *ec=FRONTIER_OK;
  return ret;
 }
 



