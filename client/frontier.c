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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <frontier.h>
#include "fn-internal.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


int frontier_log_level;
char *frontier_log_file;
void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);
static int initialized=0;

static void channel_delete(Channel *chn);

char *frontier_str_ncopy(const char *str, size_t len)
 {
  char *ret;
  
  ret=frontier_mem_alloc(len+1);
  if(!ret) return ret;
  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }


char *frontier_str_copy(const char *str)
 {
  int len=strlen(str);
  char *ret;
  
  ret=frontier_mem_alloc(len+1);
  if(!ret) return ret;
  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }
 

int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr))
 {
  char *env;
  
  if(initialized) return FRONTIER_OK;

  if(!f_mem_alloc) {f_mem_alloc=malloc; f_mem_free=free;}
  if(!f_mem_free) {f_mem_alloc=malloc; f_mem_free=free;}

  frontier_mem_alloc=f_mem_alloc;
  frontier_mem_free=f_mem_free;
    
  env=getenv(FRONTIER_ENV_LOG_LEVEL);
  if(!env) 
   {
    frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
    frontier_log_file=(char*)0;
   }
  else
   {
    if(strcasecmp(env,"warning")==0 || strcasecmp(env,"info")==0) frontier_log_level=FRONTIER_LOGLEVEL_WARNING;
    else if(strcasecmp(env,"error")==0) frontier_log_level=FRONTIER_LOGLEVEL_ERROR;
    else if(strcasecmp(env,"nolog")==0) frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
    else frontier_log_level=FRONTIER_LOGLEVEL_DEBUG;
    
    env=getenv(FRONTIER_ENV_LOG_FILE);
    if(!env)
     {
      frontier_log_file=(char*)0;
     }
    else
     {
      int fd=open(env,O_CREAT|O_APPEND|O_WRONLY,0644);
      if(fd<0) 
       {
        printf("Can not open log file %s. Log is disabled.\n",env);
	frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
        frontier_log_file=(char*)0;
       }
      else
       {
        close(fd);
        frontier_log_file=frontier_str_copy(env);
       }
     }
   }

  initialized=1;
  
  return FRONTIER_OK;
 }



static Channel *channel_create(const char *srv,const char *proxy,int *ec)
 {
  Channel *chn;
  int ret;
  const char *p;

  chn=frontier_mem_alloc(sizeof(Channel));
  if(!chn) 
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    return (void*)0;
   }
  bzero(chn,sizeof(Channel));

  chn->cfg=frontierConfig_get(srv,proxy);
  if(!chn->cfg)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    channel_delete(chn);    
    return (void*)0;
   }
  if(!chn->cfg->server_num)
   {
    *ec=FRONTIER_ECFG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no servers configured");
    channel_delete(chn);    
    return (void*)0;
   }
  
  chn->ht_clnt=frontierHttpClnt_create(ec);
  if(!chn->ht_clnt || *ec)
   {
    channel_delete(chn);    
    return (void*)0;
   }
   
  do
   {
    p=frontierConfig_getServerUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addServer(chn->ht_clnt,p);
    if(ret)
     {
      *ec=ret;
      channel_delete(chn);    
      return (void*)0;
     }
   }while(frontierConfig_nextServer(chn->cfg)==0);
   
  if(!chn->ht_clnt->total_server)
   {
    *ec=FRONTIER_ECFG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no server configured");
    channel_delete(chn);    
    return (void*)0;
   }

  do
   {
    p=frontierConfig_getProxyUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addProxy(chn->ht_clnt,p);
    if(ret)
     {
      *ec=ret;
      channel_delete(chn);    
      return (void*)0;
     }
   }while(frontierConfig_nextProxy(chn->cfg)==0);
        
  chn->reload=0;
  *ec=FRONTIER_OK; 
  return chn;
 }


static void channel_delete(Channel *chn)
 {
  if(!chn) return;
  frontierHttpClnt_delete(chn->ht_clnt);
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
  chn->user_reload=reload;
 }
 

static int write_data(FrontierResponse *resp,void *buf,int len)
 {
  int ret;
  ret=FrontierResponse_append(resp,buf,len);
  return ret;
 }


static int prepare_channel(Channel *chn)
 {
  int ec;
  
  if(chn->resp) frontierResponse_delete(chn->resp);
  chn->resp=frontierResponse_create(&ec);  
  if(!chn->resp) return ec;
  
  return FRONTIER_OK;
 }
 
 
static int get_data(Channel *chn,const char *uri)
 {
  int ret=FRONTIER_OK;
  char buf[8192];

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }

  ret=prepare_channel(chn);
  if(ret) return ret;
  
  frontierHttpClnt_setCacheRefreshFlag(chn->ht_clnt,chn->reload);
  
  ret=frontierHttpClnt_open(chn->ht_clnt,uri);
  if(ret) return ret;
  
  while(1)
   {
    ret=frontierHttpClnt_read(chn->ht_clnt,buf,8192);
    if(ret<=0) return ret;
    ret=write_data(chn->resp,buf,ret);
    if(ret!=FRONTIER_OK) 
     return ret;
   }
     
  return FRONTIER_OK;
 }
 

int frontier_getRawData(FrontierChannel u_channel,const char *uri)
 {
  Channel *chn=(Channel*)u_channel;
  int ret=FRONTIER_OK;
  FrontierHttpClnt *clnt;

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }
  
  clnt=chn->ht_clnt;

  chn->reload=chn->user_reload;
  
  while(1)
   {
    ret=get_data(chn,uri);    
    if(ret==FRONTIER_OK) break;    
    frontier_log(FRONTIER_LOGLEVEL_INFO,__FILE__,__LINE__,"Request failed: %d %s",ret,frontier_getErrorMsg());
    
    if(!chn->reload)
     {
      frontier_log(FRONTIER_LOGLEVEL_INFO,__FILE__,__LINE__,"Trying refresh cache");
      chn->reload=1;
      continue;
     }
    chn->reload=chn->user_reload;
    
    if(clnt->cur_proxy<clnt->total_proxy)
     {
      clnt->cur_proxy++;
      frontier_log(FRONTIER_LOGLEVEL_INFO,__FILE__,__LINE__,"Trying next proxy (possibly direct connect)");
      continue;
     }
    if(clnt->cur_server+1<clnt->total_server)
     {
      clnt->cur_server++;
      frontier_log(FRONTIER_LOGLEVEL_INFO,__FILE__,__LINE__,"Trying next server");
      continue;      
     }    
    frontier_setErrorMsg(__FILE__,__LINE__,"No more servers/proxies");
    break;
   }
   
  if(ret) return ret;

  ret=frontierResponse_finalize(chn->resp);
  return ret;
 }


