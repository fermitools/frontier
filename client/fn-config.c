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
#include <curl/curl.h>
#include <frontier.h>
#include "fn-internal.h"

void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);

static char *str_dup(const char *str)
 {
  char *ret;
  size_t len=strlen(str);
  
  ret=frontier_mem_alloc(len+1);
  bcopy(str,ret,len+1);
  return ret;
 }
 

FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url)
 {
  FrontierConfig *cfg;
  char *env_val;
  int i;
  char buf[1024];
  
  cfg=(FrontierConfig*)frontier_mem_alloc(sizeof(FrontierConfig));
  if(!cfg) return cfg;
  bzero(cfg,sizeof(FrontierConfig));
  
  // If server_url is set then use it and ignore environment settings
  if(server_url && *server_url)
   {
    cfg->server[0]=str_dup(server_url);
    cfg->server_num=1;
    goto set_proxy;
   }
     
  // Get environment settings
  
  // Foolresistance
  env_val=getenv(FRONTIER_ENV_SERVER);
  if(env_val)
   {
    cfg->server[cfg->server_num]=str_dup(env_val);
    printf("Server <%s>\n",cfg->server[cfg->server_num]);    
    cfg->server_num++;
   }
  // Normal settings    
  for(i=0;i<FRONTIER_MAX_SERVERN && cfg->server_num<=FRONTIER_MAX_SERVERN;i++)
   {
    snprintf(buf,1024,"%s%d",FRONTIER_ENV_SERVER,(i+1));
    env_val=getenv(buf);
    if(!env_val) continue; // Another foolresistance - should have break here
    cfg->server[cfg->server_num]=str_dup(env_val);
    printf("Server <%s>\n",cfg->server[cfg->server_num]);    
    cfg->server_num++;
   }
  printf("Total %d servers\n",cfg->server_num);
  
  // Proxy settings
set_proxy:  
  if(proxy_url) 
   {
    if(!*proxy_url) return cfg; // When proxy_url is "" do not use any proxy
    cfg->proxy[0]=str_dup(proxy_url);
    cfg->proxy_num=1;
    return cfg;
   }

  // Foolresistance for proxy settings
  env_val=getenv(FRONTIER_ENV_PROXY);
  if(env_val)
   {
    cfg->proxy[cfg->proxy_num]=str_dup(env_val);
    printf("Proxy <%s>\n",cfg->proxy[cfg->proxy_num]);    
    cfg->proxy_num++;
   }
  // Normal settings    
  for(i=0;i<FRONTIER_MAX_PROXYN && cfg->proxy_num<=FRONTIER_MAX_PROXYN;i++)
   {
    snprintf(buf,1024,"%s%d",FRONTIER_ENV_PROXY,(i+1));
    env_val=getenv(buf);
    if(!env_val) continue;
    cfg->proxy[cfg->proxy_num]=str_dup(env_val);
    printf("Proxy <%s>\n",cfg->proxy[cfg->proxy_num]);    
    cfg->proxy_num++;
   }
  printf("Total %d proxies\n",cfg->proxy_num);     
    
  return cfg;
 }


const char *frontierConfig_getServerUrl(FrontierConfig *cfg)
 {
  return cfg->server[cfg->server_cur];
 }
 

const char *frontierConfig_getProxyUrl(FrontierConfig *cfg)
 {
  if(cfg->proxy_cur>=cfg->proxy_num) return NULL;
  
  return cfg->proxy[cfg->proxy_cur];
 } 


int frontierConfig_nextServer(FrontierConfig *cfg)
 {
  if(cfg->server_cur+1>=cfg->server_num) return -1;
  ++cfg->server_cur;
  //printf("Next server %d\n",cfg->server_cur);
  return 0;
 } 

 
int frontierConfig_nextProxy(FrontierConfig *cfg)
 {
  if(cfg->proxy_cur>=cfg->proxy_num) return -1; // One implicit which is always ""
  ++cfg->proxy_cur;
  return 0;
 } 

   
void frontierConfig_delete(FrontierConfig *cfg)
 {
  int i;
  
  for(i=0;i<cfg->server_num;i++)
   {
    frontier_mem_free(cfg->server[i]);
   }
  for(i=0;i<cfg->proxy_num;i++)
   {
    frontier_mem_free(cfg->proxy[i]);
   }
   
  frontier_mem_free(cfg);
 }

