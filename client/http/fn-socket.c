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
 
#include <fn-htclient.h> 
 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

 
int frontier_socket()
 {
  int s;
  int flags;
  
  s=socket(PF_INET,SOCK_STREAM,0);
  if(s<0) return s;
  
  flags=fcntl(s,F_GETFL);
  if(flags<0) {close(s); return flags;}
  
  flags=flags|O_NONBLOCK;
  flags=fcntl(s,F_SETFL,(long)flags);
  if(flags<0) {close(s); return flags;}
  
  return s;
 }
 

int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen)
 {
  int ret;
  fd_set wfds;
  struct timeval tv;
  int val;
  socklen_t s_len;
  
  ret=connect(s,serv_addr,addrlen);
  if(ret<0 && errno==EINPROGRESS)
   {
    //printf("Connect in process. Will select.\n");
    FD_ZERO(&wfds);
    FD_SET(s,&wfds);
    tv.tv_sec=30;
    tv.tv_usec=0;
    ret=select(s+1,NULL,&wfds,NULL,&tv);
    if(ret<0) 
     {
      printf("connect: select() error %d: %s\n",errno,strerror(errno));
      return -1;
     }
    if(ret==0)
     {
      printf("connect: select() timeout\n");
      errno=ETIMEDOUT;
      return -1;
     }
    if(!FD_ISSET(s,&wfds))
     {
      printf("connect: select() unindentified error\n");
      return -1;
     }
    s_len=sizeof(val);
    val=0;
    ret=getsockopt(s,SOL_SOCKET,SO_ERROR,&val,&s_len);
    if(ret<0)
     {
      printf("connect: getsockopt() error %d: %s\n",errno,strerror(errno));
      return -1;
     }
    if(val)
     {
      printf("connect: socket error %d: %s\n",val,strerror(val));
      errno=val;
      return -1;
     }
   }
  return 0;
 }
 
 
int frontier_write(int s,const char *buf, int len)
 {
  int ret;
  
  ret=write(s,buf,len);
  if(ret<0) return ret;
  if(ret!=len)
   {
    printf("write: wrote %d, expected %d\n",ret,len);
    return -1;
   }
  return ret;
 }


int frontier_read(int s, char *buf, int size)
 {
  int ret;
  fd_set rfds;
  struct timeval tv;

  FD_ZERO(&rfds);
  FD_SET(s,&rfds);
  tv.tv_sec=30;
  tv.tv_usec=0;
  ret=select(s+1,&rfds,NULL,NULL,&tv);
  if(ret<0) 
   {
    printf("read: select() error %d: %s\n",errno,strerror(errno));
    return -1;
   }
  if(ret==0)
   {
    printf("read: select() timeout\n");
    errno=ETIMEDOUT;
    return -1;
   }
  if(!FD_ISSET(s,&rfds))
   {
    printf("read: select() unindentified error\n");
    return -1;
   }
   
  ret=read(s,buf,size);
  if(ret<0)
   {
    printf("read: error %d: %s\n",errno,strerror(errno));
    return -1;
   }
  return ret;
 }
 
