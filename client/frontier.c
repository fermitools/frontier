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

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <curl/curl.h>
#include <frontier.h>
#include "fn-internal.h"

void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);
static int initialized=0;


struct s_HttpHeader
 {
  char *name;
  char *value;
 };
typedef struct s_HttpHeader HttpHeader;


static void clean_headers(FrontierMemData *m);


static const char *str_copy(const char *str, size_t len)
 {
  char *ret=frontier_mem_alloc(len+1);

  if(!ret) return ret;

  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }


int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr))
 {
  CURLcode res;

  if(initialized) return FRONTIER_OK;

  if(!f_mem_alloc) {f_mem_alloc=malloc; f_mem_free=free;}
  if(!f_mem_free) {f_mem_alloc=malloc; f_mem_free=free;}

  frontier_mem_alloc=f_mem_alloc;
  frontier_mem_free=f_mem_free;

  res=curl_global_init(CURL_GLOBAL_ALL);
  if(res) return FRONTIER_EEND-res;

  initialized=1;
  
  return FRONTIER_OK;
 }



static Channel *channel_create(int *ec)
 {
  Channel *chn;

  chn=frontier_mem_alloc(sizeof(Channel));
  if(!chn) {*ec=FRONTIER_EMEM;return chn;}

  chn->resp=(void*)0;
 
  chn->md_head=frontierMemData_create();
  if(!chn->md_head)
   {
    frontier_mem_free(chn);
    *ec=FRONTIER_EMEM;
    return (void*)0;
   }

  chn->curl=curl_easy_init();
  if(!chn->curl)
   {
    frontierMemData_delete(chn->md_head);
    frontier_mem_free(chn);
    *ec=FRONTIER_ECURLI;
    return (void*)0;
   }
  
  chn->status=FRONTIER_SEMPTY;
  chn->proxy_url=(char*)0;

  *ec=FRONTIER_OK;
 
  return chn;
 }


static void channel_delete(Channel *chn)
 {
  if(!chn) return;
  clean_headers(chn->md_head);
  if(chn->curl) curl_easy_cleanup(chn->curl);
  frontierMemData_delete(chn->md_head);
  frontierResponse_delete(chn->resp);

  if(chn->proxy_url) frontier_mem_free((void*)chn->proxy_url);

  frontier_mem_free(chn);
 }


FrontierChannel frontier_createChannel(int *ec)
 {
  Channel *chn=channel_create(ec);
  return (unsigned long)chn;
 }


void frontier_closeChannel(FrontierChannel fchn)
 {
  channel_delete((Channel*)fchn);
 }


void frontier_setProxy(FrontierChannel u_channel,const char *proxy,int *ec)
 {
  Channel *chn=(Channel*)u_channel;
  int len;

  if(!proxy)
   {
    *ec=FRONTIER_EIARG;
    return;
   }

  len=strlen(proxy);
  if(!len)
   {
    *ec=FRONTIER_EIARG;
    return;
   }
  
  if(chn->proxy_url) frontier_mem_free((void*)chn->proxy_url);

  chn->proxy_url=str_copy(proxy,len);

  if(!chn->proxy_url) *ec=FRONTIER_EMEM;
  else *ec=FRONTIER_OK;
 }


static size_t write_data(void *buf,size_t size,size_t nmemb,void *u)
 {
  FrontierResponse *resp=(FrontierResponse*)u;
  size_t len=size*nmemb;
  int ret;

  ret=FrontierResponse_append(resp,buf,len);
  if(ret!=FRONTIER_OK)
   {
    return 0;
   }
  return len;
 }



static int httpHeader_append(FrontierMemData *m,char *str,int size)
 {
  HttpHeader dummy;
  HttpHeader *h;
  char *p;
  int i;
  int len;
  int ret;


  if(m->len==0 && strncmp(str,"HTTP/",5)==0)
   {
    return FRONTIER_OK;
   }

  if(m->len && *str=='\r')
   {
    return FRONTIER_OK;
   }

  if(!str) return FRONTIER_EIARG;
  if(size<=4) return FRONTIER_EIARG; // 3 is the minimum - A:b\0 at least

  bzero(&dummy,sizeof(HttpHeader));
  ret=frontierMemData_append(m,(char*)&dummy,sizeof(HttpHeader));
  if(ret!=FRONTIER_OK) return ret;
 
  m->len-=sizeof(HttpHeader); // Will restore after successful parsing
  h=(HttpHeader*)(m->buf+m->len);

  p=str;
  i=0;
  len=size;
  while(p[i]!=':' && i<len) ++i;
  if(!i) goto err;
  if(i>=len) goto err;

  p[i]=0;
  h->name=frontier_mem_alloc(i+1); // +1 for the trailing 0
  if(!h->name) goto err_mem;
  bcopy(p,h->name,i+1);
  p=p+(i+1);
  len=len-i-1;

  i=0;
  while(p[i]==' ' && i<len) ++i;
  if(i>=len) goto err;
  p=p+i;
  len=len-i;
  i=0;

  while(p[i] && p[i]!='\n' && p[i]!='\r' && i<len) ++i;
  if(i>=len || !i) {goto err;}
  p[i]=0;
  h->value=frontier_mem_alloc(i+1); // +1 for \0
  if(!h->value) {frontier_mem_free(h->name);goto err_mem;}
  bcopy(p,h->value,i+1);
  goto Ok;

err_mem:
  return FRONTIER_EMEM;

err:
  if(h->name) frontier_mem_free(h->name);
  if(h->value) frontier_mem_free(h->value);
  return FRONTIER_EIARG;

Ok:
  m->len+=sizeof(HttpHeader); // Add the header
  return FRONTIER_OK;
 }



static size_t write_header(void *buf,size_t size,size_t nmemb,void *u)
 {
  FrontierMemData *md=(FrontierMemData*)u;
  size_t len=size*nmemb;
  int ret;

  ret=httpHeader_append(md,buf,len);
  if(ret!=FRONTIER_OK)
   {
    return -1;
   }

  return len;
 }



static void clean_headers(FrontierMemData *m)
 {
  int i;
  HttpHeader *h;

  for(i=0;i<m->len;i+=sizeof(HttpHeader))
   {
    h=(HttpHeader*)(m->buf+i);
    frontier_mem_free(h->name);
    frontier_mem_free(h->value);
   }
  m->len=0;
 }



int frontier_getRawData(FrontierChannel u_channel,const char *url)
 {
  CURLcode res;
  Channel *chn=(Channel*)u_channel;
  struct curl_slist *headers=(void*)0;
  int ret=FRONTIER_OK;
  long v_long;

  if(!chn) {ret=FRONTIER_EIARG; goto err;}

  if(chn->resp) frontierResponse_delete(chn->resp);

  chn->resp=frontierResponse_create();
  if(!chn->resp) { ret=FRONTIER_EMEM; goto err; }

  clean_headers(chn->md_head);
  chn->status=FRONTIER_SEMPTY;

  res=curl_easy_setopt(chn->curl,CURLOPT_URL,url);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  res=curl_easy_setopt(chn->curl,CURLOPT_WRITEFUNCTION,write_data);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  res=curl_easy_setopt(chn->curl,CURLOPT_WRITEDATA,chn->resp);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  res=curl_easy_setopt(chn->curl,CURLOPT_HEADERFUNCTION,write_header);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  res=curl_easy_setopt(chn->curl,CURLOPT_WRITEHEADER,chn->md_head);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  headers=curl_slist_append(headers, "pragma:");  
  res=curl_easy_setopt(chn->curl,CURLOPT_HTTPHEADER,headers);
  if(res) {ret=FRONTIER_EEND-res; goto err;}

  if(chn->proxy_url)
   {
    res=curl_easy_setopt(chn->curl,CURLOPT_PROXY,chn->proxy_url);
    if(res) {ret=FRONTIER_EEND-res; goto err;}
   }

  res=curl_easy_perform(chn->curl);
  curl_slist_free_all(headers);

  if(res)
   {
    if(res==CURLE_WRITE_ERROR && chn->resp->error==FRONTIER_XMLPARSE)
     {
      ret=FRONTIER_XMLPARSE;
      goto err;
     }
    ret=FRONTIER_EEND-res;
    goto err;
   }

  res=curl_easy_getinfo(chn->curl,CURLINFO_HTTP_CODE,&v_long);
  if(res==CURLE_OK) chn->http_resp_code=v_long;
  else chn->http_resp_code=-1;

  if(chn->http_resp_code!=200)
   {
    ret=FRONTIER_ENON200;
    goto err;
   }

  ret=frontierResponse_finalize(chn->resp);
  if(ret!=FRONTIER_OK) goto err;

  chn->status=FRONTIER_SRAW_DATA;
  goto Ok;

err:
  chn->status=FRONTIER_SERROR;
Ok:
  chn->error=ret;
  return ret;
 }


void frontier_getRespStat(FrontierChannel u_channel,FrontierRespStat *stat)
 {
  Channel *chn=(Channel*)u_channel;

  if(!chn)
   {
    stat->status=FRONTIER_SERROR;
    stat->error=FRONTIER_EIARG;
    return;
   }

  stat->status=chn->status;
  stat->error=chn->error;
  stat->http_resp_code=chn->http_resp_code;
  stat->raw_data_size=0; // XXX
  stat->raw_data_buf=(void*)0; // XXX
 }


const char *frontier_getHttpHeaderName(FrontierChannel u_channel,int num)
 {
  Channel *chn=(Channel*)u_channel;
  HttpHeader *h;
  int off=num*sizeof(HttpHeader);

  if(off>=chn->md_head->len) return (void*)0;
  h=(HttpHeader*)(chn->md_head->buf+off);
  return h->name;
 }



const char *frontier_getHttpHeaderValue(FrontierChannel u_channel,int num)
 {
  Channel *chn=(Channel*)u_channel;
  HttpHeader *h;
  int off=num*sizeof(HttpHeader);

  if(off>=chn->md_head->len) return (void*)0;
  h=(HttpHeader*)(chn->md_head->buf+off);
  return h->value;
 }


