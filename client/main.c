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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <frontier.h>


#define CHECK(){if(ec!=FRONTIER_OK){printf("Error %d\n",ec); exit(1);}}

static int mem_count=0;

static void *my_malloc(size_t size)
 {
  ++mem_count;
  printf("malloc, count %d\n",mem_count);
  return malloc(size);
 }

static void my_free(void *ptr)
 {
  --mem_count;
  printf("free, count %d\n",mem_count);
  free(ptr);
 }

int main(int argc, char **argv)
 {
  int ret;
  FrontierChannel chnl;
  FrontierRespStat stat;
  FrontierRSBlob *rs;
  int i;
  int ec;
  
  if(argc!=2)
   {
    printf("Usage: %s server_url\n",argv[0]);
    exit(1);
   }

  ret=frontier_init(my_malloc,my_free);
  if(ret)
   {
    printf("Error %d\n",ret);
    exit(1);
   }

  chnl=frontier_createChannel(argv[1],NULL,&ec);
  CHECK()

  ret=frontier_getRawData(chnl,"?type=svxbeamposition:1&encoding=BLOB&cid=316011&type=caltrigweights:1&encoding=BLOB&cid=14319");

  frontier_getRespStat(chnl,&stat);

  if(ret)
   {
    printf("error %d: %s\n",ret,frontier_error_desc(ret));
    printf("HRC %d\n",stat.http_resp_code);
    exit(1);
   }

  printf("HRC=%d size=%d\n",stat.http_resp_code,(int)stat.raw_data_size);
  printf("data %s\n",stat.raw_data_buf);

  for(i=0;;i++)
   {
    const char *n=frontier_getHttpHeaderName(chnl,i);
    if(!n) break;
    const char *v=frontier_getHttpHeaderValue(chnl,i);
    printf("\"%s: %s\"\n",n,v);
   }

  rs=frontierRSBlob_get(chnl,1,&ec);
  CHECK()
  for(i=0;i<100;i++)
   {
    int v=frontierRSBlob_getInt(rs,&ec);
    CHECK()
    printf("%d %d\n",i,v);
   }
  frontierRSBlob_close(rs,&ec);

  frontier_closeChannel(chnl);

  exit(0);
 }


