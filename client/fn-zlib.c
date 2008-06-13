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


long fn_gzip_str(const char *src,long src_size,char *dest,long dest_size)
 {
  int ret;
  long res_size=dest_size;
  
  ret=compress2((Bytef *)dest,(uLongf *)&res_size,(const Bytef *)src,(uLong)src_size,9);
  switch(ret)
   {
    case Z_OK: return res_size;
    case Z_BUF_ERROR: return FN_ZLIB_E_SMALLBUF;
    case Z_MEM_ERROR: return FN_ZLIB_E_NOMEM;
    default: return FN_ZLIB_E_OTHER;
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
