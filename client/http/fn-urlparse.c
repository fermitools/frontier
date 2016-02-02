/*
 * frontier client url parsing
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

#include <fn-htclient.h> 
#include "../fn-internal.h"
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
  fui->fai=fui->lastfai=fui->firstfaiinfamily=&fui->firstfai;
  
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
 
void frontier_FreeAddrInfo(FrontierUrlInfo *fui)
 {
  if(fui->ai)
   {
    // this frees all addrinfo structures in the round-robin chain
    freeaddrinfo(fui->ai);
    fui->ai=0;
   }
  while(fui->firstfai.next)
   {
    fui->fai=fui->firstfai.next->next;
    frontier_mem_free(fui->firstfai.next);
    fui->firstfai.next=fui->fai;
   }
  bzero(&fui->firstfai,sizeof(FrontierAddrInfo));
  fui->fai=fui->lastfai=fui->firstfaiinfamily=&fui->firstfai;
 }
 
int frontier_resolv_host(FrontierUrlInfo *fui,int preferipfamily)
 {
  struct addrinfo hints;
  int ret;
  struct addrinfo *ai;
  FrontierAddrInfo *fai;
  sa_family_t preferaf;
  
  if(fui->fai->ai)
    return FRONTIER_OK;

  if (preferipfamily!=0)
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"getting addr info for %s, prefer ipv%d",fui->host,preferipfamily);

  bzero(&hints,sizeof(struct addrinfo));
  
  hints.ai_family=PF_UNSPEC;
  hints.ai_socktype=SOCK_STREAM;
  hints.ai_protocol=0;
  
  frontier_FreeAddrInfo(fui);
  ret=getaddrinfo(fui->host,NULL,&hints,&(fui->ai));
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

  // for each ai after the first, allocate an fai
  // (it's after the first because one fai is pre-allocated)
  ai=fui->ai->ai_next;
  fai=&fui->firstfai;
  while(ai)
   {
    FrontierAddrInfo *nextfai=frontier_mem_alloc(sizeof(FrontierAddrInfo));
    if(!fai)
     {
      FRONTIER_MSG(FRONTIER_EMEM);
      return FRONTIER_EMEM;
     }
    bzero(nextfai,sizeof(FrontierAddrInfo));
    fai->next=nextfai;
    fai=nextfai;
    ai=ai->ai_next;
   }

  fai=&fui->firstfai;

  // add preferred family addresses
  ai=fui->ai;
  if(preferipfamily==4)
    preferaf=AF_INET;
  else if(preferipfamily==6)
    preferaf=AF_INET6;
  else
    preferaf=ai->ai_family; // prefer the family of the first one
  while(ai)
   {
    if(ai->ai_family==preferaf)
     {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%s: found ipv%d addr <%s>",fui->host,preferipfamily,frontier_ipaddr(ai->ai_addr));
      fai->ai=ai;
      fai=fai->next;
     }
    ai=ai->ai_next;
   }
   
  // add non-preferred family addresses
  ai=fui->ai;
  while(ai)
   {
    if(ai->ai_family!=preferaf)
     {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%s: found addr <%s>",fui->host,frontier_ipaddr(ai->ai_addr));
      fai->ai=ai;
      fai=fai->next;
     }
    ai=ai->ai_next;
   }
   
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
 
