/*
 * FroNTier client API
 * 
 * Author: Sergey Kosyakov
 *
 * $Header$
 *
 * $Id$
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
int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen);
int frontier_write(int s,const char *buf, int len);
int frontier_read(int s, char *buf, int size);


struct s_FrontierUrlInfo
 {
  char *url;
  char *proto;
  char *host;
  int port;
  char *path;
  struct addrinfo *addr;
 };
typedef struct s_FrontierUrlInfo FrontierUrlInfo;

FrontierUrlInfo *frontier_CreateUrlInfo(const char *url,int *ec);
int frontier_resolv_host(FrontierUrlInfo *fui);
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui);

#define FRONTIER_HTTP_BUF_SIZE	(32*1024)

struct s_FrontierHttpClnt
 {
  FrontierUrlInfo *proxy[FRONTIER_MAX_PROXYN];
  FrontierUrlInfo *server[FRONTIER_MAX_SERVERN];
  int cur_proxy;
  int cur_server;
  int total_proxy;
  int total_server;
  int is_refresh;
  char *frontier_id;
  
  int socket;
    
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
void frontierHttpClnt_setFrontierId(FrontierHttpClnt *c,const char *frontier_id);
int frontierHttpClnt_open(FrontierHttpClnt *c,const char *url);
int frontierHttpClnt_post(FrontierHttpClnt *c,const char *url,const char *body);
int frontierHttpClnt_read(FrontierHttpClnt *c,char *buf,int buf_len);
void frontierHttpClnt_close(FrontierHttpClnt *c);
void frontierHttpClnt_delete(FrontierHttpClnt *c);


#endif //__HEADER_HTTP_H_FN_HTCLIENT_H
