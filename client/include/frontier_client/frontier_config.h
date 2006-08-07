/*
 * Frontier config.
 * 
 * $Id$
 *
 */

#ifndef FRONTIER_CONFIG_H
#define FRONTIER_CONFIG_H

#define FRONTIER_MAX_SERVERN	6	// Max number of servers in FRONTIER_SERVER env. variable
#define FRONTIER_MAX_PROXYN	4	// Max number of proxies in FRONTIER_PROXY env. variable

#define FRONTIER_ENV_LOGICALSERVER	"FRONTIER_LOGICALSERVER"
#define FRONTIER_ENV_PHYSICALSERVERS	"FRONTIER_PHYSICALSERVERS"
#define FRONTIER_ENV_SERVER		"FRONTIER_SERVER"
#define FRONTIER_ENV_PROXY		"FRONTIER_PROXY"
#define FRONTIER_ENV_RETRIEVEZIPLEVEL	"FRONTIER_RETRIEVEZIPLEVEL"

struct s_FrontierConfig
 {
  char *server[FRONTIER_MAX_SERVERN];
  char *proxy[FRONTIER_MAX_PROXYN];
  int server_num;
  int proxy_num;
  int server_cur;
  int proxy_cur;
  int retrieve_zip_level;
 };
typedef struct s_FrontierConfig FrontierConfig;
FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url,int *errorCode);
const char *frontierConfig_getServerUrl(FrontierConfig *cfg);
const char *frontierConfig_getProxyUrl(FrontierConfig *cfg);
int frontierConfig_nextServer(FrontierConfig *cfg);
int frontierConfig_nextProxy(FrontierConfig *cfg);
void frontierConfig_delete(FrontierConfig *cfg);
int frontierConfig_addServer(FrontierConfig *cfg, const char* server_url);
int frontierConfig_addProxy(FrontierConfig *cfg, const char* proxy_url);
int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg);
void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level);
void frontierConfig_setDefaultRetrieveZipLevel(int level);
int frontierConfig_getDefaultRetrieveZipLevel();
void frontierConfig_setDefaultLogicalServer(const char *logical_server);
char *frontierConfig_getDefaultLogicalServer();
void frontierConfig_setDefaultPhysicalServers(const char *physical_servers);
char *frontierConfig_getDefaultPhysicalServers();

#endif /* FRONTIER_CONFIG_H */


