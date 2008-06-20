/*
 * frontier client error API header
 * 
 * Author: Sergey Kosyakov
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

#ifndef FRONTIER_ERROR_H
#define FRONTIER_ERROR_H

#define FRONTIER_OK		0
#define FRONTIER_EIARG		-1	/*Invalid argument passed*/
#define FRONTIER_EMEM		-2	/*mem_alloc failed*/
#define FRONTIER_ECFG		-3	/*config error*/
#define FRONTIER_ESYS		-4	/*system error*/
#define FRONTIER_EUNKNOWN	-5	/*unknown error*/
#define FRONTIER_ENETWORK	-6	/*error while communicating over network*/
#define FRONTIER_EPROTO		-7	/*protocol level error (e.g. wrong response)*/
#define FRONTIER_ESERVER	-8	/*server error (may be cached for short time)*/

void frontier_setErrorMsg(const char *file, int line,const char *fmt,...);
const char *frontier_get_err_desc(int err_num);
const char *frontier_getErrorMsg();
void frontier_log(int level,const char *file,int line,const char *fmt,...);

#define FRONTIER_MSG(e) do{frontier_setErrorMsg(__FILE__,__LINE__,"error %d: %s",(e),frontier_get_err_desc(e));}while(0)

#endif /* FRONTIER_ERROR_H */

