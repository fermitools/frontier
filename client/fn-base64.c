/*
** Jack Jansen, CWI, July 1995.
** Brandon Long, September 2001.
** Python 2.3.3 code adapted by Sergey Kosyakov, May 2004
** See http://www.python.org for license information
*/

#include "fn-base64.h"

static char table_a2b_base64[] = {
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
	-1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,62, -1,-1,-1,63,
	52,53,54,55, 56,57,58,59, 60,61,-1,-1, -1, 0,-1,-1, /* Note PAD->0 */
	-1, 0, 1, 2,  3, 4, 5, 6,  7, 8, 9,10, 11,12,13,14,
	15,16,17,18, 19,20,21,22, 23,24,25,-1, -1,-1,-1,-1,
	-1,26,27,28, 29,30,31,32, 33,34,35,36, 37,38,39,40,
	41,42,43,44, 45,46,47,48, 49,50,51,-1, -1,-1,-1,-1
};

#define BASE64_PAD '='

/* Max binary chunk size; limited only by available memory */
#define BASE64_MAXBIN 2*1024*1024*1024

static int binascii_find_valid(const unsigned char *s, int slen, int num)
{
	/* Finds & returns the (num+1)th
	** valid character for base64, or -1 if none.
	*/

	int ret = -1;
	unsigned char c, b64val;

	while ((slen > 0) && (ret == -1)) {
		c = *s;
		b64val = table_a2b_base64[c & 0x7f];
		if ( ((c <= 0x7f) && (b64val != (unsigned char)-1)) ) {
			if (num == 0)
				ret = *s;
			num--;
		}

		s++;
		slen--;
	}
	return ret;
}

int base64_ascii2bin(const unsigned char *ascii_data,int ascii_len,
                        unsigned char *bin_data,int buf_size)
{
	int leftbits = 0;
	unsigned char this_ch;
	unsigned int leftchar = 0;
	int quad_pos = 0;
        int bin_len;

	bin_len = ((ascii_len+3)/4)*3; /* Upper bound, corrected later */
        if(buf_size<bin_len) return BASE64_NOSPACE;

        bin_len=0;

	for( ; ascii_len > 0; ascii_len--, ascii_data++) {
		this_ch = *ascii_data;

		if (this_ch > 0x7f ||
		    this_ch == '\r' || this_ch == '\n' || this_ch == ' ')
			continue;

		/* Check for pad sequences and ignore
		** the invalid ones.
		*/
		if (this_ch == BASE64_PAD) {
			if ( (quad_pos < 2) ||
			     ((quad_pos == 2) &&
			      (binascii_find_valid(ascii_data, ascii_len, 1)
			       != BASE64_PAD)) )
			{
				continue;
			}
			else {
				/* A pad sequence means no more input.
				** We've already interpreted the data
				** from the quad at this point.
				*/
				leftbits = 0;
				break;
			}
		}

		this_ch = table_a2b_base64[*ascii_data];
		if ( this_ch == (unsigned char) -1 )
			continue;

		/*
		** Shift it in on the low end, and see if there's
		** a byte ready for output.
		*/
		quad_pos = (quad_pos + 1) & 0x03;
		leftchar = (leftchar << 6) | (this_ch);
		leftbits += 6;

		if ( leftbits >= 8 ) {
			leftbits -= 8;
			*bin_data++ = (leftchar >> leftbits) & 0xff;
			bin_len++;
			leftchar &= ((1 << leftbits) - 1);
		}
 	}

	if (leftbits != 0) {
		return BASE64_INVPADDING;
	}

	/* And set string size correctly. If the result string is empty
	** (because the input was all invalid) return the shared empty
	** string instead; _PyString_Resize() won't do this for us.
	*/
	return bin_len;
}

