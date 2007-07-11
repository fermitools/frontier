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
#include "frontier_client/frontier_log.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_BUF_SIZE	1024*2

static
#ifdef _REENTRANT
__thread
#endif /*_REENTRANT*/
char _frontier_log_msg[LOG_BUF_SIZE];


extern int frontier_log_level;
extern char *frontier_log_file;

static const char *log_desc[]=
 {
  "debug",
  "warn ",
  "error"
 };


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

  if(ret>LOG_BUF_SIZE-2)
    ret=LOG_BUF_SIZE-2;
  
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

