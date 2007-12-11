/*
 * frontier client main for a standalone program
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


int do_main(int argc, char **argv);

int main(int argc, char **argv)
 {
  while(1)
   {
    do_main(argc,argv);
   }
 }
 
int do_main(int argc, char **argv)
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

  ret=frontier_getRawData(chnl,"Frontier?type=SvxBeamPosition:1&encoding=BLOB&cid=316011&type=CALTrigWeights:1&encoding=BLOB&cid=14319");
  if(ret)
   {
    printf("Error %d\n",ret);
    exit(1);
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

  return(0);
 }


