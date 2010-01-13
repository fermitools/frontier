/*
 * frontier client zlib interface functions
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
#include "frontier_client/frontier.h"
#include "fn-zlib.h"
#include "fn-base64.h"

#include "zlib.h"

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);

static z_stream *dezstream=0;
static z_stream *inzstream=0;

static void *fn_zalloc(void *opaque,uInt items,uInt size)
 {
  return frontier_mem_alloc(items*size);
 }

static void fn_zfree(void *opaque,void *address)
 {
  frontier_mem_free(address);
 }

static void fn_decleanup()
 {
  if(dezstream!=0)
   {
    deflateEnd(dezstream);
    frontier_mem_free(dezstream);
    dezstream=0;
   }
 }

static void fn_incleanup()
 {
  if(inzstream!=0)
   {
    inflateEnd(inzstream);
    frontier_mem_free(inzstream);
    inzstream=0;
   }
 }

void fn_gzip_cleanup()
 {
  fn_decleanup();
  fn_incleanup();
 }
 
long fn_gzip_str(const char *src,long src_size,char *dest,long dest_size)
 {
  int ret;

  if(dezstream==0)
   {
    // open a stream and leave it open until channel closes
    //  because deflateInit does many, large allocs and thrashes
    dezstream=frontier_mem_alloc(sizeof(*dezstream));
    if(dezstream==0)
      return FN_ZLIB_E_NOMEM;
    dezstream->zalloc=fn_zalloc;
    dezstream->zfree=fn_zfree;
    dezstream->opaque=0;
    ret=deflateInit(dezstream,9);
    if(ret!=Z_OK)
     {
      fn_decleanup();
      if(ret==Z_MEM_ERROR)
	return FN_ZLIB_E_NOMEM;
      return FN_ZLIB_E_OTHER;
     }
   }
  else
   {
    // reuse existing stream
    ret=deflateReset(dezstream);
    if(ret!=Z_OK)
     {
      fn_decleanup();
      return FN_ZLIB_E_OTHER;
     }
   }
  
  dezstream->next_in=(Bytef *)src;
  dezstream->avail_in=(uLongf)src_size;
  dezstream->next_out=(Bytef *)dest;
  dezstream->avail_out=(uLongf)dest_size;
  ret=deflate(dezstream,Z_FINISH);
  if(ret==Z_STREAM_END)
   {
    // leave stream available
    return dest_size-(long)dezstream->avail_out;
   }

  fn_decleanup();
  if(ret==Z_BUF_ERROR)
    return FN_ZLIB_E_SMALLBUF;
  return FN_ZLIB_E_OTHER;
 }

// fn_gunzip is intended to be exactly like zlib's uncompress except it keeps
//  the z_stream initialized between invocations to reduce memory alloc/free.
// Unlike fn_gzip_str above, it returns Z_* errors instead of FN_ZLIB errors
//  because it doesn't try to return the length in the return value: it
//  returns the length in *dest_sizep instead.
int fn_gunzip(unsigned char *dest,long *dest_sizep,const unsigned char *src,long src_size)
 {
  int ret;

  if(inzstream==0)
   {
    // open a stream and leave it open just like with deflate above
    inzstream=frontier_mem_alloc(sizeof(*inzstream));
    if(inzstream==0)
      return Z_MEM_ERROR;
    inzstream->zalloc=fn_zalloc;
    inzstream->zfree=fn_zfree;
    inzstream->opaque=0;
    inzstream->next_in=Z_NULL;
    inzstream->avail_in=0;
    ret=inflateInit(inzstream);
    if(ret!=Z_OK)
     {
      fn_incleanup();
      return ret;
     }
   }
  else
   {
    // reuse existing stream
    ret=inflateReset(inzstream);
    if(ret!=Z_OK)
     {
      fn_incleanup();
      return ret;
     }
   }

  inzstream->next_in=(Bytef *)src;
  inzstream->avail_in=(uLongf)src_size;
  inzstream->next_out=(Bytef *)dest;
  inzstream->avail_out=(uLongf)*dest_sizep;
  ret=inflate(inzstream,Z_FINISH);
  if(ret==Z_STREAM_END)
   {
    *dest_sizep-=(long)inzstream->avail_out;
    // leave stream available
    return Z_OK;
   }

  fn_incleanup();
  return ret;
 }

int fn_gzip_str2urlenc(const char *str,int size,char **out)
 {
  int zsize;
  unsigned char *zbuf=0;
  long zret;
  int ret;
  int asize;
  unsigned char *abuf=0;
  
  if(size>MAX_STR2URL_SIZE) return FN_ZLIB_E_TOOBIG;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"encoding request [%s]",str);
  
  if(str[size-1]=='\n')
    size--;  // don't include trailing newline
  zsize=(int)(((double)size)*1.001+12);
  zbuf=frontier_mem_alloc(zsize);
  if(!zbuf) return FN_ZLIB_E_NOMEM;
  
  zret=fn_gzip_str(str,size,(char *)zbuf,zsize);
  if(zret<0) {ret=zret; goto end;}
  
  asize=((int)zret)*2+3;
  abuf=frontier_mem_alloc(asize);
  if(!abuf) {ret=FN_ZLIB_E_NOMEM; goto end;}
  
  ret=fn_base64URL_bin2ascii(zbuf,zret,abuf,asize);
  if(ret<0) goto end;
  
  *out=(char *)abuf;
  abuf=0;
  
end:
  if(abuf) frontier_mem_free(abuf);
  if(zbuf) frontier_mem_free(zbuf);
  return ret;
 }
