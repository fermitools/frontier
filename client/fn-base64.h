/*
** Jack Jansen, CWI, July 1995.
** Brandon Long, September 2001.
** Python 2.3.3 code adapted by by Sergey Kosyakov, May 2004
** See http://www.python.org for license information
*/

#ifndef __H_BASE64_CUSTOM__
#define __H_BASE64_CUSTOM__

#define BASE64_NOSPACE		-1
#define BASE64_INVPADDING	-2

int base64_ascii2bin(const unsigned char *ascii_data,int ascii_len,
                        unsigned char *bin_data,int buf_size);


#endif /*__H_BASE64_CUSTOM__*/


