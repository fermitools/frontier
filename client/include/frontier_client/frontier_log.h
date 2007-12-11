/*
 * frontier client logging header
 * 
 * Author: Sinisa Veseli
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

#ifndef FRONTIER_LOG_H
#define FRONTIER_LOG_H

#define FRONTIER_LOGLEVEL_DEBUG		0
#define FRONTIER_LOGLEVEL_WARNING	1
#define FRONTIER_LOGLEVEL_INFO		FRONTIER_LOGLEVEL_WARNING
#define FRONTIER_LOGLEVEL_ERROR		2
#define FRONTIER_LOGLEVEL_NOLOG		3

void frontier_log(int level, const char *file, int line, const char *fmt, ...);

#endif /* FRONTIER_LOG_H */

