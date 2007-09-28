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
static int default_connect_timeout_secs = -1;
static int default_read_timeout_secs = -1;
static int default_write_timeout_secs = -1;
static char *default_force_reload = 0;
static int default_retrieve_zip_level = -1;
static char *default_logical_server = 0;
static char *default_physical_servers = 0;

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
  char *env;

  cfg=(FrontierConfig*)frontier_mem_alloc(sizeof(FrontierConfig));
  if(!cfg)
   {
    *errorCode = FRONTIER_EMEM;
    return 0;
   }
  bzero(cfg,sizeof(FrontierConfig));


  // Set initial retrieve zip level first because it may be overridden
  //  by a complex server string next.
  frontierConfig_setRetrieveZipLevel(cfg,frontierConfig_getDefaultRetrieveZipLevel());

  // Likewise for the timeouts and forcereload
  if(default_connect_timeout_secs==-1)
   {
    if((env=getenv(FRONTIER_ENV_CONNECTTIMEOUTSECS))==0)
     {
      // The default connect timeout should be quite short, to fail over to
      //   the next server if one of the servers is down.
      default_connect_timeout_secs=5;
     }
    else
     default_connect_timeout_secs=atoi(env);
   }
  frontierConfig_setConnectTimeoutSecs(cfg,default_connect_timeout_secs);

  if(default_read_timeout_secs==-1)
   {
    if((env=getenv(FRONTIER_ENV_READTIMEOUTSECS))==0)
     {
      // Server should send data at least every 5 seconds; allow for double.
      default_read_timeout_secs=10;
     }
    else
     default_read_timeout_secs=atoi(env);
   }
  frontierConfig_setReadTimeoutSecs(cfg,default_read_timeout_secs);

  if(default_write_timeout_secs==-1)
   {
    if((env=getenv(FRONTIER_ENV_WRITETIMEOUTSECS))==0)
     {
      // Writes shouldn't take very long unless there is much queued,
      //   which doesn't happen in the frontier client.
      default_write_timeout_secs=5;
     }
    else
     default_write_timeout_secs=atoi(env);
   }
  frontierConfig_setWriteTimeoutSecs(cfg,default_write_timeout_secs);

  if(default_force_reload==0)
   {
    if((env=getenv(FRONTIER_ENV_FORCERELOAD))==0)
      default_force_reload="none";
    else
     default_force_reload=env;
   }
  frontierConfig_setForceReload(cfg,default_force_reload);

  // Add configured server and proxy.
  *errorCode=frontierConfig_addServer(cfg,server_url);
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  *errorCode=frontierConfig_addProxy(cfg,proxy_url);
  if(*errorCode!=FRONTIER_OK)goto cleanup;
 
  // Add additional servers/proxies from env variables.
  *errorCode=frontierConfig_addServer(cfg,getenv(FRONTIER_ENV_SERVER));
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  *errorCode=frontierConfig_addProxy(cfg,getenv(FRONTIER_ENV_PROXY));
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  for(i=0;i<FRONTIER_MAX_SERVERN;i++)
   {
    snprintf(buf,ENV_BUF_SIZE,"%s%d",FRONTIER_ENV_SERVER,(i+1));
    *errorCode=frontierConfig_addServer(cfg,getenv(buf));
    if(*errorCode!=FRONTIER_OK)goto cleanup;
   }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Total %d servers",cfg->server_num);
  
  for(i=0;i<FRONTIER_MAX_PROXYN;i++)
   {
    snprintf(buf,ENV_BUF_SIZE,"%s%d",FRONTIER_ENV_PROXY,(i+1));
    *errorCode=frontierConfig_addProxy(cfg,getenv(buf));
    if(*errorCode!=FRONTIER_OK)goto cleanup;
   }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Total %d proxies",cfg->proxy_num);
    
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Retrieve zip level is %d",cfg->retrieve_zip_level);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Connect timeoutsecs is %d",cfg->connect_timeout_secs);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Read timeoutsecs is %d",cfg->read_timeout_secs);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Write timeoutsecs is %d",cfg->write_timeout_secs);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Force reload is %s",cfg->force_reload);

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
  
  if(cfg->force_reload!=0)frontier_mem_free(cfg->force_reload);

  frontier_mem_free(cfg);
 }

void frontierConfig_setConnectTimeoutSecs(FrontierConfig *cfg,int timeoutsecs)
 {
  cfg->connect_timeout_secs=timeoutsecs;
 }

int frontierConfig_getConnectTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->connect_timeout_secs;
 }

void frontierConfig_setReadTimeoutSecs(FrontierConfig *cfg,int timeoutsecs)
 {
  cfg->read_timeout_secs=timeoutsecs;
 }

int frontierConfig_getReadTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->read_timeout_secs;
 }

void frontierConfig_setForceReload(FrontierConfig *cfg,char *forcereload)
 {
  if(cfg->force_reload!=0)frontier_mem_free(cfg->force_reload);
  cfg->force_reload=str_dup(forcereload);
 }

const char *frontierConfig_getForceReload(FrontierConfig *cfg)
 {
  return cfg->force_reload;
 }

void frontierConfig_setWriteTimeoutSecs(FrontierConfig *cfg,int timeoutsecs)
 {
  cfg->write_timeout_secs=timeoutsecs;
 }

int frontierConfig_getWriteTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->write_timeout_secs;
 }

void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level)
 {
  if (level < 0)
    level = 0;
  if (level > 9)
    level = 9;
  cfg->retrieve_zip_level = level;
 }

int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg)
 {
  return cfg->retrieve_zip_level;
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
  if (logical_server != default_logical_server)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Setting default logical server to %s", logical_server);
  if (default_logical_server != 0)
   {
    if ((logical_server != 0) &&
	(strcmp(logical_server, default_logical_server) == 0))
      return;
    frontier_mem_free(default_logical_server);
   }
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

void frontierConfig_setDefaultPhysicalServers(const char *physical_servers)
 {
  if (physical_servers != default_physical_servers)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Setting default physical servers to %s", physical_servers);     
  if (default_physical_servers != 0)
   {
    if ((physical_servers != 0) &&
	(strcmp(physical_servers, default_physical_servers) == 0))
      return;
    frontier_mem_free(default_physical_servers);
   }
  if (physical_servers != 0)
    default_physical_servers = str_dup(physical_servers);
  else
    default_physical_servers = 0;
 }

char *frontierConfig_getDefaultPhysicalServers()
 {
  if (default_physical_servers == 0)
   {
    /* try the environment */
   frontierConfig_setDefaultPhysicalServers(getenv(FRONTIER_ENV_PHYSICALSERVERS));
   }
  return default_physical_servers;
 }

static int frontierConfig_parseComplexServerSpec(FrontierConfig *cfg, const char* server_spec)
 {
  char *str = str_dup(server_spec);
  char *p = str-1;
  char *keyp = 0, *valp = 0;
  int nestlevel = 0;
  int ret;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Parsing complex server spec %s",server_spec);    

  if (str == NULL)
    return FRONTIER_EMEM;

  if ((keyp = strstr(server_spec, "(logicalserverurl=")) != NULL)
   {
    // save all but this keyword as the default physical servers for
    //  later connections
    char *nextp = strchr(keyp,')');
    if (nextp != NULL)
     {
      char *cp = (char *)frontier_mem_alloc(strlen(server_spec));
      if (cp != NULL)
       {
        strncpy(cp,server_spec,keyp-server_spec);
        strcpy(cp+(keyp-server_spec),nextp+1);
        frontierConfig_setDefaultPhysicalServers(cp);
        frontier_mem_free(cp);
       }
     }
   }

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
	else if (strcmp(keyp, "logicalserverurl") == 0)
	  frontierConfig_setDefaultLogicalServer(valp);
	else if (strcmp(keyp, "retrieve-ziplevel") == 0)
	  frontierConfig_setRetrieveZipLevel(cfg, atoi(valp));
	else if (strcmp(keyp, "connecttimeoutsecs") == 0)
	  frontierConfig_setConnectTimeoutSecs(cfg, atoi(valp));
	else if (strcmp(keyp, "readtimeoutsecs") == 0)
	  frontierConfig_setReadTimeoutSecs(cfg, atoi(valp));
	else if (strcmp(keyp, "writetimeoutsecs") == 0)
	  frontierConfig_setWriteTimeoutSecs(cfg, atoi(valp));
	else if (strcmp(keyp, "forcereload") == 0)
	  frontierConfig_setForceReload(cfg, valp);
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
  const char *logical_server;

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
  
  logical_server = frontierConfig_getDefaultLogicalServer();
  if ((logical_server != 0) && (strcmp(server_url,logical_server) == 0))
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG, __FILE__, __LINE__, "Matched logical server %s", server_url);
    /* replace it with the physical servers */
    frontierConfig_addServer(cfg, frontierConfig_getDefaultPhysicalServers());
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

