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

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *p);


FrontierUrlInfo *frontier_CreateUrlInfo(const char *url,int *ec)
 {
  FrontierUrlInfo *fui;
  const char *p;
  char *tmp;
  int i;
  char buf[PARSE_BUF_SIZE];
  
  fui=frontier_mem_alloc(sizeof(FrontierUrlInfo));
  if(!fui)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    goto err;
   }
  bzero(fui,sizeof(FrontierUrlInfo));
  
  fui->url=frontier_str_copy(url);
  if(!fui)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    goto err;
   }
  
  if(strncmp(url,"http://",7))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"config error: bad url %s",url);
    *ec=FRONTIER_ECFG;
    goto err;
   }
  fui->proto=frontier_str_copy("http"); // No other protocols yet
  p=url+7;
  bzero(buf,PARSE_BUF_SIZE);
  i=0;
  
  while(*p && (*p!='/')) {buf[i]=*p; ++i; ++p;}
  
  if(!i)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"config error: bad url %s",url);
    *ec=FRONTIER_ECFG;
    goto err;
   }
  
  tmp=strchr(buf,':');
  if(tmp==buf)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"config error: bad url %s",url);
    *ec=FRONTIER_ECFG;
    goto err;
   }
  
  if(tmp)
   {
    if(tmp!=strrchr(buf,':') || !(*(tmp+1)))
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"config error: bad url %s",url);
      *ec=FRONTIER_ECFG;
      goto err;
     }
    fui->port=atoi(tmp+1);
    *tmp=0;
    fui->host=frontier_str_copy(buf);
   }   
  else
   {
    fui->host=frontier_str_copy(buf);
    fui->port=80;
   }
   
  if(fui->port<=0 || fui->port>=65534)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"config error: bad port number in the url %s",url);
    *ec=FRONTIER_ECFG;
    goto err;
   }
   
  if(*p) while(*p && *p=='/') ++p;
  
  if(!*p) return fui;
  
  bzero(buf,PARSE_BUF_SIZE);
  i=0;  
  while(*p) {buf[i]=*p; ++i; ++p;}
  if(i) fui->path=frontier_str_copy(buf);

  goto ok;
  
err:
  frontier_DeleteUrlInfo(fui);
  return NULL;
  
ok:  
  *ec=FRONTIER_OK;
  return fui;
 }
 
 
int frontier_resolv_host(FrontierUrlInfo *fui)
 {
  struct addrinfo hints;
  int ret;
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
      frontier_setErrorMsg(__FILE__,__LINE__,"host name %s problem: %s",fui->host,strerror(errno));
     }
    else
     {     
      frontier_setErrorMsg(__FILE__,__LINE__,"host name %s problem: %s",fui->host,gai_strerror(ret));
     }
    return FRONTIER_ENETWORK;
   }    
  
  addr=fui->addr;
  do
   {
    struct sockaddr_in *sin;
    sin=(struct sockaddr_in*)(addr->ai_addr);
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%s: found addr <%s>",fui->host,inet_ntoa(sin->sin_addr));
    addr=addr->ai_next;
   }while(addr);   
   
  return FRONTIER_OK; 
 }
 
 
 
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui)
 {
  if(!fui) return;
  if(fui->addr) freeaddrinfo(fui->addr);
  if(fui->url) frontier_mem_free(fui->url);
  if(fui->proto) frontier_mem_free(fui->proto);
  if(fui->host) frontier_mem_free(fui->host);
  if(fui->path) frontier_mem_free(fui->path);
  
  frontier_mem_free(fui);
 }
 
