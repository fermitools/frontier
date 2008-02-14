/*
** The frontier base64 decoder is based on source code that came without a
**  copyright from http://www.jeremie.com/frolic/base64/, decode-1.c
**
** The frontier base64 encoder has the following copyright:
** Jack Jansen, CWI, July 1995.
** Brandon Long, September 2001.
** Python 2.3.3 code adapted by Sergey Kosyakov, May 2004
** See http://www.python.org for license information
**
*/

#include "fn-base64.h"

static unsigned char table_b2a_base64[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BASE64_PAD '='

// These two below are for URL-compatible encoding
static unsigned char table_b2a_base64URL[] =
"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789.-";
#define BASE64URL_PAD '_'


/* Max binary chunk size; limited only by available memory */
#define BASE64_MAXBIN (1024*1024*1024)

static int base64_table_a2b[256] = {
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 00-0F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 10-1F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,62,-1,-1,-1,63,  /* 20-2F */
    52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,-1,  /* 30-3F */
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,  /* 40-4F */
    15,16,17,18,19,20,21,22,23,24,25,-1,-1,-1,-1,-1,  /* 50-5F */
    -1,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,  /* 60-6F */
    41,42,43,44,45,46,47,48,49,50,51,-1,-1,-1,-1,-1,  /* 70-7F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 80-8F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* 90-9F */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* A0-AF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* B0-BF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* C0-CF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* D0-DF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,  /* E0-EF */
    -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1   /* F0-FF */
};

void fn_base64_stream_ascii2bin(fn_b64a2b_context *ctxt, 
	    const unsigned char *ascii_data,int *ascii_lenp,
                        unsigned char *bin_data,int *bin_lenp)
{
    unsigned char *p;
    const unsigned char *endp, *enda;
    int d, dlast, phase;
    unsigned char c;

    dlast = ctxt->dlast;
    phase = ctxt->phase;
    d = 0;
    p = bin_data;
    endp = p + *bin_lenp;
    enda = ascii_data + *ascii_lenp;
    while((ascii_data != enda) && (p != endp))
    {
        d = base64_table_a2b[(int)(*ascii_data++)];
        if(d != -1)
        {
            switch(phase)
            {
            case 0:
                ++phase;
                break;
            case 1:
                c = ((dlast << 2) | ((d & 0x30) >> 4));
                *p++ = c;
                ++phase;
                break;
            case 2:
                c = (((dlast & 0xf) << 4) | ((d & 0x3c) >> 2));
                *p++ = c;
                ++phase;
                break;
            case 3:
                c = (((dlast & 0x03 ) << 6) | d);
                *p++ = c;
                phase = 0;
                break;
            }
            dlast = d;
        }
    }
    ctxt->dlast = dlast;
    ctxt->phase = phase;
    *ascii_lenp = enda - ascii_data;
    *bin_lenp = endp - p;
}

int fn_base64_ascii2bin(const unsigned char *ascii_data,int ascii_len,
                        unsigned char *bin_data,int bin_len)
{
    fn_b64a2b_context ctxt;
    int save_bin_len=bin_len;
    ctxt.dlast=ctxt.phase=0;
    fn_base64_stream_ascii2bin(&ctxt,ascii_data,&ascii_len,bin_data,&bin_len);
    return save_bin_len-bin_len;
}

int fn_base64_bin2ascii(const unsigned char *bin_data,int bin_len,
                     unsigned char *ascii_data,int ascii_len)
 {
  int leftbits=0;
  unsigned char this_ch;
  unsigned int leftchar=0;
  unsigned char *ptr=ascii_data;  

  if(bin_len>BASE64_MAXBIN) return BASE64_NOSPACE;

  /* We're lazy and allocate too much (fixed up later).
   "+3" leaves room for up to two pad characters and a trailing
   newline.  Note that 'b' gets encoded as 'Yg==\n' (1 in, 5 out). */
  if(bin_len*2+3>ascii_len) return BASE64_NOSPACE;

  for(;bin_len>0 ;bin_len--, bin_data++) 
   {
    /* Shift the data into our buffer */
    leftchar = (leftchar << 8) | *bin_data;
    leftbits += 8;

    /* See if there are 6-bit groups ready */
		while ( leftbits >= 6 ) {
			this_ch = (leftchar >> (leftbits-6)) & 0x3f;
			leftbits -= 6;
			*ascii_data++ = table_b2a_base64[this_ch];
		}
	}
	if ( leftbits == 2 ) {
		*ascii_data++ = table_b2a_base64[(leftchar&3) << 4];
		*ascii_data++ = BASE64_PAD;
		*ascii_data++ = BASE64_PAD;
	} else if ( leftbits == 4 ) {
		*ascii_data++ = table_b2a_base64[(leftchar&0xf) << 2];
		*ascii_data++ = BASE64_PAD;
	}
	*ascii_data++ = '\n';	/* Append a courtesy newline */

    ascii_len=ascii_data-ptr;
    
    return ascii_len;
}


// This method is modified to create URL-compatible output
int fn_base64URL_bin2ascii(const unsigned char *bin_data,int bin_len,
                        unsigned char *ascii_data,int ascii_len)
 {
  int leftbits=0;
  unsigned char this_ch;
  unsigned int leftchar=0;
  unsigned char *ptr=ascii_data;  

  if(bin_len>BASE64_MAXBIN) return BASE64_NOSPACE;

  /* We're lazy and allocate too much (fixed up later).
   "+3" leaves room for up to two pad characters and a trailing
   newline.  Note that 'b' gets encoded as 'Yg==\n' (1 in, 5 out). */
  if(bin_len*2+3>ascii_len) return BASE64_NOSPACE;

  for(;bin_len>0 ;bin_len--, bin_data++) 
   {
    /* Shift the data into our buffer */
    leftchar = (leftchar << 8) | *bin_data;
    leftbits += 8;

    /* See if there are 6-bit groups ready */
		while ( leftbits >= 6 ) {
			this_ch = (leftchar >> (leftbits-6)) & 0x3f;
			leftbits -= 6;
			*ascii_data++ = table_b2a_base64URL[this_ch];
		}
	}
	if ( leftbits == 2 ) {
		*ascii_data++ = table_b2a_base64URL[(leftchar&3) << 4];
		*ascii_data++ = BASE64URL_PAD;
		*ascii_data++ = BASE64URL_PAD;
	} else if ( leftbits == 4 ) {
		*ascii_data++ = table_b2a_base64URL[(leftchar&0xf) << 2];
		*ascii_data++ = BASE64URL_PAD;
	}
	/* A courtesy newline is ommited */

    ascii_len=ascii_data-ptr;
    
    return ascii_len;
}
