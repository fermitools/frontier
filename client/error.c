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
#include <frontier.h>

static const char *ferror[]=
 {
  "OK",						/*0*/
  "invalid argument",				/*-1*/
  "no more memory",				/*-2*/
  "libcurl initialization failed", 		/*-3*/
  "Channel can not be created",    		/*-4*/
  "libcurl has not accepted the URL", 		/*-5*/
  "surprise - unknown error!",			/*-6*/
  "HTTP response code is not 200",		/*-7*/
  "XML parser can not parse the response",	/*-8*/
  "Basse64 decode failed",			/*-9*/
  "no more rows in the RS",			/*-10*/
  "no such RS",					/*-11*/
  "Frontier API is not initialized",		/*-12*/
  "MD5 digest mismatch",			/*-13*/
  "Payload error signalled from server",	/*-14*/
  0
 };
static int ferror_last=14;


static const char *cerror[]=
 {
  "OK",						/*0*/
  "libcurl - unsupported protocol requested",	/*1*/
  "libcurl - failed init",			/*2*/
  "libcurl - wrong URL format",			/*3*/
  "libcurl - wrong format of something (something wrong)", /*4*/
  "libcurl - proxy name lookup failed",		/*5*/
  "libcurl - host name lookup failed",		/*6*/
  "libcurl - could not connect to remote host",	/*7*/
  0
 };
static int cerror_last=7;


const char *frontier_error_desc(int err)
 {
  if(err>0) return "unknown";

  err=-err;

  if(err<=ferror_last)
   {
    return ferror[err];
   }

  err-=100;

  if(err>=0 && err<=cerror_last)
   {
    return cerror[err];
   }

  return "unknown";
 }

