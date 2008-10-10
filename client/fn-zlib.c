/*
 * frontier client zlib interface functions
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
#include "frontier_client/frontier.h"
#include "fn-zlib.h"
#include "fn-base64.h"

#include "zlib.h"

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);

static z_stream *zstream=0;

static void *fn_zalloc(void *opaque,uInt items,uInt size)
 {
  return frontier_mem_alloc(items*size);
 }

static void fn_zfree(void *opaque,void *address)
 {
  frontier_mem_free(address);
 }

long fn_gzip_str(const char *src,long src_size,char *dest,long dest_size)
 {
  int ret;

  if(zstream==0)
   {
    // open a stream and leave it open until channel closes
    //  because deflateInit does many, large allocs and thrashes
    zstream=frontier_mem_alloc(sizeof(*zstream));
    if(zstream==0)
      return FN_ZLIB_E_NOMEM;
    zstream->zalloc=fn_zalloc;
    zstream->zfree=fn_zfree;
    zstream->opaque=0;
    ret=deflateInit(zstream,9);
    if(ret!=Z_OK)
     {
      fn_gzip_cleanup();
      if(ret==Z_MEM_ERROR)
	return FN_ZLIB_E_NOMEM;
      return FN_ZLIB_E_OTHER;
     }
   }
  else
   {
    // reuse existing stream
    ret=deflateReset(zstream);
    if(ret!=Z_OK)
     {
      fn_gzip_cleanup();
      return FN_ZLIB_E_OTHER;
     }
   }
  
  zstream->next_in=(Bytef *)src;
  zstream->avail_in=(uLongf)src_size;
  zstream->next_out=(Bytef *)dest;
  zstream->avail_out=(uLongf)dest_size;
  ret=deflate(zstream,Z_FINISH);
  if(ret==Z_STREAM_END)
   {
    // leave stream available
    return dest_size-(long)zstream->avail_out;
   }

  fn_gzip_cleanup();
  if(ret==Z_BUF_ERROR)
    return FN_ZLIB_E_SMALLBUF;
  return FN_ZLIB_E_OTHER;
 }

void fn_gzip_cleanup()
 {
  if(zstream!=0)
   {
    deflateEnd(zstream);
    frontier_mem_free(zstream);
    zstream=0;
   }
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
