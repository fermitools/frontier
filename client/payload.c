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
  fp->blob=(char*)frontier_mem_alloc(fp->md->len);
  fp->blob_size=base64_ascii2bin(fp->md->buf,fp->md->len,fp->blob,fp->md->len);

  if(fp->blob_size<0) return FRONTIER_EBASE64;

  printf("Blob size %d\n",fp->blob_size);

  return FRONTIER_OK;
 }






