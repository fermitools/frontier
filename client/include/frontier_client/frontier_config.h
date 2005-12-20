/*
 * Frontier config.
 * 
 * $Id$
 *
 */

#ifndef FRONTIER_CONFIG_H
#define FRONTIER_CONFIG_H

#define FRONTIER_MAX_SERVERN	4	// Max number of servers in FRONTIER_SERVER env. variable
#define FRONTIER_MAX_PROXYN	4	// Max number of proxies in FRONTIER_PROXY env. variable

#define FRONTIER_ENV_SERVER	"FRONTIER_SERVER"
#define FRONTIER_ENV_PROXY	"FRONTIER_PROXY"

struct s_FrontierConfig
 {
  char *server[FRONTIER_MAX_SERVERN];
  char *proxy[FRONTIER_MAX_PROXYN];
  int server_num;
  int proxy_num;
  int server_cur;
  int proxy_cur;
 };
typedef struct s_FrontierConfig FrontierConfig;
FrontierConfig *frontierConfig_get(const char *server_url,const char *proxy_url);
const char *frontierConfig_getServerUrl(FrontierConfig *cfg);
const char *frontierConfig_getProxyUrl(FrontierConfig *cfg);
int frontierConfig_nextServer(FrontierConfig *cfg);
int frontierConfig_nextProxy(FrontierConfig *cfg);
void frontierConfig_delete(FrontierConfig *cfg);
int frontierConfig_addServer(FrontierConfig *cfg, const char* server_url);
int frontierConfig_addProxy(FrontierConfig *cfg, const char* proxy_url);

#endif /* FRONTIER_CONFIG_H */


