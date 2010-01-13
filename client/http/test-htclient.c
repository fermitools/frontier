/*
 * frontier client http subset main test program
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <netinet/in.h>
#include <arpa/inet.h>


#define B_SIZE	65536

int main(int argc,char **argv)
 {
  FrontierHttpClnt *clnt;
  int ret;
  char buf[B_SIZE];
  
  if(argc!=5)
   {
    printf("Usage: %s server_url obj_name key_name key_value\n",argv[0]);
    exit(1);
   }
    
  clnt=frontierHttpClnt_create();
  if(!clnt)
   {
    printf("Error %s\n",frontier_getErrorMsg());
    exit(1);
   }
   
  ret=frontierHttpClnt_addServer(clnt,argv[1]);
  if(ret)
   {
    printf("Error %s\n",frontier_getErrorMsg());
    exit(1);
   }

  ret=frontierHttpClnt_addProxy(clnt,"http://edge:8128");
  if(ret)
   {
    printf("Error %s\n",frontier_getErrorMsg());
    exit(1);
   }   

  frontierHttpClnt_setCacheRefreshFlag(clnt,0);
        
  bzero(buf,B_SIZE);
  snprintf(buf,B_SIZE,"Frontier?type=%s:1&encoding=BLOB&%s=%s",argv[2],argv[3],argv[4]);
  printf("Req <%s>\n",buf);   
   
  ret=frontierHttpClnt_open(clnt,buf);
  if(ret)
   {
    printf("Error %s\n",frontier_getErrorMsg());
    exit(1);
   }
   
  bzero(buf,B_SIZE);

  printf("Data:\n");
  while((ret=frontierHttpClnt_read(clnt,buf,B_SIZE-1))>0) {printf("%s",buf); bzero(buf,B_SIZE);}
  printf("End.\n");
  if(ret<0)
   {
    printf("Error %s\n",frontier_getErrorMsg());
    exit(1);
   }      
         
  frontierHttpClnt_delete(clnt); 
   
  exit(0);
 }
