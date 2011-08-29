/*
 * frontier client payload handler
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

#include <frontier_client/frontier.h>
#include "fn-internal.h"
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);
extern int frontier_log_level;


FrontierPayload *frontierPayload_create(const char *encoding)
 {
  FrontierPayload *fpl;

  fpl=frontier_mem_alloc(sizeof(FrontierPayload));
  if(!fpl) 
   {
    FRONTIER_MSG(FRONTIER_EMEM);   
    return fpl;
   }

  fpl->id=-1;
  if(encoding==NULL)
    fpl->encoding=(void*)0;
  else
    fpl->encoding=frontier_str_copy(encoding);
  fpl->md=frontierMemData_create(strstr(encoding,"zip")!=NULL);
  if(!fpl->md)
   {
    frontierPayload_delete(fpl);
    FRONTIER_MSG(FRONTIER_EMEM);
    return (void*)0;
   }

  fpl->blob=(void*)0;
  fpl->blob_size=0;
  
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

  if(fpl->md) frontierMemData_delete(fpl->md);

  frontier_mem_free(fpl);
 }


void frontierPayload_append(FrontierPayload *fpl,const char *s,int len)
 {
  frontierMemData_b64append(fpl->md,s,len);
 }



int frontierPayload_finalize(FrontierPayload *fpl)
 {
  int i;
  int zipped=0;
  int zipped_size=0;
  unsigned char *p;
  FrontierMemBuf *mb;
  unsigned char *md5;
  
  fpl->blob = 0;
  fpl->error = FRONTIER_OK;

  if(fpl->error_code!=FRONTIER_OK) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Server signalled payload error %d: %s",fpl->error_code,fpl->error_msg);
    fpl->error=FRONTIER_ESERVER;
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

  // finalize the MemData. 
  fpl->error=frontierMemData_finalize(fpl->md);
  if(fpl->error!=FRONTIER_OK)
    goto errcleanup;

  md5=frontierMemData_getmd5(fpl->md);
  bzero(fpl->md5_str,sizeof(fpl->md5_str));
  // convert the binary md5 characters into printable
  for(i=0;i<16;i++)
   {
    snprintf(((char*)(fpl->md5_str))+(i*2),3,"%02x",md5[i]);
   }

  // put all the buffered pieces together into one
  fpl->blob_size=fpl->md->total;
  fpl->blob=(unsigned char*)frontier_mem_alloc(fpl->blob_size);
  if(!fpl->blob)
   {
    FRONTIER_MSG(FRONTIER_EMEM);
    fpl->error=FRONTIER_EMEM;
    goto errcleanup;
   }
  mb=fpl->md->firstbuf;
  p=fpl->blob;
  while(mb!=0)
   {
    bcopy(((unsigned char *)mb)+sizeof(*mb),p,mb->len);
    p+=mb->len;
    mb=mb->nextbuf;
   }

  zipped_size=fpl->md->zipped_total;
  frontierMemData_delete(fpl->md);
  fpl->md=0;

  if (zipped)
   {
    // "uncompressed" is more accurate now but leave "uncompressing"
    //   because frontierqueries tool looks for that keyword
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"uncompressing %d byte (full size %d) payload",zipped_size,fpl->full_size);
   }
  else
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"finalizing payload (full size %d)",fpl->blob_size);
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
        n+=sprintf((char *)&dumpdata[n],"%%%02x",c);
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
  if(fpl->md)
   {
    frontierMemData_delete(fpl->md);
    fpl->md=0;
   }
  if(fpl->blob)
   {
    frontier_mem_free(fpl->blob);
    fpl->blob=0;
   }
  return fpl->error;
 }

