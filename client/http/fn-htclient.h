/*
 * frontier client http interface header
 * 
 * Author: Sergey Kosyakov
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

#ifndef __HEADER_HTTP_H_FN_HTCLIENT_H
#define __HEADER_HTTP_H_FN_HTCLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "frontier_client/frontier.h"
#include "frontier_client/frontier_error.h"

int frontier_socket();
void frontier_socket_close(int s);
int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen,int timeoutsecs);
int frontier_write(int s,const char *buf,int len,int timeoutsecs);
int frontier_read(int s, char *buf,int size,int timeoutsecs);


struct s_FrontierUrlInfo
 {
  char *url;
  char *proto;
  char *host;
  int port;
  int haderror;
  char *path;
  struct addrinfo *addr;
  struct addrinfo *nextaddr;
 };
typedef struct s_FrontierUrlInfo FrontierUrlInfo;

FrontierUrlInfo *frontier_CreateUrlInfo(const char *url,int *ec);
int frontier_resolv_host(FrontierUrlInfo *fui);
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui);

#define FRONTIER_HTTP_BUF_SIZE	(32*1024)
#define FRONTIER_MAX_PERSIST_SIZE (16*1024)

struct s_FrontierHttpClnt
 {
  FrontierUrlInfo *proxy[FRONTIER_MAX_PROXYN];
  FrontierUrlInfo *server[FRONTIER_MAX_SERVERN];
  int cur_proxy;
  int cur_server;
  int total_proxy;
  int total_server;
  int balance_proxies;
  int balance_servers;
  int first_proxy;
  int first_server;
  unsigned rand_seed;
  int is_refresh;
  int using_proxy;
  int content_length;
  int total_length;
  char *url_suffix;
  char *frontier_id;
  
  int socket;
  int connect_timeout_secs;
  int read_timeout_secs;
  int write_timeout_secs;
    
  int err_code;
  int data_size;
  int data_pos;
  char buf[FRONTIER_HTTP_BUF_SIZE];
 }; 
typedef struct s_FrontierHttpClnt FrontierHttpClnt;


FrontierHttpClnt *frontierHttpClnt_create(int *ec);
int frontierHttpClnt_addServer(FrontierHttpClnt *c,const char *url);
int frontierHttpClnt_addProxy(FrontierHttpClnt *c,const char *url);
void frontierHttpClnt_setCacheRefreshFlag(FrontierHttpClnt *c,int is_refresh);
void frontierHttpClnt_setUrlSuffix(FrontierHttpClnt *c,char *suffix);
void frontierHttpClnt_setFrontierId(FrontierHttpClnt *c,const char *frontier_id);
void frontierHttpClnt_setConnectTimeoutSecs(FrontierHttpClnt *c,int timeoutsecs);
void frontierHttpClnt_setReadTimeoutSecs(FrontierHttpClnt *c,int timeoutsecs);
void frontierHttpClnt_setWriteTimeoutSecs(FrontierHttpClnt *c,int timeoutsecs);
int frontierHttpClnt_open(FrontierHttpClnt *c);
int frontierHttpClnt_get(FrontierHttpClnt *c,const char *url);
int frontierHttpClnt_post(FrontierHttpClnt *c,const char *url,const char *body);
int frontierHttpClnt_read(FrontierHttpClnt *c,char *buf,int buf_len);
void frontierHttpClnt_close(FrontierHttpClnt *c);
void frontierHttpClnt_delete(FrontierHttpClnt *c);
int frontierHttpClnt_resetproxylist(FrontierHttpClnt *c,int shuffle);
int frontierHttpClnt_resetserverlist(FrontierHttpClnt *c,int shuffle);
int frontierHttpClnt_nextproxy(FrontierHttpClnt *c,int curhaderror);
int frontierHttpClnt_nextserver(FrontierHttpClnt *c,int curhaderror);
char *frontierHttpClnt_curproxyname(FrontierHttpClnt *c);
char *frontierHttpClnt_curservername(FrontierHttpClnt *c);
void frontierHttpClnt_setBalancedProxies(FrontierHttpClnt *c);
void frontierHttpClnt_setBalancedServers(FrontierHttpClnt *c);


#endif //__HEADER_HTTP_H_FN_HTCLIENT_H
