/*
 * frontier client config header
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

#ifndef FRONTIER_CONFIG_H
#define FRONTIER_CONFIG_H

#define FRONTIER_MAX_SERVERN	16	// Max number of servers in FRONTIER_SERVER env. variable
#define FRONTIER_MAX_PROXYN	24	// Max number of proxies in FRONTIER_PROXY env. variable
#define FRONTIER_MAX_PROXYCONFIGN 8	// Max number of proxyconfigurl

#define FRONTIER_MAX_EPROTOAGE (5*60)   // Max number of seconds to cache
					//    protocol errors
#define FRONTIER_DEFAULT_CLIENTCACHEMAXRESULTSIZE 0

#define FRONTIER_ENV_LOGICALSERVER	"FRONTIER_LOGICALSERVER"
#define FRONTIER_ENV_PHYSICALSERVERS	"FRONTIER_PHYSICALSERVERS"
#define FRONTIER_ENV_SERVER		"FRONTIER_SERVER"
#define FRONTIER_ENV_PROXY		"FRONTIER_PROXY"
#define FRONTIER_ENV_PROXYCONFIGS	"FRONTIER_PROXYCONFIGS"
#define FRONTIER_ENV_RETRIEVEZIPLEVEL	"FRONTIER_RETRIEVEZIPLEVEL"
#define FRONTIER_ENV_CONNECTTIMEOUTSECS	"FRONTIER_CONNECTTIMEOUTSECS"
#define FRONTIER_ENV_READTIMEOUTSECS	"FRONTIER_READTIMEOUTSECS"
#define FRONTIER_ENV_WRITETIMEOUTSECS	"FRONTIER_WRITETIMEOUTSECS"
#define FRONTIER_ENV_FORCERELOAD	"FRONTIER_FORCERELOAD"
#define FRONTIER_ENV_FRESHKEY		"FRONTIER_FRESHKEY"

struct s_FrontierConfig
 {
  char *server[FRONTIER_MAX_SERVERN];
  char *proxy[FRONTIER_MAX_PROXYN];
  char *proxyconfig[FRONTIER_MAX_PROXYCONFIGN];
  int server_num;
  int proxy_num;
  int proxyconfig_num;
  int server_cur;
  int proxy_cur;
  int proxyconfig_cur;
  int servers_balanced;
  int proxies_balanced;
  int num_backupproxies;
  int connect_timeout_secs;
  int read_timeout_secs;
  int write_timeout_secs;
  int max_age_secs;
  char *force_reload;
  char *freshkey;
  int retrieve_zip_level;
  int secured;
  char *capath;
  int client_cache_max_result_size;
  int failover_to_server;
  int prefer_ip_family;
 };
typedef struct s_FrontierConfig FrontierConfig;
FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url,int *errorCode);
const char *frontierConfig_getServerUrl(FrontierConfig *cfg);
const char *frontierConfig_getProxyUrl(FrontierConfig *cfg);
int frontierConfig_nextServer(FrontierConfig *cfg);
int frontierConfig_nextProxy(FrontierConfig *cfg);
void frontierConfig_delete(FrontierConfig *cfg);
int frontierConfig_addServer(FrontierConfig *cfg, const char* server_url);
int frontierConfig_addProxy(FrontierConfig *cfg, const char* proxy_url, int backup);
int frontierConfig_addProxyConfig(FrontierConfig *cfg, const char* proxyconfig_url);
int frontierConfig_doProxyConfig(FrontierConfig *cfg);
void frontierConfig_setBalancedProxies(FrontierConfig *cfg);
int frontierConfig_getNumBalancedProxies(FrontierConfig *cfg);
void frontierConfig_setBalancedServers(FrontierConfig *cfg);
int frontierConfig_getBalancedServers(FrontierConfig *cfg);
void frontierConfig_setConnectTimeoutSecs(FrontierConfig *cfg,int secs);
int frontierConfig_getConnectTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setReadTimeoutSecs(FrontierConfig *cfg,int secs);
int frontierConfig_getReadTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setWriteTimeoutSecs(FrontierConfig *cfg,int secs);
int frontierConfig_getWriteTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setMaxAgeSecs(FrontierConfig *cfg,int secs);
int frontierConfig_getMaxAgeSecs(FrontierConfig *cfg);
void frontierConfig_setPreferIpFamily(FrontierConfig *cfg,int ipfamily);
int frontierConfig_getPreferIpFamily(FrontierConfig *cfg);
void frontierConfig_setForceReload(FrontierConfig *cfg,char *forcereload);
const char *frontierConfig_getForceReload(FrontierConfig *cfg);
void frontierConfig_setFreshkey(FrontierConfig *cfg,char *freshkey);
const char *frontierConfig_getFreshkey(FrontierConfig *cfg);
void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level);
int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg);
void frontierConfig_setDefaultRetrieveZipLevel(int level);
int frontierConfig_getDefaultRetrieveZipLevel();
void frontierConfig_setDefaultLogicalServer(const char *logical_server);
char *frontierConfig_getDefaultLogicalServer();
void frontierConfig_setDefaultPhysicalServers(const char *physical_servers);
char *frontierConfig_getDefaultPhysicalServers();
void frontierConfig_setSecured(FrontierConfig *cfg,int secured);
int frontierConfig_getSecured(FrontierConfig *cfg);
void frontierConfig_setCAPath(FrontierConfig *cfg,char *capath);
char *frontierConfig_getCAPath(FrontierConfig *cfg);
void frontierConfig_setClientCacheMaxResultSize(FrontierConfig *cfg,int size);
int frontierConfig_getClientCacheMaxResultSize(FrontierConfig *cfg);
void frontierConfig_setFailoverToServer(FrontierConfig *cfg,int notno);
int frontierConfig_getFailoverToServer(FrontierConfig *cfg);

#endif /* FRONTIER_CONFIG_H */


