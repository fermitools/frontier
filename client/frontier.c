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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <frontier.h>
#include "fn-internal.h"

void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);
static int initialized=0;


static const char *str_copy(const char *str, size_t len)
 {
  char *ret=frontier_mem_alloc(len+1);

  if(!ret) return ret;

  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }


int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr))
 {
  if(initialized) return FRONTIER_OK;

  if(!f_mem_alloc) {f_mem_alloc=malloc; f_mem_free=free;}
  if(!f_mem_free) {f_mem_alloc=malloc; f_mem_free=free;}

  frontier_mem_alloc=f_mem_alloc;
  frontier_mem_free=f_mem_free;

  initialized=1;
  
  return FRONTIER_OK;
 }



static Channel *channel_create(const char *srv,const char *proxy,int *ec)
 {
  Channel *chn;
  int ret;
  const char *p;

  chn=frontier_mem_alloc(sizeof(Channel));
  if(!chn) {*ec=FRONTIER_EMEM;return chn;}

  chn->cfg=frontierConfig_get(srv,proxy);
  if(!chn->cfg)
   {
    frontier_mem_free(chn);
    *ec=FRONTIER_EMEM;
    return (void*)0;   
   }
  if(!chn->cfg->server_num)
   {
    frontierConfig_delete(chn->cfg);
    frontier_mem_free(chn);
    *ec=FRONTIER_EENVSRV;
    return (void*)0;      
   }
  
  chn->resp=(void*)0;
 
  chn->ht_clnt=frontierHttpClnt_create();
  if(!chn->ht_clnt)
   {
    frontierConfig_delete(chn->cfg);
    frontier_mem_free(chn);
    *ec=FRONTIER_EMEM;
    return (void*)0;    
   }
   
  do
   {
    p=frontierConfig_getServerUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addServer(chn->ht_clnt,p);
    if(ret)
     {
      printf("Error: %s\n",chn->ht_clnt->err_msg);
      frontierHttpClnt_delete(chn->ht_clnt);
      frontierConfig_delete(chn->cfg);
      frontier_mem_free(chn);
      *ec=FRONTIER_EIARG;
      return (void*)0;    
     }
   }while(frontierConfig_nextServer(chn->cfg)==0);
   
  if(!chn->ht_clnt->total_server)
   {
    printf("Error: no servers configured\n");
    frontierHttpClnt_delete(chn->ht_clnt);
    frontierConfig_delete(chn->cfg);
    frontier_mem_free(chn);
    *ec=FRONTIER_EIARG;
    return (void*)0;       
   }

  do
   {
    p=frontierConfig_getProxyUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addProxy(chn->ht_clnt,p);
    if(ret)
     {
      printf("Error: %s\n",chn->ht_clnt->err_msg);
      frontierHttpClnt_delete(chn->ht_clnt);
      frontierConfig_delete(chn->cfg);
      frontier_mem_free(chn);
      *ec=FRONTIER_EIARG;
      return (void*)0;    
     }
   }while(frontierConfig_nextProxy(chn->cfg)==0);
        
  chn->status=FRONTIER_SEMPTY;
  chn->reload=0;

  *ec=FRONTIER_OK;
 
  return chn;
 }


static void channel_delete(Channel *chn)
 {
  if(!chn) return;
  if(chn->ht_clnt) frontierHttpClnt_delete(chn->ht_clnt);
  frontierResponse_delete(chn->resp);

  frontierConfig_delete(chn->cfg);
  
  frontier_mem_free(chn);
 }


FrontierChannel frontier_createChannel(const char *srv,const char *proxy,int *ec)
 {
  Channel *chn=channel_create(srv,proxy,ec);
  return (unsigned long)chn;
 }


void frontier_closeChannel(FrontierChannel fchn)
 {
  channel_delete((Channel*)fchn);
 }

 
void frontier_setReload(FrontierChannel u_channel,int reload)
 {
  Channel *chn=(Channel*)u_channel;
  
  chn->reload=reload;
 }
 

static int write_data(FrontierResponse *resp,void *buf,int len)
 {
  int ret;

  ret=FrontierResponse_append(resp,buf,len);
  
  return ret;
 }


static int prepare_channel(Channel *chn)
 {  
  if(chn->resp) frontierResponse_delete(chn->resp);
  chn->resp=frontierResponse_create();  
  if(!chn->resp) return FRONTIER_EMEM;
  
  chn->status=FRONTIER_SEMPTY; 
    
  return FRONTIER_OK;
 }
 
 
static int get_data(Channel *chn,const char *uri)
 {
  int ret=FRONTIER_OK;
  char buf[8192];

  if(!chn) return FRONTIER_EIARG;

  ret=prepare_channel(chn);
  if(ret) return ret;

  printf("uri <%s>\n",uri);
  
  frontierHttpClnt_setCacheRefreshFlag(chn->ht_clnt,chn->reload);
  
  ret=frontierHttpClnt_open(chn->ht_clnt,uri);
  if(ret) return -1;
  
  while(1)
   {
    ret=frontierHttpClnt_read(chn->ht_clnt,buf,8192);
    if(ret<=0) return ret;
    ret=write_data(chn->resp,buf,ret);
    if(ret!=FRONTIER_OK) return ret;
   }
     
  return FRONTIER_OK;
 }
 

int frontier_getRawData(FrontierChannel u_channel,const char *uri)
 {
  Channel *chn=(Channel*)u_channel;
  int ret=FRONTIER_OK;
  FrontierHttpClnt *clnt;

  if(!chn) {ret=FRONTIER_EIARG; goto err;}
  
  clnt=chn->ht_clnt;

  while(1)
   {
    ret=get_data(chn,uri);
    //printf("ret=%d\n",ret);
    if(ret==FRONTIER_OK) break;
    
    if(clnt->cur_proxy<clnt->total_proxy)
     {
      clnt->cur_proxy++;
      printf("Trying next proxy (possibly direct connect)\n");
      continue;
     }
    if(clnt->cur_server+1<clnt->total_server)
     {
      clnt->cur_server++;
      printf("Trying next server\n");
      continue;      
     }    
    printf("No more servers/proxies\n");
    break;
   }
   
  if(ret) goto err;

  ret=frontierResponse_finalize(chn->resp);
  if(ret!=FRONTIER_OK) goto err;
 
  chn->status=FRONTIER_SRAW_DATA;
  goto Ok;

err:
  chn->status=FRONTIER_SERROR;
Ok:
  chn->error=ret;
  return ret;
 }


void frontier_getRespStat(FrontierChannel u_channel,FrontierRespStat *stat)
 {
  Channel *chn=(Channel*)u_channel;

  if(!chn)
   {
    stat->status=FRONTIER_SERROR;
    stat->error=FRONTIER_EIARG;
    return;
   }

  stat->status=chn->status;
  stat->error=chn->error;
  stat->http_resp_code=chn->http_resp_code;
  stat->raw_data_size=0; // XXX
  stat->raw_data_buf=(void*)0; // XXX
 }

