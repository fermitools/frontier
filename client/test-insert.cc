/*
 * FroNTier client API
 * 
 * Author: Sergey Kosyakov
 *
 * $Header$
 *
 * $Id$
 *
 *
 * This program inserts new record into the table
 * Create the table and XSD first:
 * create table test_insert 
 *
 */
#include <frontier_client/frontier-cpp.h>

#include <iostream>
#include <stdexcept>

#ifndef FNTR_USE_EXCEPTIONS
#define CHECK_ERROR() do{if(ds.err_code){std::cout<<"ERROR:"<<ds.err_msg<<std::endl; exit(1);}}while(0)
#else
#define CHECK_ERROR() 
#endif //FNTR_USE_EXCEPTIONS
