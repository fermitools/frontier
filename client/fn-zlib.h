#ifndef __H__FN_ZLIB_H
#define __H__FN_ZLIB_H

#define MAX_STR2URL_SIZE	(1024*1024)      // 1MiB

#define FN_ZLIB_E_SMALLBUF	-1
#define FN_ZLIB_E_NOMEM		-2
#define FN_ZLIB_E_OTHER		-3
#define FN_ZLIB_E_TOOBIG	-4

long fn_gzip_str(const char *src,long src_size,char *dest,long dest_size);


#endif //__H__FN_ZLIB_H
