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

#include <fn-htclient.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PARSE_BUF_SIZE	256



FrontierUrlInfo *frontier_CreateUrlInfo(const char *url)
 {
  FrontierUrlInfo *fui;
  const char *p;
  char *tmp;
  int i;
  char buf[PARSE_BUF_SIZE];
  
  fui=malloc(sizeof(FrontierUrlInfo));
  if(!fui) return fui;
  bzero(fui,sizeof(FrontierUrlInfo));
  fui->url=strdup(url);
  
  if(strncmp(url,"http://",7))
   {
    fui->err_msg=strdup("Unknown protocol");
    return fui;
   }
  fui->proto=strdup("http"); // No other protocols yet
  p=url+7;
  bzero(buf,PARSE_BUF_SIZE);
  i=0;
  
  while(*p && (*p!='/')) {buf[i]=*p; ++i; ++p;}
  
  if(!i)
   {
    fui->err_msg=strdup("Hostname is missing");
    return fui;
   }
  
  tmp=strchr(buf,':');
  if(tmp==buf)
   {
    fui->err_msg=strdup("Hostname is missing");
    return fui;
   }
  
  if(tmp)
   {
    if(tmp!=strrchr(buf,':') || !(*(tmp+1)))
     {
      fui->err_msg=strdup("Wrong port specs");
      return fui;
     }
    fui->port=atoi(tmp+1);
    *tmp=0;
    fui->host=strdup(buf);
   }   
  else
   {
    fui->host=strdup(buf);
    fui->port=80;
   }
   
  if(fui->port<=0 || fui->port>=65534)
   {
    fui->err_msg=strdup("Wrong port number");
    return fui;
   }
   
  if(*p) while(*p && *p=='/') ++p;
  
  if(!*p) return fui;
  
  bzero(buf,PARSE_BUF_SIZE);
  i=0;  
  while(*p) {buf[i]=*p; ++i; ++p;}
  if(i) fui->path=strdup(buf);
  
  return fui;
 }
 
 
int frontier_resolv_host(FrontierUrlInfo *fui)
 {
  struct addrinfo hints;
  int ret;
  const char *err_msg;
  struct addrinfo *addr;  
  
  bzero(&hints,sizeof(struct addrinfo));
  
  hints.ai_family=PF_INET;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  
  ret=getaddrinfo(fui->host,NULL,&hints,&(fui->addr));
  if(ret)
   {
    if(ret==EAI_SYSTEM)
     {
      ret=errno;
      fui->err_msg=strdup(strerror(errno));
     }
    else
     {     
      err_msg=gai_strerror(ret);
      fui->err_msg=strdup(err_msg);
     }
    return -1;
   }    
  
  addr=fui->addr;
  do
   {
    struct sockaddr_in *sin;
    sin=(struct sockaddr_in*)(addr->ai_addr);
    printf("Found addr <%s>\n",inet_ntoa(sin->sin_addr));
    addr=addr->ai_next;
   }while(addr);   
   
  return 0; 
 }
 
 
 
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui)
 {
  if(fui->addr) freeaddrinfo(fui->addr);
  if(fui->url) free(fui->url);
  if(fui->proto) free(fui->proto);
  if(fui->host) free(fui->host);
  if(fui->path) free(fui->path);
  if(fui->err_msg) free(fui->err_msg);
  
  free(fui);
 }
 
