/*
 * frontier client test base64 URL standalone main program
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
