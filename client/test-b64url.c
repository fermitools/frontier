#include "frontier_client/frontier.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
 {
  char str[]="select aaa from table";
  char *buf;
  int len;
  
  frontier_init(malloc,free);  
  
  len=fn_gzip_str2urlenc(str,strlen(str),&buf);
  
  printf("Len %d res [%s]\n",len,buf);
  
  return 0;
 }
