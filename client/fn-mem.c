/*
 * frontier client memory allocation handler
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

#include <frontier_client/frontier.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 
struct mem_chunk
 {
  void *ptr;
  int size;  
 };
 
static struct mem_chunk a_mem[1024];
static int max_mem=0;
static int mem_count=0;

void *frontier_malloc(size_t size)
 {
  void *ret;
  int i;
  
  ret=malloc(size);
  ++mem_count;
  
  for(i=0;i<=max_mem;i++)
   {
    if(!a_mem[i].ptr)
     {
      a_mem[i].ptr=ret;
      a_mem[i].size=size;
      break;
     }
   }
  
  if(i>max_mem)
   {
    max_mem++;
    a_mem[max_mem].ptr=ret;
    a_mem[max_mem].size=size;  
   }
  printf("malloc 0x%016lX, count %d, id %d, size %ld\n",(unsigned long)ret,mem_count,i,(long)size);
  return ret;
 }


void frontier_free(void *ptr)
 {
  int i;
  int size=-1;
  
  --mem_count;  
  for(i=0;i<=max_mem;i++)
   {
    if(a_mem[i].ptr==ptr)
     {
      size=a_mem[i].size;
      a_mem[i].size=-1;
      a_mem[i].ptr=(long)0;
      break;
     }
   }
   
  if(size==-1)
   {
    printf("Memory corrupted 0x%016lX, count %d, id %d\n",(unsigned long)ptr,mem_count,i);
    exit(1);
   }
  
  printf("free 0x%016lX, count %d, id %d, size %d\n",(unsigned long)ptr,mem_count,i,size);
  
  if(mem_count<2)
   {
    for(i=0;i<=max_mem;i++)
     {
      if(a_mem[i].ptr)
       {
        printf("left 0x%016lX, id %d, size %d\n",(unsigned long)a_mem[i].ptr,i,a_mem[i].size);
       }
     }
   }
  
  free(ptr);
 }
