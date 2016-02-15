/*
 * frontier client log message handling
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
#include "frontier_client/frontier_log.h"
#include "fn-internal.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define LOG_BUF_SIZE	1024*2
#define MAX_LOG_PATH_SIZE 1024

static int log_fd=-1;

static const char *log_desc[]=
 {
  "debug",
  "warn ",
  "error"
 };


int frontier_log_init()
 {
  char pathbuf[MAX_LOG_PATH_SIZE];
  char *fname=frontier_log_file;
  char *p;
  int idx;
  if((p=strstr(fname,"%P"))!=NULL)
   {
    // replace the %P with the process id
    idx=p-fname;
    if(idx>MAX_LOG_PATH_SIZE)idx=MAX_LOG_PATH_SIZE;
    strncpy(pathbuf,fname,idx);
    if(idx<MAX_LOG_PATH_SIZE)
      idx+=snprintf(pathbuf+idx,MAX_LOG_PATH_SIZE-idx,"%d",frontier_pid);
    if(idx>MAX_LOG_PATH_SIZE)idx=MAX_LOG_PATH_SIZE;
    strncpy(pathbuf+idx,p+2,MAX_LOG_PATH_SIZE-idx);
    pathbuf[MAX_LOG_PATH_SIZE-1]='\0';
    fname=pathbuf;
   }
  log_fd=open(fname,O_CREAT|O_APPEND|O_WRONLY,0644);
  if(log_fd>=0)
    return 1;
  return 0;
 }

void frontier_vlog(int level,const char *file,int line,const char *fmt,va_list ap)
 {
  int ret;
  char log_msg[LOG_BUF_SIZE];

  if(level<frontier_log_level) return;
  // To make sure that log_desc is not out of boundaries
  if(level<0) level=0;
  if(level>2) level=2;
  
  ret=snprintf(log_msg,LOG_BUF_SIZE-1,"%s [%s:%d]: ",log_desc[level],file,line);
  
  ret+=vsnprintf(log_msg+ret,LOG_BUF_SIZE-ret-1,fmt,ap);

  if(ret>LOG_BUF_SIZE-2)
    ret=LOG_BUF_SIZE-2;
  
  log_msg[ret]='\n';
  log_msg[ret+1]=0;
  
  if(!frontier_log_file||(frontier_log_dup&&(level>=FRONTIER_LOGLEVEL_WARNING)))
   {
    (void)write(1,log_msg,ret+1);
    fsync(1);
    if(!frontier_log_file) return;
   }
  if(log_fd<0)
   {
    (void)frontier_log_init();
    if(log_fd<0) return;
   }
  (void)write(log_fd,log_msg,ret+1);
 }

void frontier_log(int level,const char *file,int line,const char *fmt,...)
 {
  va_list ap;
  va_start(ap,fmt);
  frontier_vlog(level,file,line,fmt,ap);
  va_end(ap);
 }

void frontier_log_close()
 {
  if(log_fd>=0)
   {
    close(log_fd);
    log_fd=-1;
   }
 }

