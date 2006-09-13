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
#include "zlib.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);
extern int frontier_log_level;


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
  fpl->encoding=(void*)0;
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
  bzero(fpl->md5_str,sizeof(fpl->md5_str));
  bzero(fpl->srv_md5_str,sizeof(fpl->srv_md5_str));
  
  fpl->error=0;
  fpl->error_code=0;
  fpl->error_msg=(void*)0;
  
  return fpl;
 }


void frontierPayload_delete(FrontierPayload *fpl)
 {
  if(!fpl) return;

  if(fpl->blob) frontier_mem_free(fpl->blob);
  
  if(fpl->encoding) frontier_mem_free(fpl->encoding);

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
  char *md5_ctx = 0;
  int i;
  int zipped=0;
  int bin_size;
  /* uLongf needed for 64-bit uncompress! */
  uLongf blob_size;
  char *bin_data = 0;
  
  fpl->blob = 0;
  fpl->error = FRONTIER_OK;

  if(fpl->error_code!=FRONTIER_OK) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Server signalled payload error %d: %s",fpl->error_code,fpl->error_msg);
    fpl->error=FRONTIER_EPROTO;
    goto errcleanup;
   }

  if(fpl->encoding==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Encoding not specified in response");
    fpl->error=FRONTIER_EPROTO;
    goto errcleanup;
   }
  
  if(strncmp(fpl->encoding,"BLOB",4)!=0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Unrecognized encoding type: %s",fpl->encoding);
    fpl->error=FRONTIER_EPROTO;
    goto errcleanup;
   }
  
  if(fpl->encoding[4]!='\0')
   {
    if (strncmp(&fpl->encoding[4],"zip",3)!=0)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"Unrecognized BLOB encoding subtype: %s",&fpl->encoding[4]);
      fpl->error=FRONTIER_EPROTO;
      goto errcleanup;
     }
     zipped=1;
   }

  /* binary data is not larger than base64-encoded data */
  bin_data=(char*)frontier_mem_alloc(fpl->md->len);
  if(!bin_data)
   {
    FRONTIER_MSG(FRONTIER_EMEM);   
    fpl->error=FRONTIER_EMEM;
    goto errcleanup;
   }
  bin_size=base64_ascii2bin(fpl->md->buf,fpl->md->len,bin_data,fpl->md->len);

  if(bin_size<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong response - base64 decode failed");
    fpl->error=FRONTIER_EPROTO;
    goto errcleanup;
   }

  md5_ctx=frontier_mem_alloc(frontier_md5_get_ctx_size());
  if(!md5_ctx) 
   {
    FRONTIER_MSG(FRONTIER_EMEM);       
    fpl->error=FRONTIER_EMEM;
    goto errcleanup;
   }
  frontier_md5_init(md5_ctx);
  frontier_md5_update(md5_ctx,bin_data,bin_size);
  frontier_md5_final(md5_ctx,fpl->md5);
  frontier_mem_free(md5_ctx);
  md5_ctx=0;
  bzero(fpl->md5_str,sizeof(fpl->md5_str));
  for(i=0;i<16;i++)
   {
    snprintf(((char*)(fpl->md5_str))+(i*2),3,"%02x",fpl->md5[i]);
   }

  if (zipped)
   {
#if 0
    char hexdata[60*3+1];

    for (i=0;(i<bin_size)&&(i<(sizeof(hexdata)/3));i++)
	sprintf(&hexdata[i*3],"%02x\n",(unsigned char)bin_data[i]);
    hexdata[i*3]='\0';

    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"pre-uncompressed %d byte (full size %d) payload: %s",bin_size,fpl->full_size,hexdata);
#else
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"uncompressing %d byte (full size %d) payload",bin_size,fpl->full_size);
#endif

    fpl->blob=(char*)frontier_mem_alloc(fpl->full_size);
    if(!fpl->blob)
     {
      FRONTIER_MSG(FRONTIER_EMEM);   
      fpl->error=FRONTIER_EMEM;
      goto errcleanup;
     }
    blob_size=fpl->full_size;
    switch(uncompress(fpl->blob,&blob_size,bin_data,bin_size))
     {
      case Z_OK:
	break;
      case Z_BUF_ERROR:
	frontier_setErrorMsg(__FILE__,__LINE__,"uncompress buf error");
	fpl->error=FRONTIER_EPROTO;
	goto errcleanup;
      case Z_MEM_ERROR:
	frontier_setErrorMsg(__FILE__,__LINE__,"uncompress memory error");
	fpl->error=FRONTIER_EMEM;
	goto errcleanup;
      case Z_DATA_ERROR:
	frontier_setErrorMsg(__FILE__,__LINE__,"uncompress data error");
	fpl->error=FRONTIER_EPROTO;
	goto errcleanup;
      default:
	frontier_setErrorMsg(__FILE__,__LINE__,"uncompress unknown error");
	fpl->error=FRONTIER_EUNKNOWN;
	goto errcleanup;
     }
    frontier_mem_free(bin_data);
    bin_data=0;
    fpl->blob_size=(int) blob_size;
   }
  else
   {
    fpl->blob_size=bin_size;
    fpl->blob=bin_data;
   }

  if(frontier_log_level>=FRONTIER_LOGLEVEL_DEBUG)
   {
    unsigned char dumpdata[80*5+1];
    int n=0;
    unsigned char c;
    for(i=0;(i<fpl->blob_size)&&(n<(sizeof(dumpdata)-sizeof("%00")));i++)
     {
      c=fpl->blob[i];
      if(c==0x07)
       {
        /* 0x07 separates records */
        dumpdata[n++]='\n';
       }
      else if(c<=26)
       {
	dumpdata[n++]='^';
	if(c==0)
          dumpdata[n++]='@';
	else
          dumpdata[n++]=c+'a'-1;
       }
      else if(isprint(c))
        dumpdata[n++]=c;
      else
        n+=sprintf(&dumpdata[n],"%%%02x",c);
     }
    dumpdata[n]='\0';
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"start of decoded response: %s",dumpdata);
   }
  
  if (fpl->blob_size != fpl->full_size)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"BLOB decoded size %d did not match expected size %d",fpl->blob_size,fpl->full_size);
    fpl->error=FRONTIER_EPROTO;
    goto errcleanup;
   }

  //printf("Blob size %d md5 %s\n",fpl->blob_size,fpl->md5_str);

  return FRONTIER_OK;

errcleanup:
  if (bin_data)
    frontier_mem_free(bin_data);
  if (md5_ctx)
    frontier_mem_free(md5_ctx);
  if (fpl->blob)
   {
    frontier_mem_free(fpl->blob);
    fpl->blob=0;
   }
  return fpl->error;
 }

