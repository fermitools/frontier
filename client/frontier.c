/*
 * frontier client C API implementation
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"
#include "fn-hash.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>


int frontier_log_level;
char *frontier_log_file;
int frontier_log_dup = 0;
void *(*frontier_mem_alloc)(size_t size);
void (*frontier_mem_free)(void *ptr);
static int initialized=0;

static char frontier_id[FRONTIER_ID_SIZE];
static char frontier_api_version[]=FNAPI_VERSION;

static int chan_seqnum=0;
static void channel_delete(Channel *chn);

static fn_client_cache_list *client_cache_list=0;


// our own implementation of strndup
char *frontier_str_ncopy(const char *str, size_t len)
 {
  char *ret;
  
  ret=frontier_mem_alloc(len+1);
  if(!ret) return ret;
  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }


// our own implementation of strdup
char *frontier_str_copy(const char *str)
 {
  int len=strlen(str);
  char *ret;
  
  ret=frontier_mem_alloc(len+1);
  if(!ret) return ret;
  bcopy(str,ret,len);
  ret[len]=0;

  return ret;
 }

/* this returns the real owner of grid jobs */
static char *getX509Subject()
 {
  char *filename=getenv("X509_USER_PROXY");
  BIO *biof=NULL;
  X509 *x509cert=NULL;
  char *subject=NULL;

  if(filename==NULL)return NULL;
  biof=BIO_new(BIO_s_file());
  if(BIO_read_filename(biof,filename)!=0)
   {
    x509cert=PEM_read_bio_X509_AUX(biof,0,0,0);
    if(x509cert!=NULL)
      subject=X509_NAME_oneline(X509_get_subject_name(x509cert),0,0);
   }

  if (biof != NULL) BIO_free(biof);
  if (x509cert != NULL) X509_free(x509cert);
  return subject;
 }
 

// get current time as a string
char *frontier_str_now()
 {
  time_t now=time(NULL);
  char *cnow=ctime(&now);
  // eliminate trailing newline
  cnow[strlen(cnow)-1]='\0';
  return cnow;
 }

int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr))
{
  return(frontier_initdebug(f_mem_alloc,f_mem_free,
		getenv(FRONTIER_ENV_LOG_FILE),getenv(FRONTIER_ENV_LOG_LEVEL)));
}

int frontier_initdebug(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr),
			const char *logfilename, const char *loglevel)
 {
  uid_t uid;
  struct passwd *pwent;
  pid_t pid;
  char *appId;
  char *x509Subject;
  
  if(initialized) return FRONTIER_OK;

  if(!f_mem_alloc) {f_mem_alloc=malloc; f_mem_free=free;}
  if(!f_mem_free) {f_mem_alloc=malloc; f_mem_free=free;}

  frontier_mem_alloc=f_mem_alloc;
  frontier_mem_free=f_mem_free;
    
  if(!loglevel) 
   {
    frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
    frontier_log_file=(char*)0;
   }
  else
   {
    if(strcasecmp(loglevel,"warning")==0 || strcasecmp(loglevel,"info")==0) frontier_log_level=FRONTIER_LOGLEVEL_WARNING;
    else if(strcasecmp(loglevel,"error")==0) frontier_log_level=FRONTIER_LOGLEVEL_ERROR;
    else if(strcasecmp(loglevel,"nolog")==0) frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
    else frontier_log_level=FRONTIER_LOGLEVEL_DEBUG;
    
    if(!logfilename)
     {
      frontier_log_file=(char*)0;
     }
    else
     {
      if(*logfilename=='+')
       {
	frontier_log_dup=1;
	logfilename++;
       }
      int fd=open(logfilename,O_CREAT|O_APPEND|O_WRONLY,0644);
      if(fd<0) 
       {
        printf("Can not open log file %s. Log is disabled.\n",logfilename);
	frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
        frontier_log_file=(char*)0;
       }
      else
       {
        close(fd);
        frontier_log_file=frontier_str_copy(logfilename);
       }
     }
   }

  uid=getuid();
  pwent=getpwuid(uid);
  pid=getpid();
  appId=getenv("CMSSW_VERSION");
  if(appId==NULL)
    appId="client";
  x509Subject=getX509Subject();

  snprintf(frontier_id,FRONTIER_ID_SIZE,"%s %s %d %s(%d) %s",appId,frontier_api_version,pid,pwent->pw_name,uid,(x509Subject!=NULL)?x509Subject:pwent->pw_gecos);
  
  initialized=1;
  
  return FRONTIER_OK;
 }


/* note that caller is responsible for creating config but channel_create2
   or later call to channel_delete is responsible for deleting it */
static Channel *channel_create2(FrontierConfig *config, int *ec)
 {
  Channel *chn;
  int ret;
  const char *p;
  fn_client_cache_list *cache_listp;
  char *servlet;

  chn=frontier_mem_alloc(sizeof(Channel));
  if(!chn) 
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    if(config)frontierConfig_delete(config);
    return (void*)0;
   }
  bzero(chn,sizeof(Channel));

  chn->seqnum=++chan_seqnum;
  chn->cfg=config;
  if(!chn->cfg)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    channel_delete(chn);    
    return (void*)0;
   }
  if(!chn->cfg->server_num)
   {
    *ec=FRONTIER_ECFG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no servers configured");
    channel_delete(chn);    
    return (void*)0;
   }
  
  chn->ht_clnt=frontierHttpClnt_create(ec);
  if(!chn->ht_clnt||*ec)
   {
    channel_delete(chn);    
    return (void*)0;
   }
   
  do
   {
    p=frontierConfig_getServerUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addServer(chn->ht_clnt,p);
    if(ret)
     {
      *ec=ret;
      channel_delete(chn);    
      return (void*)0;
     }
   }while(frontierConfig_nextServer(chn->cfg)==0);
   
  if(!chn->ht_clnt->total_server)
   {
    *ec=FRONTIER_ECFG;
    frontier_setErrorMsg(__FILE__,__LINE__,"no server configured");
    channel_delete(chn);    
    return (void*)0;
   }

  if(frontierConfig_getBalancedServers(chn->cfg))
    frontierHttpClnt_setBalancedServers(chn->ht_clnt);

  do
   {
    p=frontierConfig_getProxyUrl(chn->cfg);
    if(!p) break;
    ret=frontierHttpClnt_addProxy(chn->ht_clnt,p);
    if(ret)
     {
      *ec=ret;
      channel_delete(chn);    
      return (void*)0;
     }
   }while(frontierConfig_nextProxy(chn->cfg)==0);

  if(frontierConfig_getBalancedProxies(chn->cfg))
    frontierHttpClnt_setBalancedProxies(chn->ht_clnt);

  chn->client_cache_maxsize=frontierConfig_getClientCacheMaxResultSize(chn->cfg);
  if(chn->client_cache_maxsize>0)
   {
    chn->client_cache_buf=frontier_mem_alloc(chn->client_cache_maxsize);
    if(!chn->client_cache_buf)
     {
      *ec=FRONTIER_EMEM;
      FRONTIER_MSG(*ec);
      channel_delete(chn);    
      return (void*)0;
     }

    // get the path component of one of the servers (they're all the same)
    //  which is the servlet name
    servlet=frontierHttpClnt_curserverpath(chn->ht_clnt);

    // locate the client cache for this servlet if it exists
    for(cache_listp=client_cache_list;cache_listp!=NULL;cache_listp=cache_listp->next)
     {
      if(strcmp(cache_listp->servlet,servlet)==0)
       {
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"using existing %s client cache for responses less than %d bytes",
				servlet,chn->client_cache_maxsize);
	break;
       }
     }
    if(!cache_listp)
     {
      // doesn't yet exist, create a client cache and its list entry.
      // note that they are never deleted.
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"creating %s client cache for responses less than %d bytes",
      				servlet,chn->client_cache_maxsize);
      // leave room for the servlet name after the list entry
      cache_listp=(fn_client_cache_list *)frontier_mem_alloc(sizeof(*cache_listp)+strlen(servlet)+1);
      if(!cache_listp)
       {
        *ec=FRONTIER_EMEM;
        FRONTIER_MSG(*ec);
        channel_delete(chn);    
	frontier_mem_free(cache_listp);
        return (void*)0;
       }
      cache_listp->table=fn_inithashtable();
      if(!cache_listp->table)
       {
        *ec=FRONTIER_EMEM;
        FRONTIER_MSG(*ec);
        channel_delete(chn);    
	frontier_mem_free(cache_listp);
        return (void*)0;
       }
      // tack the servlet name on the end, space was allocated above
      cache_listp->servlet=((char *)cache_listp)+sizeof(*cache_listp);
      strcpy(cache_listp->servlet,servlet);
      cache_listp->next=client_cache_list;
      client_cache_list=cache_listp;
     }
    chn->client_cache=cache_listp;
   }
  frontierHttpClnt_setConnectTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getConnectTimeoutSecs(chn->cfg));
  frontierHttpClnt_setReadTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getReadTimeoutSecs(chn->cfg));
  frontierHttpClnt_setWriteTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getWriteTimeoutSecs(chn->cfg));

  chn->reload=0;
  *ec=FRONTIER_OK; 
  return chn;
 }

static Channel *channel_create(const char *srv,const char *proxy,int *ec)
 {
  FrontierConfig *cfg=frontierConfig_get(srv,proxy,ec);
  if(!cfg) 
    return (void*)0;
  return channel_create2(cfg,ec);
 }

static void channel_delete(Channel *chn)
 {
  if(!chn) return;
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"closing chan %d at %s",chn->seqnum,frontier_str_now());
  frontierHttpClnt_delete(chn->ht_clnt);
  if(chn->resp)frontierResponse_delete(chn->resp);
  frontierConfig_delete(chn->cfg);
  if(chn->client_cache_buf)frontier_mem_free(chn->client_cache_buf);
  frontier_mem_free(chn);
 }


FrontierChannel frontier_createChannel(const char *srv,const char *proxy,int *ec)
 {
  Channel *chn=channel_create(srv,proxy,ec);
  return (unsigned long)chn;
 }

FrontierChannel frontier_createChannel2(FrontierConfig* config, int *ec) {
  Channel *chn = channel_create2(config, ec);
  return (unsigned long)chn;
}


void frontier_closeChannel(FrontierChannel fchn)
 {
  channel_delete((Channel*)fchn);
 }

 
void frontier_setReload(FrontierChannel u_channel,int reload)
 {
  Channel *chn=(Channel*)u_channel;  
  chn->user_reload=reload;
 }
 

static int write_data(FrontierResponse *resp,void *buf,int len)
 {
  int ret;
  ret=FrontierResponse_append(resp,buf,len);
  return ret;
 }


static int prepare_channel(Channel *chn)
 {
  int ec;
  
  if(chn->resp) frontierResponse_delete(chn->resp);
  chn->resp=frontierResponse_create(&ec);  
  chn->resp->seqnum=++chn->response_seqnum;
  if(!chn->resp) return ec;
  
  return FRONTIER_OK;
 }
 
 
static int get_data(Channel *chn,const char *uri,const char *body)
 {
  int ret=FRONTIER_OK;
  char buf[8192];
  const char *force_reload;
  char *uri_copy,*data_copy;
  int uri_len;
  int reload;
  int client_cache_bufsize;
  fn_hashval *hashval;
  char statbuf[128];

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }

  ret=prepare_channel(chn);
  if(ret) return ret;

  if(!body&&(chn->client_cache_maxsize>0))
   {
    if((hashval=fn_hashtable_search(chn->client_cache->table,(char *)uri))!=0)
     {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"HIT in %s client cache, skipping contacting server",chn->client_cache->servlet);
      return write_data(chn->resp,hashval->data,hashval->len);
     }
   }
  
  reload=chn->reload;
  if(!reload)
  {
    // Reload not requested, see if need to force a reload
    force_reload=frontierConfig_getForceReload(chn->cfg);
    if(strcmp(force_reload,"short")==0)
      reload=chn->user_reload;
    else if(strcmp(force_reload,"long")==0)
      reload=1;
  }
  frontierHttpClnt_setCacheRefreshFlag(chn->ht_clnt,reload);
  /* User-requested reloads are translated into a "short" time-to-live
     where the length of time is defined by the server.  Add the suffix
     even when reloads are forced to make sure the same URL is cleared
     in the caches. */
  frontierHttpClnt_setUrlSuffix(chn->ht_clnt,chn->user_reload ? "&ttl=short" : "");
  frontierHttpClnt_setFrontierId(chn->ht_clnt,frontier_id);
  
  ret=frontierHttpClnt_open(chn->ht_clnt);
  if(ret) goto end;

  if(body) ret=frontierHttpClnt_post(chn->ht_clnt,uri,body);
  else ret=frontierHttpClnt_get(chn->ht_clnt,uri);
  if(ret) goto end;
  
  if(body)client_cache_bufsize=-1;
  else client_cache_bufsize=0;

  while(1)
   {
    ret=frontierHttpClnt_read(chn->ht_clnt,buf,8192);
    if(ret<0) goto end;
    if(ret==0) break;
#if 0
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"read %d bytes from server",ret);
#endif
    if((chn->client_cache_maxsize>0)&&(client_cache_bufsize>=0))
     {
      if(ret+client_cache_bufsize<=chn->client_cache_maxsize)
       {
        bcopy(buf,chn->client_cache_buf+client_cache_bufsize,ret);
        client_cache_bufsize+=ret;
       }
       else
       {
        //too big, don't append any more chunks that might fit
        client_cache_bufsize=-1;
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"result too large to cache");
       }
     }
    ret=write_data(chn->resp,buf,ret);
    if(ret!=FRONTIER_OK)  goto end;
   }

  if(client_cache_bufsize>0)
   {
    uri_len=strlen(uri)+1;
    if((uri_copy=frontier_mem_alloc(uri_len))==0)
     {
      ret=FRONTIER_EMEM;
      FRONTIER_MSG(ret);
     }
    else if((data_copy=frontier_mem_alloc(client_cache_bufsize))==0)
     {
      ret=FRONTIER_EMEM;
      FRONTIER_MSG(ret);
      frontier_mem_free(uri_copy);
     }
    else if((hashval=(fn_hashval *)frontier_mem_alloc(sizeof(fn_hashval)))==0)
     {
      ret=FRONTIER_EMEM;
      FRONTIER_MSG(ret);
      frontier_mem_free(uri_copy);
      frontier_mem_free(data_copy);
     }
    else
     {
      bcopy(uri,uri_copy,uri_len);
      bcopy(chn->client_cache_buf,data_copy,client_cache_bufsize);
      hashval->len=client_cache_bufsize;
      hashval->data=data_copy;
      if(!fn_hashtable_insert(chn->client_cache->table,uri_copy,hashval))
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"error inserting result in client cache");
	ret=FRONTIER_EMEM;
	frontier_mem_free(uri_copy);
	frontier_mem_free(data_copy);
	frontier_mem_free(hashval);
       }
      else if(frontier_log_level==FRONTIER_LOGLEVEL_DEBUG)
       {
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"inserted %d byte result in %s client cache with %d byte key",
		client_cache_bufsize,chn->client_cache->servlet,uri_len);
        fn_hashtable_stats(chn->client_cache->table,statbuf,sizeof(statbuf));
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"%s client cache hash stats: %s",chn->client_cache->servlet,statbuf);
       }
     }
   }
   
end:
  frontierHttpClnt_close(chn->ht_clnt);
        
  return ret;
 }
 

#define ERR_LAST_BUF_SIZE 1024

int frontier_getRawData(FrontierChannel u_channel,const char *uri)
 {
  int ret;
  
  ret=frontier_postRawData(u_channel,uri,NULL);
  
  return ret;
 }
 
 
int frontier_postRawData(FrontierChannel u_channel,const char *uri,const char *body)
 {
  Channel *chn=(Channel*)u_channel;
  int ret=FRONTIER_OK;
  FrontierHttpClnt *clnt;
  char err_last_buf[ERR_LAST_BUF_SIZE];
  int tried_refresh_proxies=0;
  int curproxy,curserver;

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }
  
  clnt=chn->ht_clnt;
  curproxy=frontierHttpClnt_resetproxylist(clnt,1);
  curserver=frontierHttpClnt_resetserverlist(clnt,1);
  chn->reload=0;
  bzero(err_last_buf,ERR_LAST_BUF_SIZE);
  
  while(1)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"querying on chan %d at %s",chn->seqnum,frontier_str_now());

    ret=get_data(chn,uri,body);    
    if(ret==FRONTIER_OK) 
     {
      ret=frontierResponse_finalize(chn->resp);
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"chan %d response %d finished at %s",chn->seqnum,chn->resp->seqnum,frontier_str_now());
      if(ret==FRONTIER_OK) break;
     }
    snprintf(err_last_buf,ERR_LAST_BUF_SIZE,"Request %d on chan %d failed at %s: %d %s",chn->resp->seqnum,chn->seqnum,frontier_str_now(),ret,frontier_getErrorMsg());
    frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,err_last_buf);
    
    if(curproxy>=0)
     {
      if (ret==FRONTIER_EPROTO)
       {
        /*The problem was not with the proxy, it was with the server.*/
	/*Note that this doesn't cover the case of non-response from*/
	/* the server because that will be a timeout which looks the*/
	/* same as a timeout on the proxy.*/
	/*Pretend that last proxy has been tried & reloaded.*/
	chn->reload=1;
	/*and fall through to try the next server*/
       }
      else
       {
	curproxy=frontierHttpClnt_nextproxy(clnt,1);
	if(curproxy>=0)
	 {
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next proxy %s",frontierHttpClnt_curproxyname(clnt));
	  continue;
	 }
	else if(chn->reload)
	 {
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying direct connect to server %s",frontierHttpClnt_curservername(clnt));
	  chn->reload=0;
	  continue;
	 }
	 /*else fall through to refresh the server*/
       }
     }

    if(!chn->reload)
     {
      chn->reload=1;
      if(!tried_refresh_proxies)
       {
        tried_refresh_proxies=1;
	curproxy=frontierHttpClnt_resetproxylist(clnt,0);
	if(curproxy>=0)
	 {
          frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying refresh cache on proxies starting with %s",frontierHttpClnt_curproxyname(clnt));
	  continue;
	 }
       }
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying refresh cache on direct connect to server %s",frontierHttpClnt_curservername(clnt));
      continue;
     }
    chn->reload=0;
    
    curserver=frontierHttpClnt_nextserver(clnt,1);
    if(curserver>=0)
     {
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next server %s",frontierHttpClnt_curservername(clnt));
      continue;      
     }    
    frontier_setErrorMsg(__FILE__,__LINE__,"No more servers/proxies, last error: %s",err_last_buf);
    break;
   }
   
  return ret;
 }

int frontier_getRetrieveZipLevel(FrontierChannel u_channel)
 {
  Channel *chn=(Channel*)u_channel;

  return frontierConfig_getRetrieveZipLevel(chn->cfg);
 }

