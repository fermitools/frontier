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
#include <string.h>
#include <frontier.h>
#include "fn-internal.h"
#include "expat.h"

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);

static char *frontier_strdup(const char *s)
 {
  char *p;
  int len=strlen(s);
  
  p=frontier_mem_alloc(len+1);
  bcopy(s,p,len);
  p[len]=0;
  return p;
 }
 

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

  if(strcmp(name,"payload")==0)
   {
    fr->payload_num++;
    fr->payload[fr->payload_num-1]=frontierPayload_create();
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
        fr->payload[fr->payload_num-1]->error_msg=frontier_strdup(atts[i+1]);
	continue;
       }            
      if(strcmp(atts[i],"records")==0)
       {
        fr->payload[fr->payload_num-1]->nrec=atoi(atts[i+1]);
	continue;
       }
      if(strcmp(atts[i],"md5")==0)
       {
        bcopy(atts[i+1],fr->payload[fr->payload_num-1]->srv_md5_str,32);
	fr->payload[fr->payload_num-1]->srv_md5_str[32]=0;
	continue;       
       }
     }
   }
 }


static void XMLCALL
xml_endElement(void *userData,const char *name)
 {
  FrontierResponse *fr=(FrontierResponse*)userData;

  //printf("xml_end %s\n",name);

  if(strcmp(name,"data")==0)
   {
    XML_SetCharacterDataHandler(fr->parser,(void*)0);
    return;
   }

  if(strcmp(name,"payload")==0)
   {
    frontierPayload_finalize(fr->payload[fr->payload_num-1]);    
    fr->p_state=0;
   }
 }



FrontierResponse *frontierResponse_create()
 {
  FrontierResponse *fr;
  int i;

  fr=frontier_mem_alloc(sizeof(FrontierResponse));
  if(!fr) return fr;

  fr->payload_num=0;
  fr->error=0;
  fr->error_code=0;
  fr->error_payload_ind=-1;

  fr->parser=XML_ParserCreate(NULL);
  if(!fr->parser)
   {
    frontier_mem_free(fr);
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
    fr->error=FRONTIER_XMLPARSE;
    fr->error_code=XML_GetErrorCode(fr->parser);
    printf("%s at line %d\n",XML_ErrorString(fr->error_code),XML_GetCurrentLineNumber(fr->parser));
    return FRONTIER_XMLPARSE;
   }
  return FRONTIER_OK;
 }



int frontierResponse_finalize(FrontierResponse *fr)
 {
  int i;
  
  if(XML_Parse(fr->parser,"",0,1)==XML_STATUS_ERROR) 
   {
    fr->error=FRONTIER_XMLPARSE;
    fr->error_code=XML_GetErrorCode(fr->parser);
    printf("%s at line %d\n",XML_ErrorString(fr->error_code),XML_GetCurrentLineNumber(fr->parser));
    return FRONTIER_XMLPARSE;
   }

  XML_SetUserData(fr->parser,(void*)0);
  XML_ParserFree(fr->parser);
  fr->parser=(void*)0;
  
  for(i=0;i<fr->payload_num;i++)
   {
    //printf("%d r:[%s] l:[%s]\n",i,fr->payload[i]->srv_md5_str,fr->payload[i]->md5_str);
    //printf("%d\n",fr->payload[i]->error_code);
    if(fr->payload[i]->error_code) return FRONTIER_EPAYLOAD;
    if(strncmp(fr->payload[i]->srv_md5_str,fr->payload[i]->md5_str,32)) return FRONTIER_EMD5;
   }

  return FRONTIER_OK;
 }







