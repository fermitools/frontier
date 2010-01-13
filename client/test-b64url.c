/*
 * frontier client test base64 URL standalone main program
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
#include "frontier_client/frontier.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv)
 {
  //char str[]="select aaa from table";
  //char str[]="select name,version from frontier_descriptors";
  char str[] = "SELECT POS, ID_ID, MITEMS_HCALPEDESTALS_ITEM_MID, MITEMS_HCALPEDESTALS_ITEM_MV_1, MITEMS_HCALPEDESTALS_ITEM_MID, MITEMS_HCALPEDESTALS_ITEM_MV_2, MITEMS_HCALPEDESTALS_ITEM_MID, MITEMS_HCALPEDESTALS_ITEM_MV_3, MITEMS_HCALPEDESTALS_ITEM_MV_4 FROM CMS_VAL_HCAL_POOL_OWNER.HCALPEDESTAL_MITEMS WHERE ID_ID = 1 ORDER BY POS";
  char *buf;
  int len;
  
  frontier_init(malloc,free);  
  
  len=fn_gzip_str2urlenc(str,strlen(str),&buf);
  
  printf("Len %d res [%s]\n",len,buf);
  
  return 0;
 }
