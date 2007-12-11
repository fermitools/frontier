/*
 * frontier client memdata handler
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

#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"

#define MEMDATA_INISIZE	(4*4096)

extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);


FrontierMemData *frontierMemData_create()
 {
  FrontierMemData *ms;

  ms=frontier_mem_alloc(sizeof(FrontierMemData));
  if(!ms) return ms;

  ms->size=MEMDATA_INISIZE;
  ms->len=0;
  ms->buf=frontier_mem_alloc(ms->size);
  if(!ms->buf)
   {
    frontier_mem_free(ms);
    ms=(void*)0;
    return ms;
   }
  return ms;
 }


void frontierMemData_delete(FrontierMemData *md)
 {
  if(!md) return;

  if(md->buf) frontier_mem_free(md->buf);
  frontier_mem_free(md);
 }


int frontierMemData_append(FrontierMemData *md,const char *buf,size_t size)
 {
  char *tmp_buf;

  if(md->len+size>=md->size)
   {
    while(md->len+size>=md->size) md->size*=2;
    tmp_buf=frontier_mem_alloc(md->size);
    if(!tmp_buf) return FRONTIER_EMEM;

    bcopy(md->buf,tmp_buf,md->len);
    frontier_mem_free(md->buf);
    md->buf=tmp_buf;
   }

  bcopy(buf,md->buf+md->len,size);
  md->len+=size;
  return FRONTIER_OK;
 }



