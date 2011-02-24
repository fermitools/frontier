/*
 * command line tool for getting files from a frontier server
 * 
 * Author: Dave Dykstra
 *
 * $Id$
 *
 * Copyright (c) 2011, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 *
 */
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#include "frontier_client/frontier.h"

void usage()
 {
  fprintf(stderr,"Usage: fn-fileget [-c connect_string] filepath ...\n");
  exit(2);
 }

int main(int argc, char **argv)
 {
  int ec,c,i;
  unsigned long channel;
  FrontierConfig *config;
  char *connectstr="";

  while(1)
   {
    c=getopt(argc,argv,":c:");
    switch(c)
     {
      case 'c':
       connectstr=optarg;
       break;
      case -1:
       break;
      case ':':
       fprintf(stderr,"Missing argument\n");
       usage();
      default:
       fprintf(stderr,"Unrecognized option\n");
       usage();
     }
    if(c==-1)
      break;
   }

  if(optind>=argc)
   {
    fprintf(stderr,"No files requested\n");
    usage();
   }

  if(frontier_init(malloc,free)!=0)
   {
    fprintf(stderr,"Error initializing frontier client: %s\n",frontier_getErrorMsg());
    return 2;
   }
  ec=FRONTIER_OK;
  config=frontierConfig_get(connectstr,"",&ec); 
  if(ec!=FRONTIER_OK)
   {
    fprintf(stderr,"Error getting frontierConfig object: %s\n",frontier_getErrorMsg());
    return 2;
   }
  channel=frontier_createChannel2(config,&ec);
  if(ec!=FRONTIER_OK)
   {
    fprintf(stderr,"Error creating frontier channel: %s\n",frontier_getErrorMsg());
    return 2;
   }

  for(i=optind;i<argc;i++){
    char uribuf[4096];
    FrontierRSBlob *frsb;
    int fd;
    int n;
    char *p;
    char *localname;

    snprintf(uribuf,sizeof(uribuf)-1,
    	"Frontier/type=frontier_file:1:DEFAULT&encoding=BLOB&p1=%s",argv[i]);
    ec=frontier_getRawData(channel,uribuf);
    if(ec!=FRONTIER_OK)
     {
      fprintf(stderr,"Error getting data for %s: %s\n",argv[i],frontier_getErrorMsg());
      break;
     }
    frsb=frontierRSBlob_open(channel,0,1,&ec);
    if(ec!=FRONTIER_OK)
     {
      fprintf(stderr,"Error opening result blob for %s: %s\n",argv[i],frontier_getErrorMsg());
      break;
     }
    // ignore the result type, will always be an array
    (void)frontierRSBlob_getByte(frsb,&ec);
    if(ec!=FRONTIER_OK)
     {
      fprintf(stderr,"Error getting result type for %s: %s\n",argv[i],frontier_getErrorMsg());
      break;
     }
    n=frontierRSBlob_getInt(frsb,&ec);
    if(ec!=FRONTIER_OK)
     {
      fprintf(stderr,"Error getting result size for %s: %s\n",argv[i],frontier_getErrorMsg());
      break;
     }
    p=frontierRSBlob_getByteArray(frsb,n,&ec);
    if(ec!=FRONTIER_OK)
     {
      fprintf(stderr,"Error getting result data for %s: %s\n",argv[i],frontier_getErrorMsg());
      break;
     }
    localname=strrchr(argv[i],'/');
    if(localname==NULL)
      localname=argv[i];
    else
      localname++;
    fd=open(localname,O_CREAT|O_TRUNC|O_WRONLY,0666);
    if(fd==-1)
     {
      fprintf(stderr,"Error creating %s: %s\n",localname,strerror(errno));
      ec=-1;
      break;
     }
    if(write(fd,p,n)<0)
     {
      fprintf(stderr,"Error writing to %s: %s\n",localname,strerror(errno));
      ec=-1;
      close(fd);
      break;
     }
    close(fd);
    printf("%d bytes written to %s\n",n,localname);
    frontierRSBlob_close(frsb,&ec);
   }

  frontier_closeChannel(channel);
  return (ec==FRONTIER_OK);
 }
