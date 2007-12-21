/*
 * frontier client base64 memory handler
 * 
 * Author: Sergey Kosyakov
 *
 * $Id$
 *
 *  Copyright (C) 2007  Fermilab
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);

/* This is the size of each buffer after base64 decoding is applied.
   The buffers get linked together and as many as needed are allocated
   because we don't know ahead of time how much space is going to be
   needed.  Choose a size large enough that the overhead time of
   malloc becomes insignificant for large queries, but small enough
   that there isn't a lot of wasted space.
   Just in case we're using a malloc that likes to allocate on page
   boundaries, choose a size that fits nicely into a multiple of pages
   after the FrontierMemBuf structure at the beginning of the allocated
   space, minus a little more for whatever overhead malloc might take.
  */
#define MEMBUF_SIZE	(16*4096-sizeof(FrontierMemBuf)-128)

FrontierMemData *frontierMemData_create()
 {
  FrontierMemData *md;
  FrontierMemBuf *mb;

  md=frontier_mem_alloc(sizeof(*md));
  if(!md) return md;

  mb=(FrontierMemBuf *)frontier_mem_alloc(sizeof(*mb)+MEMBUF_SIZE);
  if(!mb)
   {
    frontier_mem_free(md);
    md=(void*)0;
    return md;
   }
  md->firstbuf=mb;
  md->lastbuf=mb;
  md->total=0;
  mb->nextbuf=0;
  mb->len=0;
  bzero(&md->b64context,sizeof(md->b64context));
  return md;
 }


void frontierMemData_delete(FrontierMemData *md)
 {
  FrontierMemBuf *mb,*nextmb;

  if(!md) return;

  mb=md->firstbuf;
  while(mb!=0)
   {
    nextmb=mb->nextbuf;
    frontier_mem_free(mb);
    mb=nextmb;
   }

  frontier_mem_free(md);
 }

int frontierMemData_b64append(FrontierMemData *md,const char *buf,size_t size)
 {
  FrontierMemBuf *mb=md->lastbuf;
  char *p=((char *)mb)+sizeof(*mb)+mb->len;
  size_t spaceleft=MEMBUF_SIZE-mb->len;
  size_t sizeused,spaceused;

  while(1)
   {
    sizeused=size;
    spaceused=spaceleft;
    fn_base64_stream_ascii2bin(&md->b64context,(char *)buf,&size,p,&spaceleft);
    sizeused-=size;
    spaceused-=spaceleft;
    buf+=sizeused;
    mb->len+=spaceused;
    md->total+=spaceused;
    if(size==0)
      return FRONTIER_OK;
    //else spaceleft must be zero, allocate another buffer
    mb=(FrontierMemBuf *)frontier_mem_alloc(sizeof(*mb)+MEMBUF_SIZE);
    if(!mb) return FRONTIER_EMEM;
    mb->nextbuf=0;
    mb->len=0;
    md->lastbuf->nextbuf=mb;
    md->lastbuf=mb;
    p=((char *)mb)+sizeof(*mb);
    spaceleft=MEMBUF_SIZE;
   }
 }

