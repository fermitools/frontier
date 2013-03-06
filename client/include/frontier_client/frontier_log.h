/*
 * frontier client logging header
 * 
 * Author: Sinisa Veseli
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

#ifndef FRONTIER_LOG_H
#define FRONTIER_LOG_H

#include <stdarg.h>

#define FRONTIER_LOGLEVEL_DEBUG		0
#define FRONTIER_LOGLEVEL_WARNING	1
#define FRONTIER_LOGLEVEL_INFO		FRONTIER_LOGLEVEL_WARNING
#define FRONTIER_LOGLEVEL_ERROR		2
#define FRONTIER_LOGLEVEL_NOLOG		3

int frontier_log_init();
void frontier_vlog(int level,const char *file,int line,const char *fmt,va_list ap);
void frontier_log(int level,const char *file,int line,const char *fmt,...);
void frontier_log_close();

#endif /* FRONTIER_LOG_H */

