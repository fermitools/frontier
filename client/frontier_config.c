/*
 * frontier client configuration handling
 * 
 * Author: Sinisa Veseli
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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "pacparser.h"
#include "fn-internal.h"
#include <http/fn-htclient.h>
#include "frontier_client/frontier_config.h"
#include "frontier_client/frontier_log.h"
#include "frontier_client/frontier_error.h"
#include "frontier_client/frontier.h"

/* default configuration variables */
static int default_connect_timeout_secs=-1;
static int default_read_timeout_secs=-1;
static int default_write_timeout_secs=-1;
static int default_max_age_secs=-1;
static int default_prefer_ip_family=4;
static int default_secured=0;
static char *default_capath=0;
static char *default_force_reload=0;
static char *default_freshkey=0;
static int default_retrieve_zip_level=-1;
static char *default_logical_server=0;
static char *default_physical_servers=0;

#define ENV_BUF_SIZE	1024

int frontier_pacparser_init(void);

static int getNumNonBackupProxies(FrontierConfig *cfg)
 {
  return cfg->proxy_num-cfg->num_backupproxies;
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
    *errorCode=FRONTIER_EMEM;
    FRONTIER_MSG(*errorCode);
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

  if(default_capath==0)
   {
    if((env=getenv("X509_CERT_DIR"))==0)
      default_capath="/etc/grid-security/certificates";
    else
      default_capath=env;
   }
  frontierConfig_setCAPath(cfg,default_capath);

  // No env variable for these
  frontierConfig_setMaxAgeSecs(cfg,default_max_age_secs);
  frontierConfig_setPreferIpFamily(cfg,default_prefer_ip_family);
  frontierConfig_setSecured(cfg,default_secured);

  if(default_force_reload==0)
   {
    if((env=getenv(FRONTIER_ENV_FORCERELOAD))==0)
      default_force_reload="none";
    else
      default_force_reload=env;
   }
  frontierConfig_setForceReload(cfg,default_force_reload);

  if(default_freshkey==0)
   {
    if((env=getenv(FRONTIER_ENV_FRESHKEY))==0)
      default_freshkey="";
    else
      default_freshkey=env;
   }
  frontierConfig_setFreshkey(cfg,default_freshkey);

  // Default on this is not set in environment so just set it here
  frontierConfig_setClientCacheMaxResultSize(cfg,FRONTIER_DEFAULT_CLIENTCACHEMAXRESULTSIZE);

  // FailoverToServer is always true unless turned off in complex server string
  frontierConfig_setFailoverToServer(cfg,1);

  // Add configured server and proxy.
  *errorCode=frontierConfig_addServer(cfg,server_url);
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  *errorCode=frontierConfig_addProxy(cfg,proxy_url,0);
  if(*errorCode!=FRONTIER_OK)goto cleanup;
 
  // Add additional servers/proxies from env variables.
  *errorCode=frontierConfig_addServer(cfg,getenv(FRONTIER_ENV_SERVER));
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  *errorCode=frontierConfig_addProxy(cfg,getenv(FRONTIER_ENV_PROXY),0);
  if(*errorCode!=FRONTIER_OK)goto cleanup;
  for(i=0;i<FRONTIER_MAX_SERVERN;i++)
   {
    snprintf(buf,ENV_BUF_SIZE,"%s%d",FRONTIER_ENV_SERVER,(i+1));
    *errorCode=frontierConfig_addServer(cfg,getenv(buf));
    if(*errorCode!=FRONTIER_OK)goto cleanup;
   }
  
  for(i=0;i<FRONTIER_MAX_PROXYN;i++)
   {
    snprintf(buf,ENV_BUF_SIZE,"%s%d",FRONTIER_ENV_PROXY,(i+1));
    *errorCode=frontierConfig_addProxy(cfg,getenv(buf),0);
    if(*errorCode!=FRONTIER_OK)goto cleanup;
   }

  env=getenv(FRONTIER_ENV_PROXYCONFIGS);
  while(env!=0)
   {
    char *p=strchr(env,';');
    if(p!=0)
      *p='\0';
    *errorCode=frontierConfig_addProxyConfig(cfg,env);
    env=p;
    if(p!=0)
     {
      *p=';';
      env++;
     }
    if(*errorCode!=FRONTIER_OK)goto cleanup;
   }

  *errorCode=frontierConfig_doProxyConfig(cfg);
  if(*errorCode!=FRONTIER_OK)goto cleanup;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Total %d servers",cfg->server_num);
  if (cfg->servers_balanced)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Servers load balanced");
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Total %d proxies",cfg->proxy_num);
  if (cfg->proxies_balanced)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%d proxies load balanced",getNumNonBackupProxies(cfg));
    
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Retrieve zip level is %d",cfg->retrieve_zip_level);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Connect timeoutsecs is %d",cfg->connect_timeout_secs);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Read timeoutsecs is %d",cfg->read_timeout_secs);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Write timeoutsecs is %d",cfg->write_timeout_secs);
  if(strcmp(cfg->force_reload,"none")!=0)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Force reload is %s",cfg->force_reload);
  if(cfg->max_age_secs>=0)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Max age secs is %d",cfg->max_age_secs);
  if(*cfg->freshkey!='\0')
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Freshkey is %s",cfg->freshkey);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Client cache max result size is %d",cfg->client_cache_max_result_size);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Failover to server is %s",cfg->failover_to_server?"yes":"no");

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
  if(cfg->proxy_cur>=cfg->proxy_num)return NULL;
  
  return cfg->proxy[cfg->proxy_cur];
 } 


int frontierConfig_nextServer(FrontierConfig *cfg)
 {
  if(cfg->server_cur+1>=cfg->server_num)return -1;
  ++cfg->server_cur;
  //printf("Next server %d\n",cfg->server_cur);
  return 0;
 } 

 
int frontierConfig_nextProxy(FrontierConfig *cfg)
 {
  if(cfg->proxy_cur>=cfg->proxy_num)return -1; // One implicit which is always ""
  ++cfg->proxy_cur;
  return 0;
 } 

   
void frontierConfig_delete(FrontierConfig *cfg)
 {
  int i;
  
  if(!cfg)return;
  
  for(i=0;i<cfg->server_num;i++)
   {
    frontier_mem_free(cfg->server[i]);
   }
  for(i=0;i<cfg->proxy_num;i++)
   {
    frontier_mem_free(cfg->proxy[i]);
   }
  for(i=0;i<cfg->proxyconfig_num;i++)
   {
    frontier_mem_free(cfg->proxyconfig[i]);
   }
  
  if(cfg->force_reload!=0)frontier_mem_free(cfg->force_reload);

  if(cfg->freshkey!=0)frontier_mem_free(cfg->freshkey);

  if(cfg->capath!=0)frontier_mem_free(cfg->capath);

  frontier_mem_free(cfg);
 }

void frontierConfig_setConnectTimeoutSecs(FrontierConfig *cfg,int secs)
 {
  cfg->connect_timeout_secs=secs;
 }

int frontierConfig_getConnectTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->connect_timeout_secs;
 }

void frontierConfig_setReadTimeoutSecs(FrontierConfig *cfg,int secs)
 {
  cfg->read_timeout_secs=secs;
 }

int frontierConfig_getReadTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->read_timeout_secs;
 }

void frontierConfig_setWriteTimeoutSecs(FrontierConfig *cfg,int secs)
 {
  cfg->write_timeout_secs=secs;
 }

int frontierConfig_getWriteTimeoutSecs(FrontierConfig *cfg)
 {
  return cfg->write_timeout_secs;
 }

void frontierConfig_setMaxAgeSecs(FrontierConfig *cfg,int secs)
 {
  cfg->max_age_secs=secs;
 }

int frontierConfig_getMaxAgeSecs(FrontierConfig *cfg)
 {
  return cfg->max_age_secs;
 }

void frontierConfig_setPreferIpFamily(FrontierConfig *cfg,int ipfamily)
 {
  if((ipfamily!=4)&&(ipfamily!=6))
    ipfamily=0;
  cfg->prefer_ip_family=ipfamily;
 }

int frontierConfig_getPreferIpFamily(FrontierConfig *cfg)
 {
  return cfg->prefer_ip_family;
 }

void frontierConfig_setForceReload(FrontierConfig *cfg,char *forcereload)
 {
  if(cfg->force_reload!=0)frontier_mem_free(cfg->force_reload);
  cfg->force_reload=frontier_str_copy(forcereload);
 }

const char *frontierConfig_getForceReload(FrontierConfig *cfg)
 {
  return cfg->force_reload;
 }

void frontierConfig_setFreshkey(FrontierConfig *cfg,char *freshkey)
 {
  if(cfg->freshkey!=0)frontier_mem_free(cfg->freshkey);
  cfg->freshkey=frontier_str_copy(freshkey);
 }

const char *frontierConfig_getFreshkey(FrontierConfig *cfg)
 {
  return cfg->freshkey;
 }

void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level)
 {
  if(level<0)
    level=0;
  if(level>9)
    level=9;
  cfg->retrieve_zip_level=level;
 }

int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg)
 {
  return cfg->retrieve_zip_level;
 }

void frontierConfig_setDefaultRetrieveZipLevel(int level)
 {
  if(level<0)
    level=0;
  if(level>9)
    level=9;
  default_retrieve_zip_level=level;
 }

int frontierConfig_getDefaultRetrieveZipLevel()
 {
  if(default_retrieve_zip_level==-1)
   {
    /* not yet been initialized */
    char *p;
    p=getenv(FRONTIER_ENV_RETRIEVEZIPLEVEL);
    if((p!=0)&&(*p!='\0'))
      frontierConfig_setDefaultRetrieveZipLevel(atoi(p));
   }
  if(default_retrieve_zip_level==-1)
    default_retrieve_zip_level=5;
  return default_retrieve_zip_level;
 }

void frontierConfig_setDefaultLogicalServer(const char *logical_server)
 {
  if(logical_server!=default_logical_server)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Setting default logical server to %s",logical_server);
  if(default_logical_server!=0)
   {
    if((logical_server!=0)&&
	(strcmp(logical_server,default_logical_server)==0))
      return;
    frontier_mem_free(default_logical_server);
   }
  if(logical_server!=0)
    default_logical_server=frontier_str_copy(logical_server);
  else
    default_logical_server=0;
 }

char *frontierConfig_getDefaultLogicalServer()
 {
  if(default_logical_server==0)
   {
    /* try the environment */
   frontierConfig_setDefaultLogicalServer(getenv(FRONTIER_ENV_LOGICALSERVER));
   }
  return default_logical_server;
 }

void frontierConfig_setDefaultPhysicalServers(const char *physical_servers)
 {
  if(physical_servers!=default_physical_servers)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Setting default physical servers to %s",physical_servers);
  if(default_physical_servers!=0)
   {
    if((physical_servers!=0)&&
	(strcmp(physical_servers,default_physical_servers)==0))
      return;
    frontier_mem_free(default_physical_servers);
   }
  if(physical_servers!=0)
    default_physical_servers=frontier_str_copy(physical_servers);
  else
    default_physical_servers=0;
 }

char *frontierConfig_getDefaultPhysicalServers()
 {
  if(default_physical_servers==0)
   {
    /* try the environment */
   frontierConfig_setDefaultPhysicalServers(getenv(FRONTIER_ENV_PHYSICALSERVERS));
   }
  return default_physical_servers;
 }

void frontierConfig_setSecured(FrontierConfig *cfg,int secured)
 {
  cfg->secured=secured;
 }

int frontierConfig_getSecured(FrontierConfig *cfg)
 {
  return cfg->secured;
 }

void frontierConfig_setCAPath(FrontierConfig *cfg,char *capath)
 {
  if(cfg->capath!=0)frontier_mem_free(cfg->capath);
  cfg->capath=frontier_str_copy(capath);
 }

char *frontierConfig_getCAPath(FrontierConfig *cfg)
 {
  return cfg->capath;
 }

void frontierConfig_setClientCacheMaxResultSize(FrontierConfig *cfg,int size)
 {
  cfg->client_cache_max_result_size=size;
 }

int frontierConfig_getClientCacheMaxResultSize(FrontierConfig *cfg)
 {
  return cfg->client_cache_max_result_size;
 }

void frontierConfig_setFailoverToServer(FrontierConfig *cfg,int notno)
 {
  cfg->failover_to_server=notno;
 }

int frontierConfig_getFailoverToServer(FrontierConfig *cfg)
 {
  return cfg->failover_to_server;
 }

static int frontierConfig_parseComplexServerSpec(FrontierConfig *cfg,const char* server_spec)
 {
  char *str=frontier_str_copy(server_spec);
  char *p=str-1;
  char *keyp=0,*valp=0;
  int nestlevel=0;
  int ret;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Parsing complex server spec %s",server_spec);

  if(str==NULL)
   {
    FRONTIER_MSG(FRONTIER_EMEM);
    return FRONTIER_EMEM;
   }

  if((keyp=strstr(server_spec,"(logicalserverurl="))!=NULL)
   {
    // save all but this keyword as the default physical servers for
    //  later connections
    char *nextp=strchr(keyp,')');
    if(nextp!=NULL)
     {
      char *cp=(char *)frontier_mem_alloc(strlen(server_spec));
      if(cp!=NULL)
       {
        strncpy(cp,server_spec,keyp-server_spec);
        strcpy(cp+(keyp-server_spec),nextp+1);
        frontierConfig_setDefaultPhysicalServers(cp);
        frontier_mem_free(cp);
       }
     }
   }

  while(*++p)
   {
    switch(*p)
     {
      case '(':
	if(nestlevel==0)
	 {
	  keyp=p+1;
	  valp=0;
	 }
        nestlevel++;
        break;
      case '=':
        if(nestlevel!=1)
	  continue;
	*p='\0';
	valp=p+1;
	break;
      case ')':
        --nestlevel;
        if(nestlevel<0)
	 {
	  frontier_setErrorMsg(__FILE__,__LINE__,"Too many close-parens in server spec");
	  ret=FRONTIER_ECFG;
	  goto cleanup;
	 }
	if(nestlevel>0)
	  continue;
	*p='\0';
	if(valp==0)
	 {
	  if(keyp==p)
	    /* empty parens */
	    continue;
	  frontier_setErrorMsg(__FILE__,__LINE__,"No '=' after keyword %s",keyp);
	  ret=FRONTIER_ECFG;
	  goto cleanup;
	 }
	ret=FRONTIER_OK;
	if(strcmp(keyp,"serverurl")==0)
	  ret=frontierConfig_addServer(cfg,valp);
	else if(strcmp(keyp,"proxyurl")==0)
	  ret=frontierConfig_addProxy(cfg,valp,0);
	else if(strcmp(keyp,"proxyconfigurl")==0)
	  ret=frontierConfig_addProxyConfig(cfg,valp);
	else if(strcmp(keyp,"backupproxyurl")==0)
	 {
	  /* implies failovertoserver=no */
	  frontierConfig_setFailoverToServer(cfg,0);
	  ret=frontierConfig_addProxy(cfg,valp,1);
	 }
	else if(strcmp(keyp,"logicalserverurl")==0)
	  frontierConfig_setDefaultLogicalServer(valp);
	else if(strcmp(keyp,"retrieve-ziplevel")==0)
	  frontierConfig_setRetrieveZipLevel(cfg,atoi(valp));
	else if(strcmp(keyp,"connecttimeoutsecs")==0)
	  frontierConfig_setConnectTimeoutSecs(cfg,atoi(valp));
	else if(strcmp(keyp,"readtimeoutsecs")==0)
	  frontierConfig_setReadTimeoutSecs(cfg,atoi(valp));
	else if(strcmp(keyp,"writetimeoutsecs")==0)
	  frontierConfig_setWriteTimeoutSecs(cfg,atoi(valp));
	else if(strcmp(keyp,"forcereload")==0)
	  frontierConfig_setForceReload(cfg,valp);
	else if(strcmp(keyp,"maxagesecs")==0)
	  frontierConfig_setMaxAgeSecs(cfg,atoi(valp));
	else if(strcmp(keyp,"preferipfamily")==0)
	  frontierConfig_setPreferIpFamily(cfg,atoi(valp));
	else if(strcmp(keyp,"freshkey")==0)
	  frontierConfig_setFreshkey(cfg,valp);
	else if(strcmp(keyp,"failovertoserver")==0)
	  frontierConfig_setFailoverToServer(cfg,(strcmp(valp,"no")!=0));
	else if(strcmp(keyp,"loadbalance")==0)
	 {
	  if(strcmp(valp,"proxies")==0)
	    frontierConfig_setBalancedProxies(cfg);
	  else if(strcmp(valp,"servers")==0)
	    frontierConfig_setBalancedServers(cfg);
	 }
	else if(strcmp(keyp,"security")==0)
	  frontierConfig_setSecured(cfg,(strcmp(valp,"sig")==0));
	else if(strcmp(keyp,"capath")==0)
	  frontierConfig_setCAPath(cfg,valp);
	else if(strcmp(keyp,"clientcachemaxresultsize")==0)
	  frontierConfig_setClientCacheMaxResultSize(cfg,atoi(valp));
	else
	 {
	 /* else ignore unrecognized keys */
	  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Unrecognized keyword %s",keyp);
	 }

	if(ret!=FRONTIER_OK)
	  goto cleanup;
	break;
     }
   }

  if(nestlevel>0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"Unmatched parentheses");
    ret=FRONTIER_ECFG;
    goto cleanup;
   }

  ret=FRONTIER_OK;

cleanup:
  frontier_mem_free(str);
  return ret;
 }

int frontierConfig_addServer(FrontierConfig *cfg,const char* server_url)
 {
  const char *logical_server;

  if((server_url!=0)&&(strchr(server_url,'(')!=NULL))
    return frontierConfig_parseComplexServerSpec(cfg,server_url);

  if(!server_url)
    return FRONTIER_OK;
  if(!*server_url)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Empty server url.");
    return FRONTIER_OK;
   }
  
  logical_server=frontierConfig_getDefaultLogicalServer();
  if((logical_server!=0)&&(strcmp(server_url,logical_server)==0))
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Matched logical server %s",server_url);
    /* replace it with the physical servers */
    frontierConfig_addServer(cfg,frontierConfig_getDefaultPhysicalServers());
    return FRONTIER_OK;
   }

  /* Ready to insert new server, make sure there's room */
  if(cfg->server_num>=FRONTIER_MAX_SERVERN)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
      "Reached limit of %d frontier servers",FRONTIER_MAX_SERVERN);
    return FRONTIER_ECFG;
   }

  /* Everything ok, insert new server. */
  cfg->server[cfg->server_num]=frontier_str_copy(server_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
        "Added server <%s>",cfg->server[cfg->server_num]);
  cfg->server_num++;
  return FRONTIER_OK;
 } 

int frontierConfig_addProxy(FrontierConfig *cfg,const char* proxy_url,int backup)
 {
  int n;

  if(!proxy_url)
    return FRONTIER_OK;
  if(!*proxy_url)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Empty proxy url.");    
    return FRONTIER_OK;
   }

  /* Ready to insert new proxy, make sure there's room */
  if(cfg->proxy_num>=FRONTIER_MAX_PROXYN)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
      "Reached limit of %d frontier proxies",FRONTIER_MAX_PROXYN);
    return FRONTIER_ECFG;
   }

  if(backup)
   {
    /* insert backup proxy at the end of the list */
    n=cfg->proxy_num;
    cfg->num_backupproxies++;
   }
  else
   {
    /* move any backup proxies up one to make room for this non-backup proxy */
    for(n=0;n<cfg->num_backupproxies;n++)
     {
      cfg->proxy[cfg->proxy_num-n]=cfg->proxy[cfg->proxy_num-n-1];
     }
     n=cfg->proxy_num-cfg->num_backupproxies;
   }

  /* Everything ok, insert new proxy. */
  cfg->proxy[n]=frontier_str_copy(proxy_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
        "Inserted proxy %d <%s>",n,cfg->proxy[n]);
  cfg->proxy_num++;
  return FRONTIER_OK;
 } 

int frontierConfig_addProxyConfig(FrontierConfig *cfg,const char* proxyconfig_url)
 {
  if(!proxyconfig_url)
    return FRONTIER_OK;
  if(!*proxyconfig_url)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Empty proxy config url.");    
    return FRONTIER_OK;
   }

  /* Ready to insert new proxy config, make sure there's room */
  if(cfg->proxyconfig_num>=FRONTIER_MAX_PROXYCONFIGN)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
      "Reached limit of %d frontier proxyconfig urls",FRONTIER_MAX_PROXYCONFIGN);
    return FRONTIER_ECFG;
   }

  /* Everything ok, insert new proxyconfig url. */
  if(strcmp(proxyconfig_url,"auto")==0)
    proxyconfig_url="http://wpad/wpad.dat";
  cfg->proxyconfig[cfg->proxyconfig_num]=frontier_str_copy(proxyconfig_url);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
        "Inserted proxy config %d <%s>",cfg->proxyconfig_num,
		cfg->proxyconfig[cfg->proxyconfig_num]);
  cfg->proxyconfig_num++;
  return FRONTIER_OK;
 } 

#define MAXPACSTRINGSIZE (16*4096)
#define MAXPPERRORBUFSIZE 1024
#define PPERRORMSGPREFIX " (pacparser message: "

struct pp_errcontext {
    char buf[MAXPPERRORBUFSIZE];
    int len;
};
static struct pp_errcontext *pp_errorcontext;

static int fn_pp_errorvprint(const char *fmt,va_list ap)
 {
  struct pp_errcontext *cx=pp_errorcontext;
  int start=sizeof(PPERRORMSGPREFIX)-1+cx->len;
  int left=MAXPPERRORBUFSIZE-start-2;
  int ret;
  char *p;
  
  ret=vsnprintf(&cx->buf[start],left,fmt,ap);

  // replace newlines with blank
  for(p=&cx->buf[start];*p;p++)
    if(*p=='\n')
      *p=' ';

  if(strncmp(fmt,"WARNING: ",9)==0)
   {
    frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,
    		&cx->buf[start+9]);
    return ret;
   }

  if(strncmp(fmt,"DEBUG: ",7)==0)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    		&cx->buf[start+7]);
    return ret;
   }

  if(ret>=left)
    cx->len+=left-1;
  else
    cx->len+=ret;

  return(ret);
 }

static char *fn_pp_getErrorMsg()
 {
  struct pp_errcontext *cx=pp_errorcontext;
  if(cx->len==0)
    return "";
  memcpy(&cx->buf[0],PPERRORMSGPREFIX,sizeof(PPERRORMSGPREFIX)-1);
  cx->buf[cx->len+sizeof(PPERRORMSGPREFIX)-1]=')';
  cx->buf[cx->len+sizeof(PPERRORMSGPREFIX)]='\0';
  return(&cx->buf[0]);
 }

int frontierConfig_doProxyConfig(FrontierConfig *cfg)
 {
  FrontierHttpClnt *clnt;
  FrontierUrlInfo *fui=0;
  int curproxyconfig,nextproxyconfig;
  char *proxyconfig_url;
  int n,nbytes,ret=FRONTIER_OK;
  char err_last_buf[MAXPPERRORBUFSIZE];
  struct pp_errcontext err_context;
  char *pacstring=0;
  char *ipaddr,*proxylist;
  char *p,*endp,endc;
  int gotdirect=0;
  int trynextloglevel;

  if(cfg->proxyconfig_num<=0)
    return FRONTIER_OK;

  clnt=frontierHttpClnt_create(&ret);
  if(ret!=FRONTIER_OK)
    return ret;
  if(!clnt)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
      "error creating http client object",0);
    return FRONTIER_ECFG;
   }
  frontierHttpClnt_setPreferIpFamily(clnt,
  		frontierConfig_getPreferIpFamily(cfg));
  frontierHttpClnt_setConnectTimeoutSecs(clnt,
  		frontierConfig_getConnectTimeoutSecs(cfg));
  frontierHttpClnt_setReadTimeoutSecs(clnt,
  		frontierConfig_getReadTimeoutSecs(cfg));
  frontierHttpClnt_setWriteTimeoutSecs(clnt,
  		frontierConfig_getWriteTimeoutSecs(cfg));
  frontierHttpClnt_setFrontierId(clnt,FNAPI_VERSION);

  for(curproxyconfig=0;curproxyconfig<cfg->proxyconfig_num;curproxyconfig++)
   {
    proxyconfig_url=cfg->proxyconfig[curproxyconfig];

    if(strncmp(proxyconfig_url,"file://",7)==0)
     {
      // don't consider any proxyconfig URLs after this one
      break;
     }

    if(strncmp(proxyconfig_url,"http://",7)!=0)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,
	"proxyconfigurl %s does not begin with 'http://', 'file://', or 'auto'",proxyconfig_url);
      ret=FRONTIER_ECFG;
      goto cleanup;
     }

    // check for something beyond a server name because otherwise 
    //  frontierHttpClnt_addServer will give wrong message about missing servlet
    if(strchr(proxyconfig_url+7,'/')==0)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,
	"config error: proxyconfigurl %s missing path on server",proxyconfig_url);
      ret=FRONTIER_ECFG;
      goto cleanup;
     }

    ret=frontierHttpClnt_addServer(clnt,proxyconfig_url);
    if(ret!=FRONTIER_OK) goto cleanup;
   }

  pacstring=frontier_mem_alloc(MAXPACSTRINGSIZE+1);
  if(!pacstring)
   {
    ret=FRONTIER_EMEM;
    goto cleanup;
   }

  // Messages below have either the http client servername or the current
  //  proxyconfig_url.  In general, warning messages should have the
  //  servername because that includes an IP address, and warning messages
  //  here are about server errors.  Error messages should show the
  //  proxyconfig_url because that's more relevant to the user.

  curproxyconfig=0;
  while(1)
   {
    proxyconfig_url=cfg->proxyconfig[curproxyconfig];

    if(strncmp(proxyconfig_url,"file://",7)==0)
     {
      char *fname;
      int fd;
      fname=strchr(proxyconfig_url+7,'/');
      if(fname==NULL)
       {
        frontier_setErrorMsg(__FILE__,__LINE__,
	     "bad format for file url %s, no path", proxyconfig_url);
	ret=FRONTIER_ECFG;
	goto cleanup;
       }
      fd=open(fname,O_RDONLY);
      if(fd<0)
       {
        frontier_setErrorMsg(__FILE__,__LINE__,
	     "error opening %s: %s",fname,strerror(errno));
	ret=FRONTIER_ECFG;
	goto cleanup;
       }
      nbytes=read(fd,pacstring,MAXPACSTRINGSIZE);
      if(nbytes<0)
       {
        frontier_setErrorMsg(__FILE__,__LINE__,
	     "error reading %s: %s",fname,strerror(errno));
	ret=FRONTIER_ECFG;
	close(fd);
	goto cleanup;
       }
      close(fd);
      if(nbytes==MAXPACSTRINGSIZE)
       {
	frontier_setErrorMsg(__FILE__,__LINE__,
	   "config error: proxyconfig file %s larger than limit of %d bytes",
	   	fname,MAXPACSTRINGSIZE);
	ret=FRONTIER_ECFG;
	goto cleanup;
       }
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
	"proxyconfig file from %s is %d bytes long",fname,nbytes);
      break;
     }

    frontier_turnErrorsIntoDebugs(1);
    trynextloglevel=FRONTIER_LOGLEVEL_WARNING;

    ret=frontierHttpClnt_open(clnt);
    if(ret!=FRONTIER_OK)
     {
      if (strstr(frontier_getErrorMsg(),"Name or service not known")!=0)
        trynextloglevel=FRONTIER_LOGLEVEL_DEBUG;
      frontier_log(trynextloglevel,__FILE__,__LINE__,
	 "unable to connect to proxyconfig server %s: %s",
		frontierHttpClnt_curservername(clnt), frontier_getErrorMsg());
      goto trynext;
     }

    ret=frontierHttpClnt_get(clnt,"");
    if(ret!=FRONTIER_OK)
     {
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,
	 "unable to get proxyconfig from %s: %s",
	 	frontierHttpClnt_curservername(clnt), frontier_getErrorMsg());
      goto trynext;
     }

    nbytes=0;
    while((n=frontierHttpClnt_read(clnt,pacstring+nbytes,MAXPACSTRINGSIZE-nbytes))>0)
     {
      nbytes+=n;
      if(nbytes==MAXPACSTRINGSIZE)
       {
	frontier_turnErrorsIntoDebugs(0);
	frontier_setErrorMsg(__FILE__,__LINE__,
	   "config error: downloaded proxyconfig file from %s larger than limit of %d bytes",
	   	proxyconfig_url,MAXPACSTRINGSIZE);
	ret=FRONTIER_ECFG;
	goto cleanup;
       }
    }
    if(n<0)
     {
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,
	 "problem reading proxyconfig from %s: %s",
	    frontierHttpClnt_curservername(clnt), frontier_getErrorMsg());
      goto trynext;
     }
    pacstring[nbytes]='\0';

    frontier_turnErrorsIntoDebugs(0);

    // successfully read the proxy autoconfig file
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
      "proxyconfig file from %s is %d bytes long",
	 	frontierHttpClnt_curservername(clnt),nbytes);
    break;

trynext:
    strncpy(err_last_buf,frontier_getErrorMsg(),sizeof(err_last_buf)-1);
    err_last_buf[sizeof(err_last_buf)-1]='\0';
    
    frontierHttpClnt_close(clnt);
    frontier_turnErrorsIntoDebugs(0);
   
    if((nextproxyconfig=frontierHttpClnt_nextserver(clnt,1))<0)
     {
      curproxyconfig++;
      if ((curproxyconfig<cfg->proxyconfig_num)&&
	    (strncmp(cfg->proxyconfig[curproxyconfig],"file://",7)==0))
       {
        // there is a file:// URL still to try
        frontier_log(trynextloglevel,__FILE__,__LINE__,
          "Trying next proxyconfig url %s", cfg->proxyconfig[curproxyconfig]);
       }
      else
       {
	frontier_setErrorMsg(__FILE__,__LINE__,
	  "config error: failed to read a proxyconfig url.  Last url was %s and last error was: %s",
		  proxyconfig_url,err_last_buf);
	goto cleanup;
       }
     }
    else
     {
      curproxyconfig=nextproxyconfig;
      frontier_log(trynextloglevel,__FILE__,__LINE__,
        "Trying next proxyconfig server %s",
	 	frontierHttpClnt_curservername(clnt));
     }
   }

  ret=frontier_pacparser_init();
  if(ret!=FRONTIER_OK)
    goto cleanup;

  err_context.len=0;
  err_context.buf[0]='\0';
  pp_errorcontext=&err_context;
  pacparser_set_error_printer(&fn_pp_errorvprint);

  if(!pacparser_init())
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
       "config error: cannot initialize pacparser%s",fn_pp_getErrorMsg());
    ret=FRONTIER_ECFG;
    goto cleanup;
   }

  if(strncmp(proxyconfig_url,"file://",7)!=0)
   {
    if((ipaddr=frontierHttpClnt_myipaddr(clnt))==NULL)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,
	 "config error: error determining my IP address for proxyconfig server%s",
		  frontierHttpClnt_curservername(clnt),fn_pp_getErrorMsg());
      ret=FRONTIER_ECFG;
      goto cleanup;
     }

    pacparser_setmyip(ipaddr);
   }

  if(!pacparser_parse_pac_string(pacstring))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
       "config error: failure parsing %d byte proxyconfig from %s%s",
       		nbytes,proxyconfig_url,fn_pp_getErrorMsg());
    ret=FRONTIER_ECFG;
    goto cleanup;
   }

  if(cfg->server_num<1)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
       "config error: cannot process proxyconfigurl without a serverurl");
    ret=FRONTIER_ECFG;
    goto cleanup;
   }

  // need to parse the host out of the server URL, and in addition to
  //   other things, frontier_CreateUrlInfo happens to do that
  fui=frontier_CreateUrlInfo(cfg->server[0],&ret);
  if(!fui)goto cleanup;
  proxylist=pacparser_find_proxy(cfg->server[0],fui->host);

  if(proxylist==NULL)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
       "config error: proxyconfigurl %s FindProxyForURL(\"%s\",\"%s\") returned no match%s",
	    proxyconfig_url,cfg->server[0],fui->host,fn_pp_getErrorMsg());
    ret=FRONTIER_ECFG;
    goto cleanup;
   }

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    "FindProxyForURL(\"%s\",\"%s\") returned \"%s\"",cfg->server[0],fui->host,proxylist);

  if(getNumNonBackupProxies(cfg)>0)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
      "Turning %d proxies into backup proxies so they will be after proxyconfig proxies",getNumNonBackupProxies(cfg));
    cfg->num_backupproxies=cfg->proxy_num;
   }

  if(cfg->proxies_balanced)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
      "Disabling loadbalance=proxies because of proxyconfigurl");
    cfg->proxies_balanced=0;
   }

  // Now parse the returned proxy list
  p=proxylist;
  endc=*p;
  while(endc!='\0')
   {
    while(*p==' ')
      p++;
    for(endp=p;*endp&&(*endp!=';')&&(*endp!=' ');endp++)
      ;
    endc=*endp;
    *endp='\0';
    if(strcmp(p,"PROXY")==0)
     {
      if((endc=='\0')||(endc==';'))
        p=endp;
      else
       {
	p=endp+1;
	while(*p==' ')
	  p++;
	for(endp=p;*endp&&(*endp!=';')&&(*endp!=' ');endp++)
	  ;
	endc=*endp;
	*endp='\0';
       }
      if(*p)
        frontierConfig_addProxy(cfg,p,0);
      else
       {
	frontier_setErrorMsg(__FILE__,__LINE__,
          "config error: proxyconfigurl %s FindProxyForURL(\"%s\",\"%s\") returned \"PROXY\" without a proxy",
	  	proxyconfig_url,cfg->server[0],fui->host);
	ret=FRONTIER_ECFG;
	goto cleanup;
       }
     }
    else if(strcmp(p,"DIRECT")==0)
     {
      gotdirect=1;
      break;
     }
    else if(*p)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,
        "config error: proxyconfigurl %s FindProxyForURL(\"%s\",\"%s\") returned unrecognized type %s, expect PROXY or DIRECT",
		proxyconfig_url,cfg->server[0],fui->host,p);
      ret=FRONTIER_ECFG;
      goto cleanup;
     }
    p=endp+1;
   }

  if(!gotdirect)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
      "no DIRECT found in proxyconfig list, setting failovertoserver=no");
    frontierConfig_setFailoverToServer(cfg,0);
   }

  ret=FRONTIER_OK;

cleanup:
  if(pacstring) frontier_mem_free(pacstring);
  pacparser_cleanup();
  frontierHttpClnt_close(clnt);
  frontierHttpClnt_delete(clnt);
  if(fui) frontier_DeleteUrlInfo(fui);
  return ret;
 }

void frontierConfig_setBalancedProxies(FrontierConfig *cfg)
 {
  cfg->proxies_balanced=1;
 }

int frontierConfig_getNumBalancedProxies(FrontierConfig *cfg)
 {
  if(!cfg->proxies_balanced)
    return 0;
  return getNumNonBackupProxies(cfg);
 }

void frontierConfig_setBalancedServers(FrontierConfig *cfg)
 {
  cfg->servers_balanced=1;
 }

int frontierConfig_getBalancedServers(FrontierConfig *cfg)
 {
  return cfg->servers_balanced;
 }

