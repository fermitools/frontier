/*
 * frontier client error API header
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

#ifndef FRONTIER_ERROR_H
#define FRONTIER_ERROR_H

#include "frontier_log.h"

#define FRONTIER_OK		0
#define FRONTIER_EIARG		-1	/*Invalid argument passed*/
#define FRONTIER_EMEM		-2	/*mem_alloc failed*/
#define FRONTIER_ECFG		-3	/*config error*/
#define FRONTIER_ESYS		-4	/*system error*/
#define FRONTIER_EUNKNOWN	-5	/*unknown error*/
#define FRONTIER_ENETWORK	-6	/*error while communicating over network*/
#define FRONTIER_EPROTO		-7	/*protocol level error (e.g. wrong response)*/
#define FRONTIER_ESERVER	-8	/*server error (may be cached for short time)*/
#define FRONTIER_ECONNECT       -9	/*socket connect error*/

void frontier_vsetErrorMsg(const char *file, int line,const char *fmt,va_list ap);
void frontier_setErrorMsg(const char *file, int line,const char *fmt,...);
const char *frontier_get_err_desc(int err_num);
const char *frontier_getErrorMsg();
void frontier_turnErrorsIntoDebugs(int value);

#define FRONTIER_MSG(e) do{frontier_setErrorMsg(__FILE__,__LINE__,"error %d: %s",(e),frontier_get_err_desc(e));}while(0)

#endif /* FRONTIER_ERROR_H */

