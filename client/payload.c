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

#include <frontier.h>
#include "fn-internal.h"
#include "fn-base64.h"
#include <stdio.h>
#include <strings.h>

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);


FrontierPayload *frontierPayload_create()
 {
  FrontierPayload *pl;

  pl=frontier_mem_alloc(sizeof(FrontierPayload));
  if(!pl) return pl;

  pl->id=-1;
  pl->encoding=-1;
  pl->md=frontierMemData_create();
  if(!pl->md)
   {
    frontier_mem_free(pl);
    return (void*)0;
   }

  pl->blob=(void*)0;
  pl->blob_size=0;
  
  bzero(pl->md5,16);
  bzero(pl->md5_str,36);
  bzero(pl->srv_md5_str,36);
  
  return pl;
 }


void frontierPayload_delete(FrontierPayload *pl)
 {
  if(!pl) return;

  if(pl->blob) frontier_mem_free(pl->blob);

  frontierMemData_delete(pl->md);
  frontier_mem_free(pl);
 }


void frontierPayload_append(FrontierPayload *pl,const char *s,int len)
 {
  frontierMemData_append(pl->md,s,len);
 }



int frontierPayload_finalize(FrontierPayload *fp)
 {
  char *md5_ctx;
  int i;
  
  fp->blob=(char*)frontier_mem_alloc(fp->md->len);
  fp->blob_size=base64_ascii2bin(fp->md->buf,fp->md->len,fp->blob,fp->md->len);

  if(fp->blob_size<0) return FRONTIER_EBASE64;
  
  md5_ctx=frontier_mem_alloc(frontier_md5_get_ctx_size());
  if(!md5_ctx) return FRONTIER_EMEM;
  frontier_md5_init(md5_ctx);
  frontier_md5_update(md5_ctx,fp->blob,fp->blob_size);
  frontier_md5_final(md5_ctx,fp->md5);
  frontier_mem_free(md5_ctx);
  bzero(fp->md5_str,36);
  for(i=0;i<16;i++)
   {
    snprintf(((char*)(fp->md5_str))+(i*2),3,"%02x",fp->md5[i]);
   }

  //printf("Blob size %d md5 %s\n",fp->blob_size,fp->md5_str);

  return FRONTIER_OK;
 }






