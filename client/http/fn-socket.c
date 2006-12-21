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

//static int total_socket=0;
 
int frontier_socket()
 {
  int s;
  int flags;
  
  s=socket(PF_INET,SOCK_STREAM,0);
  if(s<0) goto err;
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"new socket s=%d",s);
  
  flags=fcntl(s,F_GETFL);
  if(flags<0) goto err;
  
  flags=flags|O_NONBLOCK;
  flags=fcntl(s,F_SETFL,(long)flags);
  if(flags<0) goto err;
  goto ok;
  
err:
  if(s>=0) frontier_socket_close(s);
  frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
  return FRONTIER_ESYS;
  
ok:  
  //++total_socket;
  //printf("so %d\n",total_socket);
  return s;
 }
 
 
void frontier_socket_close(int s)
 {
  close(s);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"socket s=%d closed",s);
  //--total_socket;
  //printf("sc %d\n",total_socket);
 } 
 

int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen)
 {
  int ret;
  fd_set wfds;
  struct timeval tv;
  int val;
  socklen_t s_len;
  
  ret=connect(s,serv_addr,addrlen);
  if(ret==0) return FRONTIER_OK;

  /* ret<0 below */  
  if(errno!=EINPROGRESS)
   {
    if(errno==ECONNREFUSED || errno==ENETUNREACH)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"host is down or unreachable");
      return FRONTIER_ENETWORK;
     }
    else
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
      return FRONTIER_ESYS;     
     }
   }
 
  /* non-blocking connect in progress here */
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connect s=%d in progress, preparing for select.",s);
  FD_ZERO(&wfds);
  FD_SET(s,&wfds);
  /* the initial connect timeout should be quite short, to fail over to
     the next if one of the servers is down */
  tv.tv_sec=3;
  tv.tv_usec=0;
  do
   {
    ret=select(s+1,NULL,&wfds,NULL,&tv);
   }while((ret<0)&&(errno==EINTR));  /*this loop is to support profiling*/
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"connect timed out");
    return FRONTIER_ENETWORK;
   }
  
  if(!FD_ISSET(s,&wfds))
   {
    FRONTIER_MSG(FRONTIER_EUNKNOWN);
    return FRONTIER_EUNKNOWN;
   }
  s_len=sizeof(val);
  val=0;
  ret=getsockopt(s,SOL_SOCKET,SO_ERROR,&val,&s_len);
  if(ret<0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(val)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",val,strerror(val));
    return FRONTIER_ESYS;
   }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connected, s=%d .",s);
  return FRONTIER_OK;
 }
 
 
static int socket_write(int s,const char *buf, int len)
 {
  int ret;
  fd_set wfds;
  struct timeval tv;
  
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"write request %d bytes",len);

  FD_ZERO(&wfds);
  FD_SET(s,&wfds);
  /* writes shouldn't take very long unless there is much queued,
     which doesn't happen in the frontier client */
  tv.tv_sec=5;
  tv.tv_usec=0;
  ret=select(s+1,NULL,&wfds,NULL,&tv);
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"write timed out");
    return FRONTIER_ENETWORK;
   }
  if(!FD_ISSET(s,&wfds))
   {
    FRONTIER_MSG(FRONTIER_EUNKNOWN);
    return FRONTIER_EUNKNOWN;
   }
   
  ret=send(s,buf,len,0);
  if(ret>=0) return ret;
  
  if(errno==ECONNRESET)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"connection reset by peer");
    return FRONTIER_ENETWORK;
   }
   
  frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
  return FRONTIER_ESYS;   
 }

 
int frontier_write(int s,const char *buf, int len)
 {
  int ret;
  int total;
  int repeat;
  
  repeat=3;	// XXX repeat write 3 times (if no error)
  
  total=0;
  while(repeat--)
   {
    ret=socket_write(s,buf+total,len-total);
    if(ret<0) return ret;
    total+=ret;
    if(total==len) return total;
   }
   
  frontier_setErrorMsg(__FILE__,__LINE__,"failed to write over network");
  return FRONTIER_ENETWORK;      
 }

 

int frontier_read(int s, char *buf, int size)
 {
  int ret;
  fd_set rfds;
  struct timeval tv;

  FD_ZERO(&rfds);
  FD_SET(s,&rfds);
  /* server should send some data at least every 5 seconds; allow for double */
  tv.tv_sec=10;
  tv.tv_usec=0;
  do
   {
    ret=select(s+1,&rfds,NULL,NULL,&tv);
   }while((ret<0)&&(errno==EINTR));  /*this loop is to support profiling*/
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"read timed out");
    return FRONTIER_ENETWORK;
   }
  if(!FD_ISSET(s,&rfds))
   {
    FRONTIER_MSG(FRONTIER_EUNKNOWN);
    return FRONTIER_EUNKNOWN;
   }
   
  ret=recv(s,buf,size,0);
  if(ret>=0) return ret;
  
  frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
  return FRONTIER_ESYS;   
 }
 
