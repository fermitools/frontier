/*
 * frontier client C API implementation
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
#include "fn-zlib.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <fcntl.h>
#include <openssl/bio.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

int frontier_log_level;
char *frontier_log_file;
int frontier_log_dup = 0;
pid_t frontier_pid;
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
// buf should be a space at least 26 bytes long, according to man ctime_r
char *frontier_str_now(char *buf)
 {
  time_t now=time(NULL);
  char *cnow=ctime_r(&now,buf);
  // eliminate trailing newline
  cnow[strlen(cnow)-1]='\0';
  return cnow;
 }

/* Calculate the X-Frontier-Id value.  Assumes frontier_pid is set.  */
static void set_frontier_id()
 {
  uid_t uid;
  struct passwd *pwent;
  char *appId;
  char *x509Subject;
  char *pwname, *pwgecos;
  
  uid=getuid();
  pwent=getpwuid(uid);
  if(pwent==NULL)
    pwname=pwgecos="pwent_failed";
  else
   {
    pwname=pwent->pw_name;
    pwgecos=pwent->pw_gecos;
   }
  appId=getenv("FRONTIER_ID");
  if(appId==NULL)
    appId=getenv("CMSSW_VERSION");
  if(appId==NULL)
    appId="client";
  x509Subject=getX509Subject();

  snprintf(frontier_id,FRONTIER_ID_SIZE,"%s %s %d %s(%d) %s",appId,frontier_api_version,frontier_pid,pwname,uid,(x509Subject!=NULL)?x509Subject:pwgecos);

  if(x509Subject!=NULL)OPENSSL_free(x509Subject);

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"client id: %s",frontier_id);
 }
  
int frontier_init(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr))
 {
  return(frontier_initdebug(f_mem_alloc,f_mem_free,
		getenv(FRONTIER_ENV_LOG_FILE),getenv(FRONTIER_ENV_LOG_LEVEL)));
 }

int frontier_initdebug(void *(*f_mem_alloc)(size_t size),void (*f_mem_free)(void *ptr),
			const char *logfilename, const char *loglevel)
 {
  if(initialized) return FRONTIER_OK;

  if(!f_mem_alloc) {f_mem_alloc=malloc; f_mem_free=free;}
  if(!f_mem_free) {f_mem_alloc=malloc; f_mem_free=free;}

  frontier_mem_alloc=f_mem_alloc;
  frontier_mem_free=f_mem_free;
    
  frontier_pid=getpid();

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
      frontier_log_file=frontier_str_copy(logfilename);
      if(!frontier_log_init()) 
       {
        printf("Cannot open log file %s. Log is disabled.\n",logfilename);
	frontier_log_level=FRONTIER_LOGLEVEL_NOLOG;
	frontier_mem_free(frontier_log_file);
        frontier_log_file=(char*)0;
       }
     }
   }

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"starting frontier client version %s",frontier_api_version);
  set_frontier_id();

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
  int n,s;
  int longfresh=0;

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
  chn->pid=frontier_pid;
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
   
  n=0;
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
    n++;
   }while(frontierConfig_nextServer(chn->cfg)==0);
   
  if(n==0)
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

  if((n=frontierConfig_getNumBalancedProxies(chn->cfg))>0)
    frontierHttpClnt_setNumBalancedProxies(chn->ht_clnt,n);

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

  // calculate the url suffixes for short and long time-to-live, including
  //  &sec and &freshkey if needed
  if(frontierConfig_getSecured(chn->cfg))
    s=strlen("&sec=sig");
  else
    s=0;
  p=frontierConfig_getFreshkey(chn->cfg);
  n=strlen(p);
  if(strncmp(p,"long",4)==0)
   {
    longfresh=1;
    p+=4;
    n-=4;
   }
  if(n>0)
    n+=strlen("&freshkey=");
  chn->ttlshort_suffix=(char *)frontier_mem_alloc(s+strlen("&ttl=short")+n+1);
  chn->ttllong_suffix=(char *)frontier_mem_alloc(s+(longfresh?n:0)+1);
  chn->ttlforever_suffix=(char *)frontier_mem_alloc(s+strlen("&ttl=forever")+1);
  if(!chn->ttlshort_suffix||!chn->ttllong_suffix||!chn->ttlforever_suffix)
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    channel_delete(chn);    
    return (void*)0;
   }
  *chn->ttlshort_suffix='\0';
  *chn->ttllong_suffix='\0';
  *chn->ttlforever_suffix='\0';
  if(s>0)
   {
    strcat(chn->ttlshort_suffix,"&sec=sig");
    strcat(chn->ttllong_suffix,"&sec=sig");
    strcat(chn->ttlforever_suffix,"&sec=sig");
   }
  strcat(chn->ttlshort_suffix,"&ttl=short");
  // long is the default ttl so don't need to add anything to it
  strcat(chn->ttlforever_suffix,"&ttl=forever");
  if(n>0)
   {
    strcat(chn->ttlshort_suffix,"&freshkey=");
    strcat(chn->ttlshort_suffix,p);
    if(longfresh)
     {
      strcat(chn->ttllong_suffix,"&freshkey=");
      strcat(chn->ttllong_suffix,p);
     }
    // freshkey never applies to the forever time-to-live
   }

  frontierHttpClnt_setConnectTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getConnectTimeoutSecs(chn->cfg));
  frontierHttpClnt_setReadTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getReadTimeoutSecs(chn->cfg));
  frontierHttpClnt_setWriteTimeoutSecs(chn->ht_clnt,
  		frontierConfig_getWriteTimeoutSecs(chn->cfg));

  chn->ttl=2; // default time-to-live is "long"
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
  int i;
  char nowbuf[26];
  if(!chn) return;
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"closing chan %d at %s",chn->seqnum,frontier_str_now(nowbuf));
  frontierHttpClnt_delete(chn->ht_clnt);
  if(chn->resp)frontierResponse_delete(chn->resp);
  frontierConfig_delete(chn->cfg);
  if(chn->client_cache_buf)frontier_mem_free(chn->client_cache_buf);
  if(chn->ttlshort_suffix)frontier_mem_free(chn->ttlshort_suffix);
  if(chn->ttllong_suffix)frontier_mem_free(chn->ttllong_suffix);
  if(chn->ttlforever_suffix)frontier_mem_free(chn->ttlforever_suffix);
  for(i=0;i<FRONTIER_MAX_SERVERN;i++)
    if(chn->serverrsakey[i])
      RSA_free((RSA *)chn->serverrsakey[i]);
  frontier_mem_free(chn);
  fn_gzip_cleanup();
  frontier_log_close();
 }


// This function is called by gcc when unloading the shared library
__attribute__ ((destructor)) 
static void frontier_fini()
 {
  EVP_cleanup();
  CRYPTO_cleanup_all_ex_data();
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

 
// 1: short time
// 2: long time
// 3: forever
void frontier_setTimeToLive(FrontierChannel u_channel,int ttl)
 {
  Channel *chn=(Channel*)u_channel;  
  if((ttl<1)||(ttl>3))
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"ignoring bad value of TimeToLive %d on chan %d",ttl,chn->seqnum);
  else if(chn->ttl!=ttl)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"changing chan %d ttl flag to %d",chn->seqnum,ttl);
    chn->ttl=ttl;
   }
 }
 

void frontier_setReload(FrontierChannel u_channel,int reload)
 {
  // deprecated interface, leave for backward compatibility
  // 0=long, !0=short
  frontier_setTimeToLive(u_channel,reload?1:2);
 }
 
static int write_data(FrontierResponse *resp,void *buf,int len)
 {
  int ret;
  ret=FrontierResponse_append(resp,buf,len);
  return ret;
 }


static int prepare_channel(Channel *chn,int curserver,const char *params1,const char *params2)
 {
  int ec;

  if(chn->resp) frontierResponse_delete(chn->resp);
  chn->resp=frontierResponse_create(&ec,curserver<0?0:chn->serverrsakey[curserver],params1,params2);
  chn->resp->seqnum=++chn->response_seqnum;
  if(!chn->resp) return ec;
  
  return FRONTIER_OK;
 }
 
static char *vcb_curservername;
static int cert_verify_callback(int ok,X509_STORE_CTX *ctx)
 {
  if (!ok)
    frontier_setErrorMsg(__FILE__,__LINE__, "error verifying server %s cert: %s",vcb_curservername,X509_verify_cert_error_string(ctx->error));
  return ok;
 }
 
static int get_cert(Channel *chn,const char *uri,int curserver)
 {
  int ret;
  int refresh;
  const char *force_reload;
  char *certuri=0;
  char *p,*cert=0;
  int n,len;
  char *servername,*commonname=0;
  BIO *bio=0;
  X509 *x509cert=0;
  X509_STORE *x509store=0;
  X509_STORE_CTX *x509storectx=0;
  X509_VERIFY_PARAM *x509verifyparam=0;
  X509_LOOKUP *x509lookup=0;
  char *x509subject=0;
  GENERAL_NAMES *subjectAltNames=0;
  EVP_PKEY *pubkey=0;
  RSA *rsakey=0;

  ret=prepare_channel(chn,curserver,0,0);
  if(ret) return ret;

  frontierHttpClnt_setPreferIpFamily(chn->ht_clnt,
  		frontierConfig_getPreferIpFamily(chn->cfg));
  force_reload=frontierConfig_getForceReload(chn->cfg);
  refresh=0;
  if((strstr(force_reload,"long")!=0)||(strstr(force_reload,"forever")!=0))
   {
    refresh=1;
    if(strncmp(force_reload,"soft",4)!=0)
      refresh=2;
   }
  frontierHttpClnt_setCacheRefreshFlag(chn->ht_clnt,refresh);
  frontierHttpClnt_setUrlSuffix(chn->ht_clnt,"");
  frontierHttpClnt_setFrontierId(chn->ht_clnt,frontier_id);

  ret=frontierHttpClnt_open(chn->ht_clnt);
  if(ret) goto end;

  p=strstr(uri,"/type=");
  if(p==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"cannot find /type= in URI: %s",uri);
    return FRONTIER_EIARG;
   }
  len=p-uri;
#define CERTURISTR "/type=cert_request:1&encoding=pem"
  certuri=frontier_mem_alloc(len+sizeof(CERTURISTR));
  if(!certuri) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  strncpy(certuri,uri,len);
  strcpy(certuri+len,CERTURISTR);
#undef CERTURISTR
  
  ret=frontierHttpClnt_get(chn->ht_clnt,certuri);
  if(ret) goto end;

  servername=frontierHttpClnt_curservername(chn->ht_clnt);
  if((p=strchr(servername,'['))!=0)
    *p='\0';

  len=frontierHttpClnt_getContentLength(chn->ht_clnt);
  if(len<=0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"no content length from server cert request to server %s",servername);
    ret=FRONTIER_ESERVER;
    goto end;
   }

  cert=frontier_mem_alloc(len+1);
  if(!cert) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}

  n=0;
  while(n<len)
   {
    ret=frontierHttpClnt_read(chn->ht_clnt,cert+n,len-n);
    if(ret<=0)
     {
      if(ret==0)
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"server cert from %s too short: expected %d bytes, got %d",servername,len,n);
	ret=FRONTIER_ESERVER;
       }
      goto end;
     }
    n+=ret;
   }
  cert[len]='\0';

  if(strncmp(cert,"-----BEGIN ",11)!=0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"server cert from %s bad response",servername);
    ret=FRONTIER_ESERVER;
    goto end;
   }

  bio=BIO_new_mem_buf(cert,len);
  if(!bio) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  x509cert=PEM_read_bio_X509(bio,0,0,0);
  if(!x509cert)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"error reading x509 certificate from server %s",servername);
    ret=FRONTIER_ESERVER;
    goto end;
   }

  OpenSSL_add_all_algorithms();
  x509store=X509_STORE_new();
  x509verifyparam=X509_VERIFY_PARAM_new();
  X509_VERIFY_PARAM_set_flags(x509verifyparam,X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);
  X509_STORE_set1_param(x509store,x509verifyparam);
  X509_STORE_set_verify_cb_func(x509store,cert_verify_callback);
  x509lookup=X509_STORE_add_lookup(x509store,X509_LOOKUP_hash_dir());
  if(!x509lookup) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  if(!X509_LOOKUP_add_dir(x509lookup,frontierConfig_getCAPath(chn->cfg),X509_FILETYPE_PEM))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"error adding %s as x509 lookup dir",frontierConfig_getCAPath(chn->cfg));
    ret=FRONTIER_ECFG;
    goto end;
   }

  x509storectx=X509_STORE_CTX_new();
  if(!x509storectx) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  if(!X509_STORE_CTX_init(x509storectx,x509store,x509cert,0)) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  x509subject=X509_NAME_oneline(X509_get_subject_name(x509cert),0,0);
  if(!x509subject) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);goto end;}
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"verifying server certificate %s",x509subject);

  // Have to put server name in a static variable for the error, unfortunately,
  //  because there's no way to pass a value to the callback. 
  vcb_curservername=servername;
  if(!X509_verify_cert(x509storectx))
   {
    // error message is set inside callback
    ret=FRONTIER_ESERVER;
    goto end;
   }

  if((p=strstr(x509subject,"/CN="))!=0)
   {
    commonname=p+4;
    if((p=strchr(commonname,'/'))!=0)
      *p='\0';
   }
  if((commonname!=0)&&(strcmp(commonname,servername)==0))
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"servername %s matched CN",servername);
  else
   {
    int numalts,i;
    subjectAltNames=(GENERAL_NAMES *)X509_get_ext_d2i(x509cert,NID_subject_alt_name,0,0);
    if(!subjectAltNames)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"server %s name does not match cert common name %s",servername,commonname?commonname:"none");
      ret=FRONTIER_ESERVER;
      goto end;
     }
    numalts=sk_GENERAL_NAME_num(subjectAltNames);
    for(i=0;i<numalts;i++)
     {
      GENERAL_NAME *altname=sk_GENERAL_NAME_value(subjectAltNames,i);
      if(altname->type==GEN_DNS)
       {
	unsigned char *dns;
	ASN1_STRING_to_UTF8(&dns,altname->d.dNSName);
	if(strcmp((char *)dns,servername)==0)
	 {
	  OPENSSL_free(dns);
	  break;
	 }
	frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"cert alternative DNS name %s didn't match server name %s",dns,servername);
	OPENSSL_free(dns);
       }
     }
    if(i==numalts)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"server %s name does not match cert common name %s nor any of the alternative DNS subject names",servername,commonname?commonname:"none");
      ret=FRONTIER_ESERVER;
      goto end;
     }
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"server name %s matched alternative DNS subject name",servername);
   }

  pubkey=X509_get_pubkey(x509cert);
  if(!pubkey)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"error extracting public key from server %s cert",servername);
    ret=FRONTIER_ESERVER;
    goto end;
   }

  rsakey=EVP_PKEY_get1_RSA(pubkey);
  if(!rsakey)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"error extracting rsa key from server %s cert",servername);
    ret=FRONTIER_ESERVER;
    goto end;
   }
  
  ret=FRONTIER_OK;
  chn->serverrsakey[curserver]=rsakey;

end:
  frontierHttpClnt_close(chn->ht_clnt);
  if(certuri)frontier_mem_free(certuri);
  if(x509store)X509_STORE_free(x509store);
  // X509_STORE_free does X509_LOOKUP_free
  if(x509storectx)X509_STORE_CTX_free(x509storectx);
  if(x509verifyparam)X509_VERIFY_PARAM_free(x509verifyparam);
  if(pubkey)EVP_PKEY_free(pubkey);
  if(x509cert)X509_free(x509cert);
  if(bio)BIO_free(bio);
  if(cert)frontier_mem_free(cert);
  if(x509subject)OPENSSL_free(x509subject);
  if(subjectAltNames)sk_GENERAL_NAME_pop_free(subjectAltNames,GENERAL_NAME_free);
  return ret;
 }

static int get_data(Channel *chn,const char *uri,const char *body,int curserver)
 {
  int ret=FRONTIER_OK;
  char buf[8192];
  const char *force_reload;
  int force_ttl;
  char *url_suffix;
  char *uri_copy,*data_copy;
  int uri_len;
  int refresh,maxage;
  int client_cache_bufsize;
  fn_hashval *hashval;
  char statbuf[128];

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }

  refresh=chn->refresh;
  maxage=frontierConfig_getMaxAgeSecs(chn->cfg);
  if(refresh)
   {
    // refresh requested by server in last response
    if(refresh==1)
     {
      // soft refresh; chn->max_age contains the max cache age to request
      if((maxage<0)||(maxage>chn->max_age))
	maxage=chn->max_age;
      refresh=0; // because httpclient treats a refresh==1 as maxage==0
     }
    // else refresh==2, fall through to do a hard refresh
   }
  else
   {
    // refresh not requested by server, see if need to force a refresh
    force_reload=frontierConfig_getForceReload(chn->cfg);
    if(strcmp(force_reload,"none")!=0)
     {
      force_ttl=0;
      if(strstr(force_reload,"short")!=0)
	force_ttl=1;
      else if(strstr(force_reload,"long")!=0)
	force_ttl=2;
      else if(strstr(force_reload,"forever")!=0)
	force_ttl=3;
      if(force_ttl&&(chn->ttl<=force_ttl))
       {
	refresh=1;
	if(strncmp(force_reload,"soft",4)!=0)
	 {
	  // hard refresh if force_reload doesn't start with "soft"
	  refresh=2;
	 }
       }
     }
   }
  frontierHttpClnt_setCacheRefreshFlag(chn->ht_clnt,refresh);
  frontierHttpClnt_setCacheMaxAgeSecs(chn->ht_clnt,maxage);
  frontierHttpClnt_setPreferIpFamily(chn->ht_clnt,
  		frontierConfig_getPreferIpFamily(chn->cfg));
  if(chn->ttl==1)
    url_suffix=chn->ttlshort_suffix;
  else if(chn->ttl==3)
    url_suffix=chn->ttlforever_suffix;
  else
    url_suffix=chn->ttllong_suffix;
  frontierHttpClnt_setUrlSuffix(chn->ht_clnt,url_suffix);
  frontierHttpClnt_setFrontierId(chn->ht_clnt,frontier_id);

  ret=prepare_channel(chn,curserver,uri,url_suffix);
  if(ret) return ret;
  
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
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"result too large for client cache");
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
  fn_hashval *hashval;
  FrontierHttpClnt *clnt;
  char err_last_buf[ERR_LAST_BUF_SIZE];
  int curproxy,curserver;
  int have_reset_serverlist=0;
  pid_t pid;
  char nowbuf[26];

  if((pid=getpid())!=frontier_pid)
   {
     pid_t oldpid;
     // process must have forked
     frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"process id changed to %d",(int)pid);
     oldpid=frontier_pid;
     frontier_pid=pid;
     // switch to new log if it includes %P in the name
     frontier_log_close();
     frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"process id changed from %d",(int)oldpid);
     // re-set id to use new pid
     set_frontier_id();
   }
  if(pid!=chn->pid)
   {
     frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"dropping any chan %d persisted connection because process id changed",chn->seqnum);
     chn->pid=pid;
     // drop the socket because it is shared between parent and child
     frontierHttpClnt_drop(chn->ht_clnt);
   }

  if(!chn) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"wrong channel");
    return FRONTIER_EIARG;
   }
  
  if(!body&&(chn->client_cache_maxsize>0))
   {
    if((hashval=fn_hashtable_search(chn->client_cache->table,(char *)uri))!=0)
     {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"HIT in %s client cache, skipping contacting server",chn->client_cache->servlet);
      ret=prepare_channel(chn,-1,0,0);
      if(ret) return ret;
      return write_data(chn->resp,hashval->data,hashval->len);
     }
   }
  
  clnt=chn->ht_clnt;

  frontierHttpClnt_resetwhenold(clnt);
  curproxy=frontierHttpClnt_shuffleproxygroup(clnt);
  curserver=frontierHttpClnt_shuffleservergroup(clnt);
  chn->refresh=0;
  bzero(err_last_buf,ERR_LAST_BUF_SIZE);
  
  while(1)
   {
    // because this is a retry loop, transfer errors aren't necessarily errors
    frontier_turnErrorsIntoDebugs(1);

    ret=FRONTIER_OK;
    if(frontierConfig_getSecured(chn->cfg)&&(chn->serverrsakey[curserver]==0))
     {
      // don't yet have the server certificate, get that first
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"getting certificate on chan %d at %s",chn->seqnum,frontier_str_now(nowbuf));
      ret=get_cert(chn,uri,curserver);
     }
    if(ret==FRONTIER_OK)
     {
      frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"querying on chan %d at %s",chn->seqnum,frontier_str_now(nowbuf));
      ret=get_data(chn,uri,body,curserver);    
      if(ret==FRONTIER_OK) 
       {
	ret=frontierResponse_finalize(chn->resp);
	frontier_turnErrorsIntoDebugs(0);
	frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"chan %d response %d finished at %s",chn->seqnum,chn->resp->seqnum,frontier_str_now(nowbuf));
       }
     }

    if(ret!=FRONTIER_OK)
     {
      frontier_turnErrorsIntoDebugs(0);

      snprintf(err_last_buf,ERR_LAST_BUF_SIZE,"Request %d on chan %d failed at %s: %d %s",chn->resp->seqnum,chn->seqnum,frontier_str_now(nowbuf),ret,frontier_getErrorMsg());
      if((ret==FRONTIER_EMEM)||(ret==FRONTIER_EIARG)||(ret==FRONTIER_ECFG))
       {
	frontier_setErrorMsg(__FILE__,__LINE__,err_last_buf);
	break;
       }

      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,err_last_buf);
     }
    
    /* The retry strategy is unfortunately quite complicated.  It is
	documented in detail at
	 https://twiki.cern.ch/twiki/bin/view/Frontier/ClientRetryStrategy
    */

    if((!chn->refresh)&&((chn->resp->max_age>0)||(ret==FRONTIER_EPROTO)))
     {
      // try soft refresh on same proxy or server if age is old enough;
      int age,max_age;
      max_age=chn->resp->max_age;
      if(max_age<=0)
        max_age=FRONTIER_MAX_EPROTOAGE;
      age=frontierHttpClnt_getCacheAgeSecs(clnt);
      if(age>max_age)
       {
        chn->refresh=1;
	chn->max_age=max_age;
	if(curproxy>=0)
	 {
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying max cache age %d on proxy %s and server %s",max_age,frontierHttpClnt_curproxyname(clnt),frontierHttpClnt_curservername(clnt));
	  continue;
	 }
	frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying max cache age %d on direct connect to server %s",max_age,frontierHttpClnt_curservername(clnt));
	continue;
       }
     }

    if(ret==FRONTIER_OK)
      break;

    if((ret==FRONTIER_EPROTO)&&(chn->refresh==1))
     {
      // try hard refresh from same proxy or server
      // this is needed to clear errors that have a Last-Modified time
      //  that does not change
      chn->refresh=2;
      if(curproxy>=0)
       {
	frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying hard refresh on proxy %s and server %s",frontierHttpClnt_curproxyname(clnt),frontierHttpClnt_curservername(clnt));
	continue;
       }
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying hard refresh on direct connect to server %s",frontierHttpClnt_curservername(clnt));
      continue;
     }

    chn->refresh=0;

    if((curproxy>=0)&&(ret!=FRONTIER_ESERVER))
     {
      int stay_in_proxygroup=frontierHttpClnt_usinglastproxyingroup(clnt);
      if(have_reset_serverlist&&stay_in_proxygroup)
       {
	// This is to avoid a potential situation of alternating
	//  between server list resets and proxy group resets
        have_reset_serverlist=0;
	stay_in_proxygroup=0;
       }
      if((ret!=FRONTIER_ECONNECT)&&stay_in_proxygroup)
       {
	// At the end of the proxy group, so if there's another server then
	//   use it back at the beginning of the current proxy group.
	// We can't tell if the problem was on the server or the proxy
	//   so don't mark the current server as having had an error.
	curserver=frontierHttpClnt_nextserver(clnt,0);
	if(curserver>=0)
	 {
	  int newproxy=frontierHttpClnt_resetproxygroup(clnt);
	  if(newproxy!=curproxy)
	   {
	    curproxy=newproxy;
	    frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next server %s with first proxy %s in proxy group",
	  	frontierHttpClnt_curservername(clnt),frontierHttpClnt_curproxyname(clnt));
	    continue;
	   }
	  // there's only one proxy
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next server %s with same proxy %s",
	  	frontierHttpClnt_curservername(clnt),frontierHttpClnt_curproxyname(clnt));
	  continue;
	 }
	// no more servers so move on to the next proxy group with the
	// first server
	curserver=frontierHttpClnt_resetserverlist(clnt);
	curproxy=frontierHttpClnt_nextproxy(clnt,0);
	if(curproxy>=0)
	 {
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying first server %s with proxy %s in next group",
	      frontierHttpClnt_curservername(clnt),frontierHttpClnt_curproxyname(clnt));
	  continue;
	 }
	// no more proxies, time for direct connect
       }
      else
       {
	// select another proxy regardless of group
	curproxy=frontierHttpClnt_nextproxy(clnt,(ret==FRONTIER_ECONNECT));
	if(curproxy>=0)
	 {
	  frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next proxy %s with same server %s",
	  	frontierHttpClnt_curproxyname(clnt),frontierHttpClnt_curservername(clnt));
	  continue;
	 }
	// no more proxies, try direct with all servers
	curserver=frontierHttpClnt_resetserverlist(clnt);
       }
trydirectconnect:
      if(!frontierConfig_getFailoverToServer(chn->cfg))
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"No more proxies. Last error was: %s",err_last_buf);
	break;
       }
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying direct connect to server %s",frontierHttpClnt_curservername(clnt));
      continue;
     }

    // advance to the next server, either because we're out of proxies 
    //  or it was a server error
    curserver=frontierHttpClnt_nextserver(clnt,1);
    if(curserver>=0)
     {
      frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying next server %s",frontierHttpClnt_curservername(clnt));
      continue;      
     }

    // out of servers
    if((curproxy>=0)&&(ret==FRONTIER_ESERVER))
     {
      // even though it was a server error on the last server, there's still
      //  more proxies so it could have really been a proxy error; try again
      //  at the beginning of the server list and advance the proxy list
      curserver=frontierHttpClnt_resetserverlist(clnt);
      have_reset_serverlist=1;
      curproxy=frontierHttpClnt_nextproxy(clnt,0);
      if(curproxy>=0)
       {
	frontier_log(FRONTIER_LOGLEVEL_WARNING,__FILE__,__LINE__,"Trying again first server %s with proxy %s",
	  frontierHttpClnt_curservername(clnt),frontierHttpClnt_curproxyname(clnt));
	continue;
      }
      // no more proxies either
      goto trydirectconnect;
     }

    frontier_setErrorMsg(__FILE__,__LINE__,"No more servers/proxies. Last error was: %s",err_last_buf);
    break;
   }
   
  if(ret!=FRONTIER_OK) frontierHttpClnt_clear(clnt);

  return ret;
 }

int frontier_getRetrieveZipLevel(FrontierChannel u_channel)
 {
  Channel *chn=(Channel*)u_channel;

  return frontierConfig_getRetrieveZipLevel(chn->cfg);
 }

