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
#include <ctype.h>
#include "frontier_client/frontier.h"

#define RESULT_TYPE_ARRAY 6
#define RESULT_TYPE_EOR 7

void usage()
 {
  fprintf(stderr,"Usage: fn-fileget [-c connect_string] [-r|-R] filepath ...\n");
  fprintf(stderr,"  -c connect_string: use given connect_string (default from environment)\n");
  fprintf(stderr,"  -r: request short time-to-live\n");
  fprintf(stderr,"  -R: request forever time-to-live\n");
  exit(2);
 }

char *
doubleurlencode(char *frombuf)
 {
  int c,n;
  char *tobuf=malloc(strlen(frombuf)*5+1);
  char *p=tobuf;
  while((c=*frombuf++)!=0)
   {
    if(isalnum(c)||(c=='/')||(c=='-')||(c=='_')||(c=='.'))
      *p++=c;
    else
     {
      // %25 is the encoding of %
      *p++='%';
      *p++='2';
      *p++='5';
      n=(c>>4);
      if(n<10)
        *p++=n+'0';
      else
        *p++=n-10+'a';
      n=(c&0xf);
      if(n<10)
        *p++=n+'0';
      else
        *p++=n-10+'a';
     }
   }
  *p='\0';
  return tobuf;
 }

int main(int argc, char **argv)
 {
  int ec,c,i;
  unsigned long channel;
  int ttl=2;
  int ziplevel=0;
  char zipstr[5];
  FrontierConfig *config;
  char *connectstr="";

  while(1)
   {
    c=getopt(argc,argv,":c:rR");
    switch(c)
     {
      case 'c':
        connectstr=optarg;
        break;
      case 'r':
        ttl=1;
        break;
      case 'R':
        ttl=3;
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

  /* default zip level to 0, unlike ordinary frontier client */
  if(getenv("FRONTIER_RETRIEVEZIPLEVEL")==0)
    putenv("FRONTIER_RETRIEVEZIPLEVEL=0");

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

  frontier_setTimeToLive(channel,ttl);
  ziplevel=frontier_getRetrieveZipLevel(channel);
  if(ziplevel==0)
    zipstr[0]='\0';
  else
   {
    zipstr[0]='z';
    zipstr[1]='i';
    zipstr[2]='p';
    zipstr[3]='0'+ziplevel;
    zipstr[4]='\0';
   }

  for(i=optind;i<argc;i++){
    char *uribuf;
    FrontierRSBlob *frsb;
    int fd;
    int bytes;
    int n;
    char *p;
    char *localname;
    char *encodedargv;

    encodedargv=doubleurlencode(argv[i]);
    uribuf=malloc(strlen(encodedargv)+128);
    sprintf(uribuf,"Frontier/type=frontier_file:1:DEFAULT&encoding=BLOB%s&p1=%s",zipstr,encodedargv);
    free(encodedargv);
    ec=frontier_getRawData(channel,uribuf);
    free(uribuf);
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
    localname=strrchr(argv[i],'/');
    if(localname==NULL)
      localname=argv[i];
    else
      localname++;
    fd=open(localname,O_CREAT|O_TRUNC|O_WRONLY,0666);
    if(fd==-1)
     {
      fprintf(stderr,"Error creating %s: %s\n",localname,strerror(errno));
      ec=!FRONTIER_OK;
      break;
     }
    bytes=0;
    while(1)
     {
      char resulttype=frontierRSBlob_getByte(frsb,&ec);
      if(ec!=FRONTIER_OK)
       {
	fprintf(stderr,"Error getting result type for %s: %s\n",argv[i],frontier_getErrorMsg());
	break;
       }
      if(resulttype==RESULT_TYPE_EOR)
        break;
      if(resulttype!=RESULT_TYPE_ARRAY)
       {
	fprintf(stderr,"Unexpected result type for %s: %d\n",argv[i],resulttype);
	ec=!FRONTIER_OK;
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
      if(write(fd,p,n)<0)
       {
	fprintf(stderr,"Error writing to %s: %s\n",localname,strerror(errno));
	ec=!FRONTIER_OK;
	break;
       }
      bytes+=n;
     }
    close(fd);
    frontierRSBlob_close(frsb,&ec);
    if(ec!=FRONTIER_OK)
      break;
    printf("%d bytes written to %s\n",bytes,localname);
   }

  frontier_closeChannel(channel);
  return (ec!=FRONTIER_OK);
 }
