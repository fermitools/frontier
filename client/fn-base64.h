/*
 * frontier client base64 encode/decode header
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

#ifndef __H_FN_BASE64__
#define __H_FN_BASE64__

#define BASE64_NOSPACE		-1
#define BASE64_INVPADDING	-2

struct s_fn_b64a2b_context {
    int dlast;
    int phase;
};
typedef struct s_fn_b64a2b_context fn_b64a2b_context;

// decode a portion of a stream of data from base64 ascii to binary
// caller must ensure that the ctxt structure is initialized to zero
// When finished, *ascii_lenp will have the number of bytes left unused
//  in the ascii data, and *bin_lenp will have the number of bytes
//  left in the binary data.
void fn_base64_stream_ascii2bin(fn_b64a2b_context *ctxt, 
		const unsigned char *ascii_data,int *ascii_lenp,
                     unsigned char *bin_data,int *bin_lenp);
                        
// decode from base64 ascii to binary
// return the amount of binary data produced
int fn_base64_ascii2bin(const unsigned char *ascii_data,int ascii_len,
                     unsigned char *bin_data,int bin_len);
                        
// encode from binary to base64 ascii
// return the amount of ascii data produced
int fn_base64_bin2ascii(const unsigned char *bin_data,int bin_len,
                     unsigned char *ascii_data,int ascii_len);

// URL-compatible encoding. It can be fed to standard BASE64 decoder
// after replacing: '.'->'+', '-'->'/', '_'->'=', and appending '\n'
// return the amount of ascii data produced
int fn_base64URL_bin2ascii(const unsigned char *bin_data,int bin_len,
                        unsigned char *ascii_data,int ascii_len);
                     

#endif /*__H_FN_BASE64__*/


