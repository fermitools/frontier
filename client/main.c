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

static const char ufmt[]="http://pox:8020/P6Test/test1.jsp";

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
  double d;
  int ret;
  FrontierChannel chnl;
  FrontierRespStat stat;
  FrontierRSBlob *rs;
  int i;
  int ec;
  char url[1024];

  ret=frontier_init(my_malloc,my_free);
  if(ret)
   {
    printf("Error %d\n",ret);
    exit(1);
   }

  chnl=frontier_createChannel(&ec);
  CHECK()

  bzero(url,1024);
  snprintf(url,1020,ufmt);
  printf("url %s\n",url);
  ret=frontier_getRawData(chnl,url);

  frontier_getRespStat(chnl,&stat);

  if(ret)
   {
    printf("error %d: %s\n",ret,frontier_error_desc(ret));
    printf("HRC %d\n",stat.http_resp_code);
    exit(1);
   }

  printf("HRC=%d size=%d\n",stat.http_resp_code,stat.raw_data_size);
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

  rs=frontierRSBlob_get(chnl,2,&ec);
  CHECK()
  for(d=0.0;d<10.0;d+=0.1)
   {
    double v=frontierRSBlob_getDouble(rs,&ec);
    CHECK()
    printf("%lf %lf\n",d,v);
   }
  frontierRSBlob_close(rs,&ec);

  frontier_closeChannel(chnl);

  exit(0);
 }


