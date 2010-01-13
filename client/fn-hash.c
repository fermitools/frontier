/*
 * frontier client string-key hashtable implementation
 * 
 * Author: Dave Dykstra
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

 
#include "fn-hash.h"
#include <string.h>

#include "chashtable/hashtable.c"

DEFINE_HASHTABLE_INSERT(fn_hashtable_insert,char,fn_hashval)
DEFINE_HASHTABLE_SEARCH(fn_hashtable_search,char,fn_hashval)

static unsigned int
hashfromkey(void *k)
 {
  // This is the sdbm algorithm, generally praised on the internet as
  //  a good string hash
  unsigned int hash=0;
  unsigned char ch,*p=k;
  while((ch=*p++)!=0)
    hash=ch+(hash<<6)+(hash<<16)-hash;
  return hash;
 }

static int
equalkeys(void *k1,void *k2)
 {
  return(strcmp((char *)k1,(char *)k2)==0);
 }

fn_hashtable *
fn_inithashtable()
 {
  return(create_hashtable(256,hashfromkey,equalkeys));
 }
