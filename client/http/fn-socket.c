/*
 * frontier client socket interface
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
 
#include <fn-htclient.h>
 
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

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
 

int frontier_connect(int s,const struct sockaddr *serv_addr,socklen_t addrlen,int timeoutsecs)
 {
  int ret;
  fd_set wfds;
  struct timeval tv;
  int val;
  socklen_t s_len;
  struct sockaddr_in *sin=(struct sockaddr_in*)(serv_addr);
  
  ret=connect(s,serv_addr,addrlen);
  if(ret==0) return FRONTIER_OK;

  /* ret<0 below */  
  if(errno!=EINPROGRESS)
   {
    if(errno==ECONNREFUSED || errno==ENETUNREACH)
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"host %s is down or unreachable",inet_ntoa(sin->sin_addr));
      return FRONTIER_ENETWORK;
     }
    else
     {
      frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on connect to %s: %s",errno,inet_ntoa(sin->sin_addr),strerror(errno));
      return FRONTIER_ESYS;     
     }
   }
 
  /* non-blocking connect in progress here */
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connect s=%d addr %s waiting for response",s,inet_ntoa(sin->sin_addr));
  FD_ZERO(&wfds);
  FD_SET(s,&wfds);
  tv.tv_sec=timeoutsecs;
  tv.tv_usec=0;
  do
   {
    ret=select(s+1,NULL,&wfds,NULL,&tv);
   }while((ret<0)&&(errno==EINTR));  /*this loop is to support profiling*/
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on select when connecting to %s: %s",errno,inet_ntoa(sin->sin_addr),strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"connect to %s timed out after %d seconds",inet_ntoa(sin->sin_addr),timeoutsecs);
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
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d on connect to %s: %s",val,inet_ntoa(sin->sin_addr),strerror(val));
    return FRONTIER_ESYS;
   }
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"connected, s=%d .",s);
  return FRONTIER_OK;
 }
 
 
static int socket_write(int s,const char *buf, int len, int timeoutsecs)
 {
  int ret;
  fd_set wfds;
  struct timeval tv;
  
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,"write request %d bytes",len);

  FD_ZERO(&wfds);
  FD_SET(s,&wfds);
  tv.tv_sec=timeoutsecs;
  tv.tv_usec=0;
  ret=select(s+1,NULL,&wfds,NULL,&tv);
  if(ret<0) 
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"system error %d: %s",errno,strerror(errno));
    return FRONTIER_ESYS;
   }
  if(ret==0)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,"write timed out after %d seconds",timeoutsecs);
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

 
int frontier_write(int s,const char *buf, int len, int timeoutsecs)
 {
  int ret;
  int total;
  int repeat;
  
  repeat=3;	// XXX repeat write 3 times (if no error)
  
  total=0;
  while(repeat--)
   {
    ret=socket_write(s,buf+total,len-total,timeoutsecs);
    if(ret<0) return ret;
    total+=ret;
    if(total==len) return total;
   }
   
  frontier_setErrorMsg(__FILE__,__LINE__,"failed to write over network");
  return FRONTIER_ENETWORK;      
 }

 

int frontier_read(int s, char *buf, int size, int timeoutsecs)
 {
  int ret;
  fd_set rfds;
  struct timeval tv;

  FD_ZERO(&rfds);
  FD_SET(s,&rfds);
  tv.tv_sec=timeoutsecs;
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
    frontier_setErrorMsg(__FILE__,__LINE__,"read timed out after %d seconds",timeoutsecs);
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
 
