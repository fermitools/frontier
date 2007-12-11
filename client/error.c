/*
 * frontier client error message support
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
#include <frontier_client/frontier.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MSG_BUF_SIZE	1024
#define LOG_BUF_SIZE	1024

static
#ifdef _REENTRANT
__thread
#endif /*_REENTRANT*/
char _frontier_error_msg[MSG_BUF_SIZE];

static
#ifdef _REENTRANT
__thread
#endif /*_REENTRANT*/
char _frontier_log_msg[LOG_BUF_SIZE];


extern int frontier_log_level;
extern char *frontier_log_file;

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

static const char *log_desc[]=
 {
  "debug",
  "warn ",
  "error"
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


void frontier_log(int level,const char *file,int line,const char *fmt,...)
 {
  int ret;
  int fd;
  va_list ap;
  
  if(level<frontier_log_level) return;
  // To make sure that log_desc is not out of boundaries
  if(level<0) level=0;
  if(level>2) level=2;
  
  ret=snprintf(_frontier_log_msg,LOG_BUF_SIZE-1,"%s [%s:%d]: ",log_desc[level],file,line);
  
  va_start(ap,fmt);
  ret+=vsnprintf(_frontier_log_msg+ret,LOG_BUF_SIZE-ret-1,fmt,ap);
  va_end(ap);
  
  _frontier_log_msg[ret]='\n';
  _frontier_log_msg[ret+1]=0;
  
  if(!frontier_log_file)
   {
    write(1,_frontier_log_msg,ret+1);
    fsync(1);
    return;
   }
  fd=open(frontier_log_file,O_CREAT|O_APPEND|O_WRONLY,0644);
  if(fd<0) return;
  write(fd,_frontier_log_msg,ret+1);
  close(fd);
 }

