/*
 * Frontier logging.
 * 
 * $Id$
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

