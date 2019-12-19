/*
 * frontier client socket interface
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
 
#include <fn-htclient.h>
 
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

// return ascii representation of ipv4 or ipv6 address, for log messages
// also add colon and port number if port number is not zero
// it is stored in a static buffer, overwritten by later calls
char *frontier_ipaddr(const struct sockaddr *serv_addr)
 {
  // addrbuf may contain extra brackets, colon, port number, and
  //   null terminator.  16 will more than cover it.
  static char addrbuf[INET6_ADDRSTRLEN+16];
  in_port_t port;
  char *p;
  if (serv_addr->sa_family == AF_INET6)
   {
    addrbuf[0]='[';
    addrbuf[1]='\0';
    inet_ntop(AF_INET6,&(((struct sockaddr_in6*)serv_addr)->sin6_addr),&addrbuf[1],INET6_ADDRSTRLEN);
    p=strchr(addrbuf,'\0');
    *p++=']';
    *p='\0';
    port=ntohs(((struct sockaddr_in6*)serv_addr)->sin6_port);
   }
  else
   {
    addrbuf[0]='\0';
    inet_ntop(AF_INET,&(((struct sockaddr_in*)serv_addr)->sin_addr),addrbuf,INET_ADDRSTRLEN);
    p=strchr(addrbuf,'\0');
    port=ntohs(((struct sockaddr_in*)serv_addr)->sin_port);
   }
  if(port!=0)
   {
    snprintf(p,&addrbuf[sizeof(addrbuf)-1]-p,":%d",(int)port);
    addrbuf[sizeof(addrbuf)-1]='\0'; // in case of overflow
   }
  return addrbuf;
 }

//static int total_socket=0;

int frontier_socket(sa_family_t family)
 {
  int s;
  int flags;
  
  s=socket(family,SOCK_STREAM,0);
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
 
static int handle_connect_error(const struct sockaddr *serv_addr)
 {
  if(errno==ECONNREFUSED || errno==ENETUNREACH || errno==EHOSTUNREACH)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"network error on connect to %s: %s",frontier_ipaddr(serv_addr),strerror(errno));
    return FRONTIER_ECONNECT;
   }
  else
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on connect to %s: %s",errno,frontier_ipaddr(serv_addr),strerror(errno));
    return FRONTIER_ESYS;     
   }
 }

int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen,int timeoutsecs)
 {
  int ret;
  struct pollfd pfd;
  int val;
  socklen_t s_len;
  
  ret=connect(s,serv_addr,addrlen);
  if(ret==0) return FRONTIER_OK;

  /* ret<0 below */  
  if(errno!=EINPROGRESS)
    return(handle_connect_error(serv_addr));
 
  /* non-blocking connect in progress here */
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connect s=%d addr %s waiting for response",s,frontier_ipaddr(serv_addr));
  pfd.fd=s;
  pfd.events=POLLOUT;
  do
   {
    pfd.revents=0;
    ret=poll(&pfd,1,timeoutsecs*1000);
   }while((ret<0)&&(errno==EINTR));  /*this loop is to support profiling*/
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on poll when connecting to %s: %s",errno,frontier_ipaddr(serv_addr),strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"connect to %s timed out after %d seconds",frontier_ipaddr(serv_addr),timeoutsecs);
    return FRONTIER_ECONNECT;
   }

  s_len=sizeof(val);
  val=0;
  ret=getsockopt(s,SOL_SOCKET,SO_ERROR,&val,&s_len);
  if(ret<0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on getsockopt when connecting to %s: %s",errno,frontier_ipaddr(serv_addr),strerror(errno));
    return FRONTIER_ESYS;
   }
  if(val)
   {
    errno=val;
    return(handle_connect_error(serv_addr));
   }

  if(!(pfd.revents&POLLOUT))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"inconsistent result from connect poll on fd %d",s);
    return FRONTIER_EUNKNOWN;
   }
  
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connected, s=%d .",s);
  return FRONTIER_OK;
 }
 
 
static int socket_write(int s,const char *buf, int len, int timeoutsecs,struct addrinfo *addr)
 {
  int ret;
  struct pollfd pfd;
  
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"write request %d bytes",len);

  pfd.fd=s;
  pfd.events=POLLOUT;
  pfd.revents=0;
  ret=poll(&pfd,1,timeoutsecs*1000);
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on poll: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"write to %s timed out after %d seconds",
      frontier_ipaddr(addr->ai_addr),timeoutsecs);
    return FRONTIER_ENETWORK;
   }

  if(!(pfd.revents&POLLOUT))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"inconsistent result from write poll on fd %d",s);
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

 
int frontier_write(int s,const char *buf, int len, int timeoutsecs,struct addrinfo *addr)
 {
  int ret;
  int total;
  int repeat;
  
  repeat=3;	// XXX repeat write 3 times (if no error)
  
  total=0;
  while(repeat--)
   {
    ret=socket_write(s,buf+total,len-total,timeoutsecs,addr);
    if(ret<0) return ret;
    total+=ret;
    if(total==len) return total;
   }
   
  frontier_setErrorMsg(__FILE__,__LINE__,"failed to write over network");
  return FRONTIER_ENETWORK;      
 }

 

int frontier_read(int s, char *buf, int size, int timeoutsecs,struct addrinfo *addr)
 {
  int ret;
  struct pollfd pfd;

  pfd.fd=s;
  pfd.events=POLLIN;
  do
   {
    pfd.revents=0;
    ret=poll(&pfd,1,timeoutsecs*1000);
   }while((ret<0)&&(errno==EINTR));  /*this loop is to support profiling*/
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on poll: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"read from %s timed out after %d seconds",
      frontier_ipaddr(addr->ai_addr),timeoutsecs);
    return FRONTIER_ENETWORK;
   }

  if(!(pfd.revents&POLLIN))
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"inconsistent result from poll on fd %d",s);
    return FRONTIER_EUNKNOWN;
   }
   
  ret=recv(s,buf,size,0);
  if(ret>=0) return ret;
  
  frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
  return FRONTIER_ESYS;   
 }
 
