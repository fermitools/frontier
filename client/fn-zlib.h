/*
 * frontier client zlib support header
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
#ifndef __H__FN_ZLIB_H
#define __H__FN_ZLIB_H

#define MAX_STR2URL_SIZE	(1024*1024)      // 1MiB

#define FN_ZLIB_E_SMALLBUF	-1
#define FN_ZLIB_E_NOMEM		-2
#define FN_ZLIB_E_OTHER		-3
#define FN_ZLIB_E_TOOBIG	-4

long fn_gzip_str(const char *src,long src_size,char *dest,long dest_size);
int fn_gunzip(unsigned char *dest,long *dest_sizep,const unsigned char *src,long src_size);
void fn_gzip_cleanup();


#endif //__H__FN_ZLIB_H
