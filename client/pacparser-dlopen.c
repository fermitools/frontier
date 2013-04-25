/*
 * pacparser dlopen interface
 * 
 * Author: Dave Dykstra
 *
 * $Id$
 *
 * Copyright (c) 2013, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 *
 */

#include <dlfcn.h>
#include "pacparser.h"
#include "frontier_client/frontier_error.h"

static void *pp_dlhandle;
static int (*pp_init)(void);
static void (*pp_set_error_printer)(pacparser_error_printer);
static void (*pp_setmyip)(const char *);
static int (*pp_parse_pac_string)(const char *);
static char *(*pp_find_proxy)(const char *,const char *);
static void (*pp_cleanup)(void);

static void *pp_funcps[] =
 {
  &pp_init,
  &pp_set_error_printer,
  &pp_setmyip,
  &pp_parse_pac_string,
  &pp_find_proxy,
  &pp_cleanup,
 };
static const char *pp_names[]=
 {
  "pacparser_init",
  "pacparser_set_error_printer",
  "pacparser_setmyip",
  "pacparser_parse_pac_string",
  "pacparser_find_proxy",
  "pacparser_cleanup",
 };

int frontier_pacparser_init(void)
 {
  const char *error;
  int i;

  /* add .1 on the end because if the API ever changes in an incompatible
   * way they're supposed to change it to .2  */
  pp_dlhandle=dlopen("libpacparser.so.1",RTLD_LAZY);
  if(!pp_dlhandle)
   {
    frontier_setErrorMsg(__FILE__,__LINE__,
       "config error: cannot dlopen %s",dlerror());
    return FRONTIER_ECFG;
   }
  
  dlerror();	/* Clear any existing error */

  for(i=0;i<(sizeof(pp_names)/sizeof(pp_names[0]));i++)
   {
    *(void **)(pp_funcps[i])=dlsym(pp_dlhandle,pp_names[i]);
    if((error=dlerror())!=0)
     {
      frontier_setErrorMsg(__FILE__,__LINE__, "config error: dlsym %s",error);
      dlclose(pp_dlhandle);
      pp_dlhandle=0;
      return FRONTIER_ECFG;
     }
   }

  return FRONTIER_OK;
 }

int pacparser_init(void)
 {
  if(!pp_dlhandle)
    return 0;
  return (*pp_init)();
 }

void pacparser_set_error_printer(pacparser_error_printer func)
 {
  if(!pp_dlhandle)
    return;
  (*pp_set_error_printer)(func);
  return;
 }

void pacparser_setmyip(const char *ip)
 {
  if(!pp_dlhandle)
    return;
  (*pp_setmyip)(ip);
 }

int pacparser_parse_pac_string(const char *string)
 {
  if(!pp_dlhandle)
    return 0;
  return (*pp_parse_pac_string)(string);
 }

char *pacparser_find_proxy(const char *url,const char *host)
 {
  if(!pp_dlhandle)
    return 0;
  return (*pp_find_proxy)(url,host);
 }

void pacparser_cleanup(void)
 {
  if(!pp_dlhandle)
    return;
  (*pp_cleanup)();
  dlclose(pp_dlhandle);
  pp_dlhandle=0;
 }
