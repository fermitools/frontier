/*
 * FroNTier client API
 * 
 * Author: Sergey Kosyakov
 *
 * $Header$
 *
 * $Id$
 *
 */

#include <frontier-cpp.h>

#include <iostream>
#include <sstream>


extern "C"
 {
#include <frontier.h>
#include <stdlib.h>
 };

using namespace frontier;

static const char *blob_type_name[]={"byte","int4","int8","float","double","time","string"};
 
int AnyData::getInt()
 {
  if(isNull) return 0;
  if(t==BLOB_TYPE_INT4) return v.i4;
#ifdef FRONTIER_DEBUG
  std::cout<<"WARNING: converting "<<blob_type_name[t]<<" to int\n";
#endif //FRONTIER_DEBUG
  switch(t)
   {
    case BLOB_TYPE_BYTE: return (int)v.b;
    case BLOB_TYPE_TIME:
    case BLOB_TYPE_INT8: return (int)v.i8;
    case BLOB_TYPE_FLOAT: return (int)v.f;
    case BLOB_TYPE_DOUBLE: return (int)v.d;
    case BLOB_TYPE_ARRAY_BYTE: return atoi(v.str.p);
    default: break; // Just to make this geek gcc happy
   }
  std::cout<<"ERROR: can not be here "<<__FILE__<<":"<<__LINE__<<'\n';
  exit(2);
 }
 
long long AnyData::getLongLong()
 {
  if(isNull) return 0;
  if(t==BLOB_TYPE_INT8 || t==BLOB_TYPE_TIME) return v.i8;
#ifdef FRONTIER_DEBUG
  std::cout<<"WARNING: converting "<<blob_type_name[t]<<" to long long\n";
#endif //FRONTIER_DEBUG
  switch(t)
   {
    case BLOB_TYPE_BYTE: return (long long)v.b;
    case BLOB_TYPE_INT4: return (long long)v.i4;
    case BLOB_TYPE_FLOAT: return (long long)v.f;
    case BLOB_TYPE_DOUBLE: return (long long)v.d;
    case BLOB_TYPE_ARRAY_BYTE: return atoll(v.str.p);
    default: break; // Just to make this geek gcc happy
   }
  std::cout<<"ERROR: can not be here "<<__FILE__<<":"<<__LINE__<<'\n';
  exit(2); 
 }
   
float AnyData::getFloat()
 {
  if(isNull) return 0.0;
  if(t==BLOB_TYPE_FLOAT) return v.f;
#ifdef FRONTIER_DEBUG
  std::cout<<"WARNING: converting "<<blob_type_name[t]<<" to float\n";
#endif //FRONTIER_DEBUG
  switch(t)
   {
    case BLOB_TYPE_BYTE: return (float)v.b;
    case BLOB_TYPE_INT4: return (float)v.i4;
    case BLOB_TYPE_TIME:
    case BLOB_TYPE_INT8: return (float)v.i8;    
    case BLOB_TYPE_DOUBLE: return (float)v.d;
    case BLOB_TYPE_ARRAY_BYTE: return (float)atof(v.str.p);
    default: break; // Just to make this geek gcc happy
   }
  std::cout<<"ERROR: can not be here "<<__FILE__<<":"<<__LINE__<<'\n';
  exit(2);  
 }
   
double AnyData::getDouble()
 {
  if(isNull) return 0.0;
  if(t==BLOB_TYPE_DOUBLE) return v.d;
#ifdef FRONTIER_DEBUG
  std::cout<<"WARNING: converting "<<blob_type_name[t]<<" to double\n";
#endif //FRONTIER_DEBUG
  switch(t)
   {
    case BLOB_TYPE_BYTE: return (double)v.b;
    case BLOB_TYPE_INT4: return (double)v.i4;
    case BLOB_TYPE_TIME:
    case BLOB_TYPE_INT8: return (double)v.i8;    
    case BLOB_TYPE_FLOAT: return (double)v.f;
    case BLOB_TYPE_ARRAY_BYTE: return atof(v.str.p);
    default: break; // Just to make this geek gcc happy
   }
  std::cout<<"ERROR: can not be here "<<__FILE__<<":"<<__LINE__<<'\n';
  exit(2);   
 }
 
std::string* AnyData::getString()
 {
  if(isNull) return NULL;
  if(t==BLOB_TYPE_ARRAY_BYTE) return new std::string(v.str.p,v.str.s);
#ifdef FRONTIER_DEBUG
  std::cout<<"WARNING: converting "<<blob_type_name[t]<<" to string\n";
#endif //FRONTIER_DEBUG
  std::ostringstream oss;
  switch(t)
   {
    case BLOB_TYPE_BYTE: oss<<v.b; return new std::string(oss.str());
    case BLOB_TYPE_INT4: oss<<v.i4; return new std::string(oss.str());
    case BLOB_TYPE_TIME:
    case BLOB_TYPE_INT8: oss<<v.i8; return new std::string(oss.str());
    case BLOB_TYPE_FLOAT: oss<<v.f; return new std::string(oss.str());
    case BLOB_TYPE_DOUBLE: oss<<v.d; return new std::string(oss.str());
    default: break; // Just to make this geek gcc happy
   }
  std::cout<<"ERROR: can not be here "<<__FILE__<<":"<<__LINE__<<'\n';
  exit(2);    
 }


