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
#include "fn-internal.h"
#include "fn-base64.h"
#include <stdio.h>
#include <strings.h>

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);


FrontierPayload *frontierPayload_create()
 {
  FrontierPayload *fpl;

  fpl=frontier_mem_alloc(sizeof(FrontierPayload));
  if(!fpl) 
   {
    FRONTIER_MSG(FRONTIER_EMEM);   
    return fpl;
   }

  fpl->id=-1;
  fpl->encoding=-1;
  fpl->md=frontierMemData_create();
  if(!fpl->md)
   {
    frontierPayload_delete(fpl);
    FRONTIER_MSG(FRONTIER_EMEM);
    return (void*)0;
   }

  fpl->blob=(void*)0;
  fpl->blob_size=0;
  
  bzero(fpl->md5,16);
  bzero(fpl->md5_str,36);
  bzero(fpl->srv_md5_str,36);
  
  fpl->error=0;
  fpl->error_code=0;
  fpl->error_msg=(void*)0;
  
  return fpl;
 }


void frontierPayload_delete(FrontierPayload *fpl)
 {
  if(!fpl) return;

  if(fpl->blob) frontier_mem_free(fpl->blob);
  
  if(fpl->error_msg) frontier_mem_free(fpl->error_msg);

  frontierMemData_delete(fpl->md);
  frontier_mem_free(fpl);
 }


void frontierPayload_append(FrontierPayload *fpl,const char *s,int len)
 {
  frontierMemData_append(fpl->md,s,len);
 }



int frontierPayload_finalize(FrontierPayload *fpl)
 {
  char *md5_ctx;
  int i;
  
  if(fpl->error_code!=FRONTIER_OK) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Server signalled payload error %d: %s",fpl->error_code,fpl->error_msg);
    return FRONTIER_EPROTO;
   }
  
  fpl->blob=(char*)frontier_mem_alloc(fpl->md->len);
  if(!fpl->blob)
   {
    fpl->error=FRONTIER_EMEM;
    FRONTIER_MSG(FRONTIER_EMEM);   
    return FRONTIER_EMEM;
   }
  fpl->blob_size=base64_ascii2bin(fpl->md->buf,fpl->md->len,fpl->blob,fpl->md->len);

  if(fpl->blob_size<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong response - base64 decode failed");
    return FRONTIER_EPROTO;
   }
  
  md5_ctx=frontier_mem_alloc(frontier_md5_get_ctx_size());
  if(!md5_ctx) 
   {
    fpl->error=FRONTIER_EMEM;
    FRONTIER_MSG(FRONTIER_EMEM);       
    return FRONTIER_EMEM;
   }
  frontier_md5_init(md5_ctx);
  frontier_md5_update(md5_ctx,fpl->blob,fpl->blob_size);
  frontier_md5_final(md5_ctx,fpl->md5);
  frontier_mem_free(md5_ctx);
  bzero(fpl->md5_str,36);
  for(i=0;i<16;i++)
   {
    snprintf(((char*)(fpl->md5_str))+(i*2),3,"%02x",fpl->md5[i]);
   }

  //printf("Blob size %d md5 %s\n",fp->blob_size,fp->md5_str);

  return FRONTIER_OK;
 }

