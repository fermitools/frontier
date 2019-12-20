/*
 * frontier client http interface header
 * 
 * Author: Sergey Kosyakov
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

#ifndef __HEADER_HTTP_H_FN_HTCLIENT_H
#define __HEADER_HTTP_H_FN_HTCLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include "frontier_client/frontier.h"
#include "frontier_client/frontier_error.h"

char *frontier_ipaddr(const struct sockaddr *serv_addr);
int frontier_socket(sa_family_t family);
void frontier_socket_close(int s);
int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen,int timeoutsecs);
int frontier_write(int s,const char *buf,int len,int timeoutsecs,struct addrinfo *addr);
int frontier_read(int s, char *buf,int size,int timeoutsecs,struct addrinfo *addr);


struct s_FrontierAddrInfo
 {
  struct s_FrontierAddrInfo *next;
  struct addrinfo *ai;
  int haderror;
 };
typedef struct s_FrontierAddrInfo FrontierAddrInfo;

struct s_FrontierUrlInfo
 {
  char *url;
  char *proto;
  char *host;
  int port;
  char *path;
  struct addrinfo *ai;
  FrontierAddrInfo firstfai;
  FrontierAddrInfo *firstfaiinfamily; // first in the current ip family
  FrontierAddrInfo *fai;
  FrontierAddrInfo *lastfai;
 };
typedef struct s_FrontierUrlInfo FrontierUrlInfo;

FrontierUrlInfo *frontier_CreateUrlInfo(const char *url,int *ec);
int frontier_resolv_host(FrontierUrlInfo *fui,int preferipfamily,unsigned *rand_seedp);
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui);
void frontier_FreeAddrInfo(FrontierUrlInfo *fui);

#define FRONTIER_HTTP_DEBUG_BUF_SIZE 512

struct s_FrontierHostsInfo
 {
  FrontierUrlInfo **hosts;
  int cur;
  int total;
  int num_balanced;
  int first;
  time_t whenreset;
  char debugbuf[FRONTIER_HTTP_DEBUG_BUF_SIZE];
  unsigned rand_seed;
 };
typedef struct s_FrontierHostsInfo FrontierHostsInfo;

#define FRONTIER_RESETPROXY_SECS (60*5)
#define FRONTIER_RESETSERVER_SECS (60*30)
#define FRONTIER_HTTP_BUF_SIZE	(32*1024)
#define FRONTIER_MAX_PERSIST_SIZE (16*1024)

struct s_FrontierHttpClnt
 {
  FrontierUrlInfo *proxy[FRONTIER_MAX_PROXYN];
  FrontierUrlInfo *server[FRONTIER_MAX_SERVERN];
  FrontierHostsInfo proxyi;
  FrontierHostsInfo serveri;

  int refresh_flag;
  int max_age;
  int age;
  int prefer_ip_family;
  int using_proxy;
  int content_length;
  int total_length;
  char *url_suffix;
  char *frontier_id;
  
  int socket;
  int connect_timeout_secs;
  int read_timeout_secs;
  int write_timeout_secs;
  struct addrinfo *cur_ai;
    
  int err_code;
  int data_size;
  int data_pos;
  char buf[FRONTIER_HTTP_BUF_SIZE];
 }; 
typedef struct s_FrontierHttpClnt FrontierHttpClnt;


FrontierHttpClnt *frontierHttpClnt_create(int *ec);
int frontierHttpClnt_addServer(FrontierHttpClnt *c,const char *url);
int frontierHttpClnt_addProxy(FrontierHttpClnt *c,const char *url);
void frontierHttpClnt_setCacheRefreshFlag(FrontierHttpClnt *c,int refresh_flag);
void frontierHttpClnt_setCacheMaxAgeSecs(FrontierHttpClnt *c,int secs);
void frontierHttpClnt_setPreferIpFamily(FrontierHttpClnt *c,int ipfamily);
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
void frontierHttpClnt_drop(FrontierHttpClnt *c);
void frontierHttpClnt_delete(FrontierHttpClnt *c);
void frontierHttpClnt_clear(FrontierHttpClnt *c);
void frontierHttpClnt_resetwhenold(FrontierHttpClnt *c);
int frontierHttpClnt_getContentLength(FrontierHttpClnt *c);
int frontierHttpClnt_getCacheAgeSecs(FrontierHttpClnt *c);
int frontierHttpClnt_shuffleproxygroup(FrontierHttpClnt *c);
int frontierHttpClnt_shuffleservergroup(FrontierHttpClnt *c);
int frontierHttpClnt_resetproxygroup(FrontierHttpClnt *c);
int frontierHttpClnt_resetserverlist(FrontierHttpClnt *c);
int frontierHttpClnt_nextproxy(FrontierHttpClnt *c,int curhaderror);
int frontierHttpClnt_nextserver(FrontierHttpClnt *c,int curhaderror);
int frontierHttpClnt_usinglastproxyingroup(FrontierHttpClnt *c);
char *frontierHttpClnt_curproxyname(FrontierHttpClnt *c);
char *frontierHttpClnt_curservername(FrontierHttpClnt *c);
char *frontierHttpClnt_curserverpath(FrontierHttpClnt *c);
char *frontierHttpClnt_myipaddr(FrontierHttpClnt *c);
void frontierHttpClnt_setNumBalancedProxies(FrontierHttpClnt *c,int num);
void frontierHttpClnt_setBalancedServers(FrontierHttpClnt *c);


#endif //__HEADER_HTTP_H_FN_HTCLIENT_H
