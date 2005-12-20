/*
 * Frontier configuration.
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

#include "frontier_client/frontier_config.h"
#include "frontier_client/frontier_log.h"
#include "frontier_client/frontier_error.h"

#define ENV_BUF_SIZE	1024

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
  int i;
  char buf[ENV_BUF_SIZE];
  
  cfg=(FrontierConfig*)frontier_mem_alloc(sizeof(FrontierConfig));
  if(!cfg) return cfg;
  bzero(cfg,sizeof(FrontierConfig));
  
  // First add configured server and proxy.
  frontierConfig_addServer(cfg, server_url);
  frontierConfig_addProxy(cfg, proxy_url);
 
  // Add additional servers/proxies from env variables.
  frontierConfig_addServer(cfg, getenv(FRONTIER_ENV_SERVER));
  frontierConfig_addServer(cfg, getenv(FRONTIER_ENV_PROXY));
  for(i = 0; i < FRONTIER_MAX_SERVERN; i++) {
    snprintf(buf, ENV_BUF_SIZE, "%s%d", FRONTIER_ENV_SERVER, (i+1));
    frontierConfig_addServer(cfg, getenv(buf));
  }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Total %d servers", cfg->server_num);
  
  for(i = 0; i < FRONTIER_MAX_PROXYN; i++) {
    snprintf(buf, ENV_BUF_SIZE, "%s%d", FRONTIER_ENV_PROXY, (i+1));
    frontierConfig_addProxy(cfg, getenv(buf));
  }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Total %d proxies", cfg->proxy_num);     
    
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
  
  if(!cfg) return;
  
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

int frontierConfig_addServer(FrontierConfig *cfg, const char* server_url)
 {
  if(cfg->server_num >= FRONTIER_MAX_SERVERN) {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
      "Reached limit of %d frontier servers", FRONTIER_MAX_SERVERN);    
    return FRONTIER_ECFG;
  }

  if(!server_url) {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Undefined server url.");    
    return FRONTIER_ECFG;
  }
  else {
    if(!*server_url) {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Empty server url.");    
      return FRONTIER_ECFG;
    }
  }
  cfg->server[cfg->server_num] = str_dup(server_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
        "Added server <%s>", cfg->server[cfg->server_num]);    
  cfg->server_num++;
  return FRONTIER_OK;
 } 

int frontierConfig_addProxy(FrontierConfig *cfg, const char* proxy_url)
 {
  if(cfg->proxy_num >= FRONTIER_MAX_PROXYN) {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
      "Reached limit of %d frontier proxies", FRONTIER_MAX_PROXYN);    
    return FRONTIER_ECFG;
  }

  if(!proxy_url) {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Undefined proxy url.");    
    return FRONTIER_ECFG;
  }
  else {
    if(!*proxy_url) {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Empty proxy url.");    
      return FRONTIER_ECFG;
    }
  }
  cfg->proxy[cfg->proxy_num] = str_dup(proxy_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
        "Added proxy <%s>", cfg->proxy[cfg->proxy_num]);    
  cfg->proxy_num++;
  return FRONTIER_OK;
 } 

