/*
 * frontier client error message handling
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
#include "frontier_client/frontier_error.h"
#include "frontier_client/frontier_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_BUF_SIZE	1024

static
#ifdef _REENTRANT
__thread
#endif /*_REENTRANT*/
char _frontier_error_msg[MSG_BUF_SIZE];

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *p);


static const char *fn_errs[]=
 {
  "Ok",
  "Invalid argument passed",			/*FRONTIER_EIARG*/
  "mem_alloc failed",				/*FRONTIER_EMEM*/
  "config error",				/*FRONTIER_ECFG*/
  "system error",				/*FRONTIER_ESYS*/
  "unknown error",				/*FRONTIER_EUNKNOWN*/
  "error while communicating over network",	/*FRONTIER_ENETWORK*/
  "protocol error (bad response, etc)",		/*FRONTIER_EPROTO*/
  NULL
 };

const char *frontier_get_err_desc(int err_num)
 {
  int i;
  if(err_num>0) return "unknown";
  i=0;
  while(fn_errs[i])
   {
    if(i==(-err_num)) return fn_errs[i];
    ++i;
   }
  return "unknown_2";
 }
 
void frontier_setErrorMsg(const char *file,int line,const char *fmt,...)
 {
  int ret,pos;
  va_list ap;
  
  bzero(_frontier_error_msg,MSG_BUF_SIZE);
  
  ret=snprintf(_frontier_error_msg,MSG_BUF_SIZE,"[%s:%d]: ",file,line);
  pos=ret;
  
  va_start(ap,fmt);
  ret+=vsnprintf(_frontier_error_msg+pos,MSG_BUF_SIZE-ret,fmt,ap);
  va_end(ap);
  
  frontier_log(FRONTIER_LOGLEVEL_ERROR,file,line,"%s",_frontier_error_msg+pos);
 }
 
 
const char *frontier_getErrorMsg()
 {
  return _frontier_error_msg;
 }

