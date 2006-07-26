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

/* default configuration variables */
static int default_retrieve_zip_level = -1;
static char *default_logical_server = 0;
static char *default_server_list = 0;

#define ENV_BUF_SIZE	1024

void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);

static char *str_dup(const char *str)
 {
  char *ret;
  size_t len=strlen(str);

  ret=frontier_mem_alloc(len+1);
  if (ret != 0)
    bcopy(str,ret,len+1);
  return ret;
 }


FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url,int *errorCode)
 {
  FrontierConfig *cfg;
  int i;
  char buf[ENV_BUF_SIZE];

  cfg=(FrontierConfig*)frontier_mem_alloc(sizeof(FrontierConfig));
  if(!cfg)
   {
    *errorCode = FRONTIER_EMEM;
    return 0;
   }
  bzero(cfg,sizeof(FrontierConfig));

  cfg->retrieve_zip_level = frontierConfig_getDefaultRetrieveZipLevel();
  /* keep a local, duplicated, copy of the logical server name for the
     unlikely possibility that somebody wants to set it to something else
     for another connection while this config object is still active */
  cfg->logical_server = frontierConfig_getDefaultLogicalServer();
  if (cfg->logical_server != 0)
   {
    cfg->logical_server = str_dup(cfg->logical_server);
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Set logical server to %s", cfg->logical_server);
   }
  
  // First add configured server and proxy.
  *errorCode = frontierConfig_addServer(cfg, server_url);
  if (*errorCode != FRONTIER_OK) goto cleanup;
  *errorCode = frontierConfig_addProxy(cfg, proxy_url);
  if (*errorCode != FRONTIER_OK) goto cleanup;
 
  // Add additional servers/proxies from env variables.
  *errorCode = frontierConfig_addServer(cfg, frontierConfig_getDefaultServerList());
  if (*errorCode != FRONTIER_OK) goto cleanup;
  *errorCode = frontierConfig_addProxy(cfg, getenv(FRONTIER_ENV_PROXY));
  if (*errorCode != FRONTIER_OK) goto cleanup;
  for(i = 0; i < FRONTIER_MAX_SERVERN; i++) {
    snprintf(buf, ENV_BUF_SIZE, "%s%d", FRONTIER_ENV_SERVER, (i+1));
    *errorCode = frontierConfig_addServer(cfg, getenv(buf));
    if (*errorCode != FRONTIER_OK) goto cleanup;
  }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Total %d servers", cfg->server_num);
  
  for(i = 0; i < FRONTIER_MAX_PROXYN; i++) {
    snprintf(buf, ENV_BUF_SIZE, "%s%d", FRONTIER_ENV_PROXY, (i+1));
    *errorCode = frontierConfig_addProxy(cfg, getenv(buf));
    if (*errorCode != FRONTIER_OK) goto cleanup;
  }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Total %d proxies", cfg->proxy_num);     
    
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Retrieve zip level is %d", cfg->retrieve_zip_level);     

  return cfg;

cleanup:
  frontierConfig_delete(cfg);
  return 0;
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

  if (cfg->logical_server != 0)
    frontier_mem_free(cfg->logical_server);
   
  frontier_mem_free(cfg);
 }

int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg)
 {
  return cfg->retrieve_zip_level;
 }

void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level)
 {
  if (level < 0)
    level = 0;
  if (level > 9)
    level = 9;
  cfg->retrieve_zip_level = level;
 }

void frontierConfig_setDefaultRetrieveZipLevel(int level)
 {
  if (level < 0)
    level = 0;
  if (level > 9)
    level = 9;
  default_retrieve_zip_level = level;
 }

int frontierConfig_getDefaultRetrieveZipLevel()
 {
  if (default_retrieve_zip_level == -1)
   {
    /* not yet been initialized */
    char *p;
    p = getenv(FRONTIER_ENV_RETRIEVEZIPLEVEL);
    if ((p != 0) && (*p != '\0'))
      frontierConfig_setDefaultRetrieveZipLevel(atoi(p));
   }
  if (default_retrieve_zip_level == -1)
    default_retrieve_zip_level = 5;
  return default_retrieve_zip_level;
 }

void frontierConfig_setDefaultLogicalServer(const char *logical_server)
 {
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Setting default logical server to %s", logical_server);     
  if (default_logical_server != 0)
    frontier_mem_free(default_logical_server);
  if (logical_server != 0)
    default_logical_server = str_dup(logical_server);
  else
    default_logical_server = 0;
 }

char *frontierConfig_getDefaultLogicalServer()
 {
  if (default_logical_server == 0)
   {
    /* try the environment */
   frontierConfig_setDefaultLogicalServer(getenv(FRONTIER_ENV_LOGICALSERVER));
   }
  return default_logical_server;
 }

void frontierConfig_setDefaultServerList(const char *server_list)
 {
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Setting default server list to %s", server_list);     
  if (default_server_list != 0)
    frontier_mem_free(default_server_list);
  if (server_list != 0)
    default_server_list = str_dup(server_list);
  else
    default_server_list = 0;
 }

char *frontierConfig_getDefaultServerList()
 {
  if (default_server_list == 0)
   {
    /* try the environment */
   frontierConfig_setDefaultServerList(getenv(FRONTIER_ENV_SERVER));
   }
  return default_server_list;
 }

int frontierConfig_parseComplexServerSpec(FrontierConfig *cfg, const char* server_spec)
 {
  char *str = str_dup(server_spec);
  char *p = str-1;
  char *keyp = 0, *valp = 0;
  int nestlevel = 0;
  int ret;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Parsing complex server spec %s",server_spec);    

  if (str == NULL)
    return FRONTIER_EMEM;
   
  while (*++p)
   {
    switch(*p)
     {
      case '(':
	if (nestlevel == 0)
	 {
	  keyp = p+1;
	  valp = 0;
	 }
        nestlevel++;
        break;
      case '=':
        if (nestlevel != 1)
	  continue;
	*p = '\0';
	valp = p+1;
	break;
      case ')':
        --nestlevel;
        if (nestlevel < 0)
	 {
	  frontier_setErrorMsg(__FILE__, __LINE__, "Too many close-parens in server spec");
	  ret = FRONTIER_ECFG;
	  goto cleanup;
	 }
	if (nestlevel > 0)
	  continue;
	*p = '\0';
	if (valp == 0)
	 {
	  if (keyp == p)
	    /* empty parens */
	    continue;
	  frontier_setErrorMsg(__FILE__, __LINE__, "No '=' after keyword %s",keyp);
	  ret = FRONTIER_ECFG;
	  goto cleanup;
	 }
	ret = FRONTIER_OK;
	if (strcmp(keyp, "serverurl") == 0)
	  ret = frontierConfig_addServer(cfg, valp);
	else if (strcmp(keyp, "proxyurl") == 0)
	  ret = frontierConfig_addProxy(cfg, valp);
	else if (strcmp(keyp, "retrieve-ziplevel") == 0)
	  frontierConfig_setRetrieveZipLevel(cfg, atoi(valp));
	else
	 {
	 /* else ignore unrecognized keys */
	  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Unrecognized keyword %s",keyp);
	 }

	if (ret != FRONTIER_OK)
	  goto cleanup;
	break;
     }
   }

  if (nestlevel > 0)
   {
    frontier_setErrorMsg(__FILE__, __LINE__, "Unmatched parentheses");
    ret = FRONTIER_ECFG;
    goto cleanup;
   }

  ret = FRONTIER_OK;

cleanup:
  frontier_mem_free(str);
  return ret;
 }

int frontierConfig_addServer(FrontierConfig *cfg, const char* server_url)
 {
  if ((server_url != 0) && (strchr(server_url, '(') != NULL))
    return frontierConfig_parseComplexServerSpec(cfg, server_url);

  if(!server_url) {
    return FRONTIER_OK;
  }
  else {
    if(!*server_url) {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Empty server url.");    
      return FRONTIER_OK;
    }
  }
  
  if ((cfg->logical_server != 0) && (strcmp(server_url,cfg->logical_server) == 0))
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Matched logical server %s", server_url);
    /* just ignore it when there's a logical server match */
    return FRONTIER_OK;
   }

  /* Ready to insert new server, make sure there's room */
  if(cfg->server_num >= FRONTIER_MAX_SERVERN) {
    frontier_setErrorMsg(__FILE__, __LINE__, 
      "Reached limit of %d frontier servers", FRONTIER_MAX_SERVERN);    
    return FRONTIER_ECFG;
  }

  /* Everything ok, insert new server. */
  cfg->server[cfg->server_num] = str_dup(server_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
        "Added server <%s>", cfg->server[cfg->server_num]);    
  cfg->server_num++;
  return FRONTIER_OK;
 } 

int frontierConfig_addProxy(FrontierConfig *cfg, const char* proxy_url)
 {
  if(!proxy_url) {
    return FRONTIER_OK;
  }
  else {
    if(!*proxy_url) {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Empty proxy url.");    
      return FRONTIER_OK;
    }
  }

  /* Ready to insert new proxy, make sure there's room */
  if(cfg->proxy_num >= FRONTIER_MAX_PROXYN) {
    frontier_setErrorMsg(__FILE__, __LINE__, 
      "Reached limit of %d frontier proxies", FRONTIER_MAX_PROXYN);    
    return FRONTIER_ECFG;
  }

  /* Everything ok, insert new proxy. */
  cfg->proxy[cfg->proxy_num] = str_dup(proxy_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, 
        "Added proxy <%s>", cfg->proxy[cfg->proxy_num]);    
  cfg->proxy_num++;
  return FRONTIER_OK;
 } 

