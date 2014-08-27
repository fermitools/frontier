/*
 * frontier client response handler
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
#include <string.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"
#include "expat.h"
#include <openssl/ssl.h>
#include <openssl/rsa.h>
#include <openssl/err.h>

static void XMLCALL
xml_cdata(void *userData,const XML_Char *s,int len)
 {
  FrontierResponse *fr=(FrontierResponse*)userData;
  
  frontierPayload_append(fr->payload[fr->payload_num-1],s,len);

  //printf("xml_cdata\n");
 }


static void XMLCALL
xml_startElement(void *userData,const char *name,const char **atts)
 {
  int i;
  FrontierResponse *fr=(FrontierResponse*)userData;

  //printf("xml_start %s\n",name);

  if(strcmp(name,"keepalive")==0)
    fr->keepalives++;
  else if(fr->keepalives!=0)
   {
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"received %d keepalives",fr->keepalives);
    fr->keepalives=0;
   }
  
  if(strcmp(name,"global_error")==0)
   {
    fr->error=FRONTIER_EPROTO;
    frontier_setErrorMsg(__FILE__,__LINE__,"Server has signalled Global Error [%s]",atts[1]);
    return;
   }  

  if(strcmp(name,"frontier")==0)
   {
    char buf[80];
    int i;
    size_t l=0;
    buf[0] = '\0';
    for(i=0;atts[i];i+=2)
     {
      l+=snprintf(&buf[l],sizeof(buf)-l," %s=%s",atts[i],atts[i+1]);
      if (l>=sizeof(buf))
	break;
     }
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"frontier server%s",buf);
    return;
   }

  if(strcmp(name,"payload")==0)
   {
    const char *encoding=NULL;
    fr->payload_num++;
    for(i=0;atts[i];i+=2)
     {
      if(strcmp(atts[i],"encoding")==0)
       {
        encoding=atts[i+1];
	continue;
       }      
     }
    fr->payload[fr->payload_num-1]=frontierPayload_create(encoding,(fr->srv_rsakey!=0),fr->params1,fr->params2);
    fr->p_state=FNTR_WITHIN_PAYLOAD;
    return;
   }

  if(strcmp(name,"data")==0)
   {
    XML_SetCharacterDataHandler(fr->parser,xml_cdata);
    return;
   }   
   
  if(strcmp(name,"quality")==0 && fr->p_state==FNTR_WITHIN_PAYLOAD)
   {
    for(i=0;atts[i];i+=2)
     {
      //printf("attr <%s><%s>\n",atts[i],atts[i+1]);
      fflush(stdout);
      if(strcmp(atts[i],"error")==0)
       {
        fr->payload[fr->payload_num-1]->error_code=atoi(atts[i+1]);
	continue;
       }      
      if(strcmp(atts[i],"message")==0)
       {
        fr->payload[fr->payload_num-1]->error_msg=frontier_str_copy(atts[i+1]);
	continue;
       }            
      if(strcmp(atts[i],"max_age")==0)
       {
	int max_age=atoi(atts[i+1]);
	if((fr->max_age<=0)||(max_age<fr->max_age))
	  fr->max_age=max_age;
	continue;
       }      
      if(strcmp(atts[i],"records")==0)
       {
	//printf("Number of records: %d\n", atoi(atts[i+1]));
        fr->payload[fr->payload_num-1]->nrec=atoi(atts[i+1]);
	continue;
       }
      if(strcmp(atts[i],"full_size")==0)
       {
        fr->payload[fr->payload_num-1]->full_size=atoi(atts[i+1]);
	continue;
       }
      if(strcmp(atts[i],"md5")==0)
       {
        bcopy(atts[i+1],fr->payload[fr->payload_num-1]->srv_md5_str,MD5_DIGEST_LENGTH*2);
	fr->payload[fr->payload_num-1]->srv_md5_str[MD5_DIGEST_LENGTH*2]=0;
	continue;       
       }
      if(strcmp(atts[i],"sig")==0)
       {
	int len=strlen(atts[i+1]);
	unsigned char *srv_sig=frontier_mem_alloc(len*3/4);
	if(srv_sig)
	 {
	  fr->payload[fr->payload_num-1]->srv_sig_len=
	    fn_base64_ascii2bin((unsigned char *)atts[i+1],len,srv_sig,len*3/4);
	  fr->payload[fr->payload_num-1]->srv_sig=srv_sig;
	 }
       }
     }
   }
 }


static void XMLCALL
xml_endElement(void *userData,const char *name)
 {
  int ret;
  
  FrontierResponse *fr=(FrontierResponse*)userData;

  //printf("xml_end %s\n",name);

  if(strcmp(name,"data")==0)
   {
    XML_SetCharacterDataHandler(fr->parser,(void*)0);
    return;
   }

  if(strcmp(name,"payload")==0)
   {
    ret=frontierPayload_finalize(fr->payload[fr->payload_num-1]);    
    fr->p_state=0;
    fr->error=ret;
   }
 }



FrontierResponse *frontierResponse_create(int *ec,void *srv_rsakey,const char *params1,const char *params2)
 {
  FrontierResponse *fr;
  int i;

  fr=frontier_mem_alloc(sizeof(FrontierResponse));
  if(!fr) 
   {
    *ec=FRONTIER_EMEM;
    FRONTIER_MSG(*ec);
    return fr;
   }

  fr->error=0;
  fr->payload_num=0;
  fr->error_payload_ind=-1;
  fr->keepalives=0;
  fr->max_age=-1;
  fr->srv_rsakey=srv_rsakey;
  fr->params1=params1;
  fr->params2=params2;

  fr->parser=XML_ParserCreate(NULL);
  if(!fr->parser)
   {
    frontier_mem_free(fr);
    *ec=FRONTIER_EUNKNOWN;
    frontier_setErrorMsg(__FILE__,__LINE__,"Can not create XML parser instance.");
    return (void*)0;
   }
  XML_SetUserData(fr->parser,fr);
  XML_SetElementHandler(fr->parser,xml_startElement,xml_endElement);

  fr->p_state=0;

  for(i=0;i<FRONTIER_MAX_PAYLOADNUM;i++)
   {
    fr->payload[i]=(void*)0;
   }

  return fr;
 }


void frontierResponse_delete(FrontierResponse *fr)
 {
  int i;

  if(!fr) return;

  if(fr->parser)
   {
    XML_SetUserData(fr->parser,(void*)0);
    XML_ParserFree(fr->parser);
   }

  for(i=0;i<fr->payload_num;i++)
   {
    frontierPayload_delete(fr->payload[i]);
   }
  
  frontier_mem_free(fr);
 }


int FrontierResponse_append(FrontierResponse *fr,char *buf,int len)
 {
  if(XML_Parse(fr->parser,buf,len,0)==XML_STATUS_ERROR) 
   {
    int xml_err=XML_GetErrorCode(fr->parser);
    frontier_setErrorMsg(__FILE__,__LINE__,"XML parse error %d:%s at line %d",xml_err,XML_ErrorString(xml_err),XML_GetCurrentLineNumber(fr->parser));
    return FRONTIER_EPROTO;
   }
   
  if(fr->error)
   {
    return fr->error;
   }
   
  return FRONTIER_OK;
 }


int frontierResponse_finalize(FrontierResponse *fr)
 {
  int i;
  unsigned char *digest;

  if(XML_Parse(fr->parser,"",0,1)==XML_STATUS_ERROR) 
   {
    int xml_err=XML_GetErrorCode(fr->parser);
    frontier_setErrorMsg(__FILE__,__LINE__,"XML parse error %d:%s at line %d",xml_err,XML_ErrorString(xml_err),XML_GetCurrentLineNumber(fr->parser));
    return FRONTIER_EPROTO;
   }

  XML_SetUserData(fr->parser,(void*)0);
  XML_ParserFree(fr->parser);
  fr->parser=(void*)0;
  
  for(i=0;i<fr->payload_num;i++)
   {
    //printf("%d r:[%s] l:[%s]\n",i,fr->payload[i]->srv_md5_str);
    frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Payload[%d] error %d error code %d",(i+1),fr->payload[i]->error,fr->payload[i]->error_code);
    if(fr->payload[i]->error) 
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d] got error %d",(i+1),fr->payload[i]->error);
      return fr->payload[i]->error;
     }    
    if(fr->payload[i]->error_code) 
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d] has error code %d",(i+1),fr->payload[i]->error_code);
      return FRONTIER_EPROTO;
     }

    digest=fr->payload[i]->digest;

    if(fr->srv_rsakey!=0)
     {
      unsigned char *sha256_buf=0;
      int ret=FRONTIER_OK;
      int siglen=fr->payload[i]->srv_sig_len;
      int declen;

      if(siglen==0)
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d]: no signature found",i+1);
	return FRONTIER_EPROTO;
       }

      // If rsa decrypt fails, the output can be as big as the input.
      // If it succeeds, the length should be SHA256_DIGEST_LENGTH.
      // So malloc a buffer of the bigger size just in case
      sha256_buf=frontier_mem_alloc(siglen);
      if(!sha256_buf) {ret=FRONTIER_EMEM;FRONTIER_MSG(ret);return ret;};
      declen=RSA_public_decrypt(siglen,fr->payload[i]->srv_sig,sha256_buf,
      			(RSA *)fr->srv_rsakey,RSA_PKCS1_PADDING);
      if(declen==-1)
       {
	SSL_load_error_strings();
	frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d]: error decrypting signature: %s",i+1,ERR_error_string(ERR_get_error(),0));
	ERR_free_strings();
	ret=FRONTIER_EPROTO;
	goto endsigned;
       }
      if(declen!=SHA256_DIGEST_LENGTH)
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d]: decrypted signature wrong length; expect %d, got %d",i+1,SHA256_DIGEST_LENGTH,declen);
	ret=FRONTIER_EPROTO;
	goto endsigned;
       }

      if(memcmp(digest,sha256_buf,declen)==0)
        frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"Payload[%d] signature passed",i+1);
      else
       {
        char digest_str[SHA256_DIGEST_LENGTH*2+1];
	char sha256_str[SHA256_DIGEST_LENGTH*2+1];
        for(i=0;i<SHA256_DIGEST_LENGTH;i++)
	 {
	  snprintf(&digest_str[i*2],3,"%02x",digest[i]);
	  snprintf(&sha256_str[i*2],3,"%02x",sha256_buf[i]);
	 }
	if(strcmp(digest_str,sha256_str)!=0) 
	 {
	  frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d]: signature hash mismatch: server [%s], local [%s]",(i+1),sha256_str,digest_str);
	  ret=FRONTIER_EPROTO;
	  goto endsigned;
	 }
       }
endsigned:
      if(sha256_buf)frontier_mem_free(sha256_buf);
      if(ret!=FRONTIER_OK)return ret;
     }
    else
     {
      char md5_str[sizeof(fr->payload[0]->srv_md5_str)];
      char *srv_md5_str;

      // this variable is set here to avoid a compiler bug; moving this to
      //  just before the strncmp below (or eliminating the variable) loses
      // the upper bits & causes a seg fault, at least on gcc 4.1.2
      srv_md5_str=fr->payload[i]->srv_md5_str;

      bzero(md5_str,sizeof(md5_str));
      // convert the binary md5 characters into printable
      for(i=0;i<MD5_DIGEST_LENGTH;i++)
	snprintf(&md5_str[i*2],3,"%02x",digest[i]);

      if(strcmp(srv_md5_str,md5_str)!=0) 
       {
	frontier_setErrorMsg(__FILE__,__LINE__,"Payload[%d]: MD5 hash mismatch: server [%s], local [%s]",(i+1),srv_md5_str,md5_str);
	return FRONTIER_EPROTO;
       }
     }
   }

  return FRONTIER_OK;
 }
