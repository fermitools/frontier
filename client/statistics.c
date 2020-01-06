/*
 * frontier client statistics API implementation
 * 
 * Author: Dave Dykstra
 *
 * Copyright (c) 2020, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <strings.h>
#include <frontier_client/frontier.h>
#include "fn-internal.h"

static FrontierStatistics *statistics;
int started_by_debug=0;

void frontier_statistics_start()
 {
  (void) frontier_init(NULL, NULL);

  if(statistics!=NULL)
    return;

  started_by_debug=0;

  statistics=frontier_mem_alloc(sizeof(FrontierStatistics));
  if(statistics==NULL)
   {
    // out of memory already, so we'll skip taking stats
    return;
   }

  bzero(statistics,sizeof(FrontierStatistics));
 }

void frontier_statistics_start_debug()
 {
  if((frontier_log_level==FRONTIER_LOGLEVEL_DEBUG)&&(statistics==NULL))
   {
    frontier_statistics_start();
    started_by_debug=1;
   }
 }

void frontier_statistics_stop()
 {
  if(!statistics)
    return;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    "Total queries: %d",statistics->num_queries);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    "Total query errors: %d",statistics->num_errors);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    "Bytes per query min: %d, avg: %d, max: %d",
      statistics->bytes_per_query.min,
      statistics->bytes_per_query.avg,
      statistics->bytes_per_query.max);
  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
    "Milliseconds per query min: %d, avg: %d, max: %d",
      statistics->msecs_per_query.min,
      statistics->msecs_per_query.avg,
      statistics->msecs_per_query.max);

  frontier_mem_free(statistics);
  statistics=NULL;
 }

void frontier_statistics_stop_debug()
 {
  if(started_by_debug)
   {
    started_by_debug=0;
    frontier_statistics_stop();
   }
 }

int frontier_statistics_get_bytes(FrontierStatistics *stats,int maxbytes)
 {
  if((stats==NULL)||(maxbytes<=0)||(maxbytes>sizeof(FrontierStatistics)))
    return FRONTIER_EIARG;
  if(statistics==NULL)
    return FRONTIER_ECFG;

  bcopy(statistics,stats,maxbytes);
  return FRONTIER_OK;
 }

void frontier_statistics_start_query(fn_query_stat *query_stat)
 {
  if((statistics==NULL)||(query_stat==NULL))
    return;
 if(clock_gettime(CLOCK_REALTIME,&query_stat->starttime)!=0)
   query_stat->starttime.tv_sec=0;
 }

static void calcnumstats(FrontierStatisticsNum *num,unsigned int val)
 {
  double dqueries;
  if((num->min==0)||(val<num->min))
    num->min=val;
  if(val>num->max)
    num->max=val;
  dqueries=statistics->num_queries;
  num->avg=round((dqueries*num->avg+val)/(statistics->num_queries+1));
 }

void frontier_statistics_stop_query(fn_query_stat *query_stat,unsigned int numbytes)
 {
  unsigned int msec;
  struct timespec now;

  if((statistics==NULL)||(query_stat==NULL)||(query_stat->starttime.tv_sec==0))
    return;
  if(numbytes==0)
   {
    statistics->num_errors++;
    query_stat->starttime.tv_sec=0;
    return;
   }

  if(clock_gettime(CLOCK_REALTIME,&now)!=0)
   {
    query_stat->starttime.tv_sec=0;
    return;
   }

  msec=round((now.tv_nsec-query_stat->starttime.tv_nsec)/1.0e6);
  msec+=(now.tv_sec-query_stat->starttime.tv_sec)*1000;

  frontier_log(FRONTIER_LOGLEVEL_DEBUG,__FILE__,__LINE__,
      "query took %u milliseconds and read %u bytes",msec,numbytes);

  calcnumstats(&statistics->msecs_per_query,msec);
  calcnumstats(&statistics->bytes_per_query,numbytes);

  statistics->num_queries++;
  query_stat->starttime.tv_sec=0;
 }
