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
#include "fn-internal.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_BUF_SIZE	1024

static char _frontier_error_msg[MSG_BUF_SIZE];
static int errors_into_debugs=0;

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
 
void frontier_vsetErrorMsg(const char *file,int line,const char *fmt,va_list ap)
 {
  int ret,pos;
  
  bzero(_frontier_error_msg,MSG_BUF_SIZE);
  
  ret=snprintf(_frontier_error_msg,MSG_BUF_SIZE,"[%s:%d]: ",file,line);
  pos=ret;
  
  ret+=vsnprintf(_frontier_error_msg+pos,MSG_BUF_SIZE-ret,fmt,ap);
  
  frontier_log(errors_into_debugs?FRONTIER_LOGLEVEL_DEBUG:FRONTIER_LOGLEVEL_ERROR,
  		file,line,"%s",_frontier_error_msg+pos);
 }
 
void frontier_setErrorMsg(const char *file,int line,const char *fmt,...)
 {
  va_list ap;
  va_start(ap,fmt);
  frontier_vsetErrorMsg(file,line,fmt,ap);
  va_end(ap);
 }
 
const char *frontier_getErrorMsg()
 {
  return _frontier_error_msg;
 }

void frontier_turnErrorsIntoDebugs(int value)
 {
  errors_into_debugs=value;
 }
