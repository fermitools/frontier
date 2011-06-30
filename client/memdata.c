/*
 * frontier client base64 memory handler
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

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"
#include "fn-zlib.h"
#include "zlib.h"

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

FrontierMemData *frontierMemData_create(int zipped)
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
  mb->nextbuf=0;
  mb->len=0;
  md->firstbuf=mb;
  md->lastbuf=mb;
  md->error=FRONTIER_OK;
  md->total=0;
  bzero(&md->b64context,sizeof(md->b64context));
  bzero(md->md5,sizeof(md->md5));
  frontier_md5_init(&md->md5_ctx);
  md->zipped_total=0;
  md->binzipped=zipped;
  md->zipbuflen=0;
  fn_gunzip_init();
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

static FrontierMemBuf *frontierMemData_allocbuffer(FrontierMemData *md)
 {
  FrontierMemBuf *mb;
  mb=(FrontierMemBuf *)frontier_mem_alloc(sizeof(*mb)+MEMBUF_SIZE);
  if(!mb) return mb;
  mb->nextbuf=0;
  mb->len=0;
  md->lastbuf->nextbuf=mb;
  md->lastbuf=mb;
  return mb;
 }

// append new base64-encoded data: decode it, calculate the md5 of the
//  decoded data, and unzip it if it was zipped.  Put the result in 
//  membufs.  If it is a final update, finalize the md5 & unzipping.
static int frontierMemData_append(FrontierMemData *md,
			const unsigned char *buf,int size,int final)
 {
  FrontierMemBuf *mb=md->lastbuf;
  unsigned char *p=((unsigned char *)mb)+sizeof(*mb)+mb->len;
  int spaceleft=MEMBUF_SIZE-mb->len;
  int sizeused,spaceused;
  unsigned char *p2=0;
  int size2,spaceleft2;

  if(md->error!=FRONTIER_OK)
   {
    // if called again after an error, just return previous error
    return md->error;
   }

  if(md->binzipped)
   {
    // base64-decode into intermediate buffer "zipbuf"
    p2=p;
    spaceleft2=spaceleft;
    p=&md->zipbuf[md->zipbuflen];
    spaceleft=sizeof(md->zipbuf)-md->zipbuflen;
   }

  while(1)
   {
    sizeused=size;
    spaceused=spaceleft;
    fn_base64_stream_ascii2bin(&md->b64context,buf,&size,p,&spaceleft);
    sizeused-=size;
    spaceused-=spaceleft;
    buf+=sizeused;
    if(md->binzipped)
     {
      md->zipbuflen+=spaceused;
      md->zipped_total+=spaceused;
      if((size==0)&&(!final))
       {
	// all the incoming buffer was copied to the zipbuf
        return FRONTIER_OK;
       }
      // else finished filling the zipbuf, update md5
      frontier_md5_update(&md->md5_ctx,((u8 *)&md->zipbuf[0]),md->zipbuflen);
      // and unzip the zipbuf
      p=md->zipbuf;
      size2=md->zipbuflen;
      while(1)
       {
        // unzip as much of the zipbuf as there's room for in the membuf
	int ret;
	sizeused=size2;
	spaceused=spaceleft2;
        ret=fn_gunzip_update(p,&size2,p2,&spaceleft2,final);
	switch(ret)
	 {
	  case Z_OK:
	    break;
	  case Z_BUF_ERROR:
	    // this is fine, it just means there was no space for progress
	    //  so allocate another buffer and try again
	    break;
	  case Z_MEM_ERROR:
	    frontier_setErrorMsg(__FILE__,__LINE__,"unzip memory error");
	    md->error=FRONTIER_EMEM;
	    return md->error;
	  case Z_DATA_ERROR:
	    frontier_setErrorMsg(__FILE__,__LINE__,"unzip data error");
	    md->error=FRONTIER_EPROTO;
	    return md->error;
	  default:
	    frontier_setErrorMsg(__FILE__,__LINE__,"unzip unknown error");
	    md->error=FRONTIER_EUNKNOWN;
	    return md->error;
	 }
	sizeused-=size2;
	spaceused-=spaceleft2;
	p+=sizeused;
        mb->len+=spaceused;
        md->total+=spaceused;
	if(size2==0)
	 {
	  // all the zipbuf was uncompressed
	  md->zipbuflen=0;
	  if(final)
	   {
	    if(ret==Z_OK)
	      break;
	    // else Z_BUF_ERROR, allocate another membuf and try again
	   }
	  else
	   {
	    p=md->zipbuf;
	    spaceleft=sizeof(md->zipbuf);
	    break;
	   }
	 }
	// else spaceleft2 must be zero, allocate another membuf
        mb=frontierMemData_allocbuffer(md);
        if(!mb) return FRONTIER_EMEM;
        p2=((unsigned char *)mb)+sizeof(*mb);
        spaceleft2=MEMBUF_SIZE;
       }
     }
    else
     {
      mb->len+=spaceused;
      md->total+=spaceused;
      if((size==0)&&!final)
        return FRONTIER_OK;
      //else finished with this membuf, update md5
      frontier_md5_update(&md->md5_ctx,((u8 *)mb)+sizeof(*mb),mb->len);
      if(!final)
       {
	// allocate another membuf
	mb=frontierMemData_allocbuffer(md);
	if(!mb)
	 {
	 md->error=FRONTIER_EMEM;
	 return md->error;
	 }
	p=((unsigned char *)mb)+sizeof(*mb);
	spaceleft=MEMBUF_SIZE;
      }
     }
    if(final)
     {
      frontier_md5_final(&md->md5_ctx,md->md5);
      return FRONTIER_OK;
     }
   }
 }

int frontierMemData_b64append(FrontierMemData *md,const char *buf,int size)
 {
  return frontierMemData_append(md,(const unsigned char *)buf,size,0);
 }

int frontierMemData_finalize(FrontierMemData *md)
 {
  return frontierMemData_append(md,0,0,1);
 }

unsigned char *frontierMemData_getmd5(FrontierMemData *md)
 {
  return md->md5;
 }
