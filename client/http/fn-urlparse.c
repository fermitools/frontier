/*
 * frontier client url parsing
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

#include <fn-htclient.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
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
  fui->fai=&fui->firstfai;
  
  fui->url=frontier_str_copy(url);
  if(!fui->url)
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
 
static void frontier_FreeAddrInfo(FrontierUrlInfo *fui)
 {
  if(fui->firstfai.addr)
   {
    // this frees all addrinfo structures in the round-robin chain
    freeaddrinfo(fui->firstfai.addr);
   }
  while(fui->firstfai.next)
   {
    fui->fai=fui->firstfai.next->next;
    frontier_mem_free(fui->firstfai.next);
    fui->firstfai.next=fui->fai;
   }
  bzero(&fui->firstfai,sizeof(FrontierAddrInfo));
 }
 
int frontier_resolv_host(FrontierUrlInfo *fui)
 {
  struct addrinfo hints;
  int ret;
  struct addrinfo *addr;
  FrontierAddrInfo *fai;
  time_t now=time(0);
  
  if(fui->fai->addr)
   {
    if((now-fui->whenresolved)<FRONTIER_RERESOLVE_SECS)
      return FRONTIER_OK;
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"re-resolving host %s",fui->host);
   }
  fui->whenresolved=now;

  bzero(&hints,sizeof(struct addrinfo));
  
  hints.ai_family=PF_INET;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  
  frontier_FreeAddrInfo(fui);
  ret=getaddrinfo(fui->host,NULL,&hints,&(fui->firstfai.addr));
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

  fai=&fui->firstfai;
  addr=fai->addr;
  do
   {
    struct sockaddr_in *sin;
    sin=(struct sockaddr_in*)(addr->ai_addr);
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%s: found addr <%s>",fui->host,inet_ntoa(sin->sin_addr));
    addr=addr->ai_next;
    if(addr)
     {
      FrontierAddrInfo *nextfai=frontier_mem_alloc(sizeof(FrontierAddrInfo));
      if(!fai)
       {
	FRONTIER_MSG(FRONTIER_EMEM);
	return FRONTIER_EMEM;
       }
      bzero(nextfai,sizeof(FrontierAddrInfo));
      nextfai->addr=addr;
      fai->next=nextfai;
      fai=nextfai;
     }
   }while(addr);   
   
  fui->fai=fui->lastfai=&fui->firstfai;
  return FRONTIER_OK; 
 }
 
 
 
void frontier_DeleteUrlInfo(FrontierUrlInfo *fui)
 {
  if(!fui) return;
  frontier_FreeAddrInfo(fui);
  if(fui->url) frontier_mem_free(fui->url);
  if(fui->proto) frontier_mem_free(fui->proto);
  if(fui->host) frontier_mem_free(fui->host);
  if(fui->path) frontier_mem_free(fui->path);
  
  frontier_mem_free(fui);
 }
 
