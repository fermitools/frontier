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
  
  ret=compress2(dest,&res_size,src,src_size,9);
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
  char *zbuf=0;
  long zret;
  int ret;
  int asize;
  char *abuf=0;
  
  if(size>MAX_STR2URL_SIZE) return FN_ZLIB_E_TOOBIG;
  
  zsize=(int)(((double)size)*1.001+12);
  zbuf=frontier_mem_alloc(zsize);
  if(!zbuf) return FN_ZLIB_E_NOMEM;
  
  zret=fn_gzip_str(str,size,zbuf,zsize);
  if(zret<0) {ret=zret; goto end;}
  
  asize=((int)zret)*2+3;
  abuf=frontier_mem_alloc(asize);
  if(!abuf) {ret=FN_ZLIB_E_NOMEM; goto end;}
  
  ret=base64URL_bin2ascii(zbuf,zret,abuf,asize);
  if(ret<0) goto end;
  
  *out=abuf;
  abuf=0;
  
end:
  if(abuf) frontier_mem_free(abuf);
  if(zbuf) frontier_mem_free(zbuf);
  return ret;
 }
