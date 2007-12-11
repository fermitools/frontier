/*
 * frontier client endian header
 * 
 * Author: Dave Dykstra
 *
 * $Id$
 *
 *  Copyright (C) 2007  Fermilab
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
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
