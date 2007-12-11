/*
 * frontier client config header
 * 
 * $Id$
 *
 *  Copyright (C) 2007  Fermilab
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program.  If not, see
 *  <http://www.gnu.org/licenses/>.
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
#define FRONTIER_ENV_CONNECTTIMEOUTSECS	"FRONTIER_CONNECTTIMEOUTSECS"
#define FRONTIER_ENV_READTIMEOUTSECS	"FRONTIER_READTIMEOUTSECS"
#define FRONTIER_ENV_WRITETIMEOUTSECS	"FRONTIER_WRITETIMEOUTSECS"
#define FRONTIER_ENV_FORCERELOAD	"FRONTIER_FORCERELOAD"

struct s_FrontierConfig
 {
  char *server[FRONTIER_MAX_SERVERN];
  char *proxy[FRONTIER_MAX_PROXYN];
  int server_num;
  int proxy_num;
  int server_cur;
  int proxy_cur;
  int servers_balanced;
  int proxies_balanced;
  int connect_timeout_secs;
  int read_timeout_secs;
  int write_timeout_secs;
  char *force_reload;
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
void frontierConfig_setBalancedProxies(FrontierConfig *cfg);
int frontierConfig_getBalancedProxies(FrontierConfig *cfg);
void frontierConfig_setBalancedServers(FrontierConfig *cfg);
int frontierConfig_getBalancedServers(FrontierConfig *cfg);
void frontierConfig_setConnectTimeoutSecs(FrontierConfig *cfg,int level);
int frontierConfig_getConnectTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setReadTimeoutSecs(FrontierConfig *cfg,int level);
int frontierConfig_getReadTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setForceReload(FrontierConfig *cfg,char *forcereload);
const char *frontierConfig_getForceReload(FrontierConfig *cfg);
void frontierConfig_setWriteTimeoutSecs(FrontierConfig *cfg,int level);
int frontierConfig_getWriteTimeoutSecs(FrontierConfig *cfg);
void frontierConfig_setRetrieveZipLevel(FrontierConfig *cfg,int level);
int frontierConfig_getRetrieveZipLevel(FrontierConfig *cfg);
void frontierConfig_setDefaultRetrieveZipLevel(int level);
int frontierConfig_getDefaultRetrieveZipLevel();
void frontierConfig_setDefaultLogicalServer(const char *logical_server);
char *frontierConfig_getDefaultLogicalServer();
void frontierConfig_setDefaultPhysicalServers(const char *physical_servers);
char *frontierConfig_getDefaultPhysicalServers();

#endif /* FRONTIER_CONFIG_H */


