#ifndef __HEADER_H_FN_ENDIAN_H
#define __HEADER_H_FN_ENDIAN_H

/* __BIG_ENDIAN__ or __LITTLE_ENDIAN__ are predefined on Mac OSX
   and endian.h doesn't exist there */

#ifdef __BIG_ENDIAN__
#define __BIG_ENDIAN	4321
#define __LITTLE_ENDIAN 1234
#define __BYTE_ORDER __BIG_ENDIAN
#else
#ifdef __LITTLE_ENDIAN__
#define __BIG_ENDIAN	4321
#define __LITTLE_ENDIAN 1234
#define __BYTE_ORDER __LITTLE_ENDIAN
#else
#include <endian.h>
#endif
#endif

#endif /*__HEADER_H_FN_ENDIAN_H*/
