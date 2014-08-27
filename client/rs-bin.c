/*
 * frontier client response set handler
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
#include <frontier_client/frontier.h>
#include <sys/types.h>
#include <stdlib.h>
#include <strings.h>
#include "fn-internal.h"
#include "fn-endian.h"

#define TYPED_BLOB

union u_Buf32 { int v; float f; char b[4]; };
union u_Buf64 { long long v; double d; char b[8]; };

#if __BYTE_ORDER == __LITTLE_ENDIAN
static inline int n2h_i32(const unsigned char *p) 
 {
  union u_Buf32 u; 
  u.b[3]=p[0]; 
  u.b[2]=p[1]; 
  u.b[1]=p[2]; 
  u.b[0]=p[3];  
  return u.v;
 }
static inline float n2h_f32(const unsigned char *p) 
 {
  union u_Buf32 u; 
  u.b[3]=p[0]; 
  u.b[2]=p[1]; 
  u.b[1]=p[2]; 
  u.b[0]=p[3];  
  return u.f;
 }
static inline long long n2h_i64(const unsigned char *p) 
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
static inline double n2h_d64(const unsigned char *p) 
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
static inline int n2h_i32(const unsigned char *p) 
 {
  return *((int*)p);
 }
static inline float n2h_f32(const unsigned char *p) 
 {
  return *((float*)p);
 }
static inline long long n2h_i64(const unsigned char *p) 
 {
  return *((long long*)p);
 }
static inline double n2h_d64(const unsigned char *p) 
 {
  return *((double*)p);
 }
#endif /*__BYTE_ORDER*/

int frontier_n2h_i32(const void* p){return n2h_i32(p);}
float frontier_n2h_f32(const void* p){return n2h_f32(p);}
long long frontier_n2h_i64(const void* p){return n2h_i64(p);}
double frontier_n2h_d64(const void* p){return n2h_d64(p);}

/*this function is for backward compatibility*/
FrontierRSBlob *frontierRSBlob_get(FrontierChannel u_channel,int n,int *ec)
 {
   return frontierRSBlob_open(u_channel,0,n,ec);
 }

FrontierRSBlob *frontierRSBlob_open(FrontierChannel u_channel,FrontierRSBlob *oldfrs,int n,int *ec)
 {
  Channel *chn=(Channel*)u_channel;
  FrontierResponse *resp;
  FrontierPayload *fp;
  FrontierRSBlob *frs=(void*)0;
  RSBlob *rs=(void*)0;
  RSBlob *oldrs=(RSBlob *)oldfrs;

  if(oldrs&&(oldrs->resp))
   {
    /*move the response from the old RSblob to here*/
    resp=oldrs->resp;
    oldrs->resp=0;
   }
  else
   {
    /*move the response from the channel to here*/
    resp=chn->resp;
    chn->resp=0;
   }
  
  if(n>resp->payload_num)
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no such payload - total %d, requested %d",resp->payload_num,n);
    if(resp)frontierResponse_delete(resp);
    return frs;
   }

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"parsing chan %d response %d payload %d",chn->seqnum,resp->seqnum,n);

  fp=resp->payload[n-1];

  rs=frontier_mem_alloc(sizeof(RSBlob));
  frs=(FrontierRSBlob *)rs;
  if(!frs)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    if(resp)frontierResponse_delete(resp);
    return frs;
   }

  rs->resp=resp;
  rs->buf=fp->blob;
  rs->size=fp->blob_size;
  rs->pos=0;
  rs->nrec=fp->nrec;
  rs->respnum=resp->seqnum; // for debugging
  
  rs->payload_error=fp->error_code;
  rs->payload_msg=fp->error_msg;

  *ec=FRONTIER_OK;
  return frs;
 }

 
 
void frontierRSBlob_close(FrontierRSBlob *frs,int *ec)
 {
  RSBlob *rs=(RSBlob *)frs;
  *ec=FRONTIER_OK;

  if(!rs) return;
  if(rs->resp)frontierResponse_delete(rs->resp);
  frontier_mem_free(rs);
 }


void frontierRSBlob_rsctl(FrontierRSBlob *frs,int ctl_fn,void *data,int size,int *ec)
 {
  *ec=FRONTIER_OK;
 }


void frontierRSBlob_start(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
  *ec=FRONTIER_OK;
  rs->pos=0;
 }

 
 
char frontierRSBlob_getByte(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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
 
 
char frontierRSBlob_checkByte(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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
 
 
char *frontierRSBlob_getByteArray(FrontierRSBlob *frs,unsigned int len,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
  const unsigned char *buf;
  
  if(rs->pos>=rs->size-(len-1))
   {
    *ec=FRONTIER_EIARG;
    frontier_setErrorMsg(__FILE__,__LINE__,"resultset size (%d bytes) violation",rs->size);
    return 0;
   }
  buf=rs->buf+rs->pos;
  rs->pos+=len;
  *ec=FRONTIER_OK;
  return (char *)buf; 
 } 


int frontierRSBlob_getInt(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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


long long frontierRSBlob_getLong(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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


double frontierRSBlob_getDouble(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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
 

float frontierRSBlob_getFloat(FrontierRSBlob *frs,int *ec)
 {
  RSBlob* rs=(RSBlob *)frs;
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
 

unsigned int frontierRSBlob_getRecNum(FrontierRSBlob *frs)
 {
  RSBlob* rs=(RSBlob *)frs;
  return rs->nrec;
 }

unsigned int frontierRSBlob_getPos(FrontierRSBlob *frs)
 {
  RSBlob* rs=(RSBlob *)frs;
  return rs->pos;
 }

unsigned int frontierRSBlob_getSize(FrontierRSBlob *frs)
 {
  RSBlob* rs=(RSBlob *)frs;
  return rs->size;
 }

int frontierRSBlob_payload_error(FrontierRSBlob *frs)
 {
  RSBlob* rs=(RSBlob *)frs;
  return rs->payload_error;
 }

const char *frontierRSBlob_payload_msg(FrontierRSBlob *frs)
 {
  RSBlob* rs=(RSBlob *)frs;
  return rs->payload_msg;
 }


