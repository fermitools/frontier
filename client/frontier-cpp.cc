/*
 * frontier client C++ API implementation
 * 
 * Author: Sergey Kosyakov
 *
 * $Id$
 *
 * Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
 * All rights reserved. 
 *
 * For details of the Fermitools (BSD) license see Fermilab-2009.txt or
 *  http://fermitools.fnal.gov/about/terms.html
 *
 */

#include <cstdlib>
#include <sstream>
#include <cstring>
#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"
#include "frontier_client/FrontierExceptionMapper.hpp"

extern "C"
{
extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);
};

static std::string create_err_msg(const char *str)
 {
  return std::string(str)+std::string(": ")+std::string(frontier_getErrorMsg());
 }

using namespace frontier;


void Request::addKey(const std::string& key,const std::string& value)
 {
  if(!v_key) v_key=new std::vector<std::string>();
  if(!v_val) v_val=new std::vector<std::string>();
  
  v_key->insert(v_key->end(),key);
  v_val->insert(v_val->end(),value);
 }
 

Request::~Request()
 {
  if(v_val) {delete v_val; v_val=NULL;}
  if(v_key) {delete v_key; v_key=NULL;}
 }

 
 
std::string Request::encodeParam(const std::string &value)
 {
  const char *str=value.c_str();
  char *buf;
  int len;
  
  len=fn_gzip_str2urlenc(str,strlen(str),&buf);
  if(len<0)
   {
    std::ostringstream oss;
    oss<<"Error "<<len<<" while encoding parameter ["<<value<<"]";
    throw RuntimeError(oss.str());
   }
   
  std::string ret(buf,len);
  frontier_mem_free(buf);
  return ret;
 }
 
 
void Request::setRetrieveZipLevel(int level)
 {
  frontierConfig_setDefaultRetrieveZipLevel(level);
 }
 
int frontier::init()
 {
  int ret;

#ifdef FN_MEMORY_DEBUG
  ret=frontier_init(frontier_malloc,frontier_free);
#else  
  ret=frontier_init(malloc,free);
#endif //FN_MEMORY_DEBUG
  if(ret) throw RuntimeError(create_err_msg("Frontier initialization failed"));
  return ret;
 }

int frontier::init(const std::string& logfilename, const std::string& loglevel)
 {
  int ret;

#ifdef FN_MEMORY_DEBUG
  ret=frontier_initdebug(frontier_malloc,frontier_free,logfilename.c_str(),loglevel.c_str());
#else  
  ret=frontier_initdebug(malloc,free,logfilename.c_str(),loglevel.c_str());
#endif //FN_MEMORY_DEBUG
  if(ret) throw RuntimeError(create_err_msg("Frontier initialization failed"));
  return ret;
 }

 
Connection::Connection(const std::string& server_url,const std::string* proxy_url)
 {
  int ec=FRONTIER_OK;
  const char *proxy_url_c=NULL;
  
  init();
  
  if(proxy_url) proxy_url_c=proxy_url->c_str();

  channel=frontier_createChannel(server_url.c_str(),proxy_url_c,&ec);
  if(ec!=FRONTIER_OK) {
    FrontierExceptionMapper::throwException(ec, "Can not create frontier channel");
  }
 }

Connection::Connection(const std::list<std::string>& serverUrlList,
  const std::list<std::string>& proxyUrlList) {
 
  init();
  
  /*
   * Create empty config struct and add server/proxies to it. 
   */
  int errorCode = FRONTIER_OK;
  FrontierConfig* config = frontierConfig_get("", "", &errorCode); 
  if(errorCode != FRONTIER_OK) {
    FrontierExceptionMapper::throwException(errorCode, "Can not get frontier config object");
  }
  typedef std::list<std::string>::const_iterator LI;
  for(LI i = serverUrlList.begin(); i != serverUrlList.end(); ++i) {
    errorCode = frontierConfig_addServer(config, i->c_str());
    if(errorCode != FRONTIER_OK) {
      std::ostringstream oss;
      oss << "Error adding frontier server " << i->c_str();
      FrontierExceptionMapper::throwException(errorCode, oss.str());
    }
  }
  for(LI i = proxyUrlList.begin(); i != proxyUrlList.end(); ++i) {
    errorCode = frontierConfig_addProxy(config, i->c_str(), 0);
    if(errorCode != FRONTIER_OK) {
      std::ostringstream oss;
      oss << "Error adding frontier proxy " << i->c_str();
      FrontierExceptionMapper::throwException(errorCode, oss.str());
    }
  }
  errorCode = FRONTIER_OK;
  channel = frontier_createChannel2(config, &errorCode);
  if(errorCode != FRONTIER_OK) {
    FrontierExceptionMapper::throwException(errorCode, "Error creating frontier channel");
  }

}
 
Connection::~Connection()
 {
  frontier_closeChannel(channel);
 }

void Connection::setReload(int reload)
 {
  // deprecated interface
  setTimeToLive(reload?1:2);
 }
 
void Connection::setTimeToLive(int ttl)
 {
  frontier_setTimeToLive(channel,ttl);
 }
 

void Connection::setDefaultParams(const std::string& logicalServer,
	    const std::string& parameterList)
 {
  frontier::init();
  frontierConfig_setDefaultLogicalServer(logicalServer.c_str());
  frontierConfig_setDefaultPhysicalServers(parameterList.c_str());
 }


Session::Session(Connection *connection)
 {
  first_row=0;
  uri=NULL;
  internal_data=NULL;
  channel=connection->channel;
  have_saved_byte=0;
  num_records=0;
}
  
void Session::getData(const std::vector<const Request*>& v_req)
 {
  int ec;

  if(uri) {delete uri; uri=NULL;}

  if(internal_data) {frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);internal_data=NULL;}

  std::ostringstream oss;
  oss<<"Frontier";
  char delim='/';
 
  for(std::vector<const Request*>::size_type i=0;i<v_req.size();i++)
   {
    std::string enc;
    switch(v_req[i]->enc)
     {
      case BLOB: enc="BLOB"; break;
      default: throw InvalidArgument("Unknown encoding requested");
     }    
    oss << delim;
    if(v_req[i]->is_meta)
      oss << "meta=";
    else
      oss << "type=";
    oss << v_req[i]->obj_name;
    delim='&';
    oss << delim << "encoding=" << enc;
    int ziplevel = frontier_getRetrieveZipLevel(channel);
    if (ziplevel > 0)
      oss << "zip" << ziplevel;
    
    if(v_req[i]->v_key)
     {
      for(std::vector<std::string>::size_type n=0;n<v_req[i]->v_key->size();n++)
       {
        oss << delim << v_req[i]->v_key->operator[](n) << '=' 
                     << v_req[i]->v_val->operator[](n);
       }
     }
   }

  uri=new std::string(oss.str());
  //std::cout << "URL <" << *url << ">\n";

  ec=frontier_getRawData(channel,uri->c_str());
  if(ec!=FRONTIER_OK) {
    FrontierExceptionMapper::throwException(ec, "Can not get data");
  }
 }


void Session::setCurrentLoad(int n) {
  int ec=FRONTIER_OK;
  first_row=0;
  FrontierRSBlob *oldrsb=(static_cast<FrontierRSBlob*>(internal_data));
  FrontierRSBlob *rsb=frontierRSBlob_open(channel,oldrsb,n,&ec);
  if(ec!=FRONTIER_OK) {
    FrontierExceptionMapper::throwException(ec, "Can not set current load");
  }
  if(oldrsb) frontierRSBlob_close(oldrsb,&ec);
  
  internal_data=rsb;
  if(getCurrentLoadError() != FRONTIER_OK) {
    throw ProtocolError(getCurrentLoadErrorMessage());
  }
  num_records=frontierRSBlob_getRecNum(rsb);
}

 
int Session::getCurrentLoadError() const
 {
  FrontierRSBlob *rsb=(FrontierRSBlob*)internal_data;
  return frontierRSBlob_payload_error(rsb);
 }
 

const char* Session::getCurrentLoadErrorMessage() const
 {
  FrontierRSBlob *rsb=(FrontierRSBlob*)internal_data;
  return frontierRSBlob_payload_msg(rsb);
 }
 

unsigned int Session::getRecNum()
 {
   return num_records;
 }

// Get number of records.
unsigned int Session::getNumberOfRecords()
 {
  return num_records;
 }

 
unsigned int Session::getRSBinarySize()
 {
  if(!internal_data) {
    // this can happen at the end, return same as getRSBinaryPos
    return 0;
  }
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return frontierRSBlob_getSize(rs);
 }

 
unsigned int Session::getRSBinaryPos()
 {
  if(!internal_data) {
    // this can happen at the end, return same as getRSBinarySize
    return 0;
  }
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return frontierRSBlob_getPos(rs);
 }
  
 
int Session::getAnyData(AnyData* buf,int not_eor)
 {
  buf->isNull=0;
  buf->sessionp=this;
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt;
  
  if(not_eor && isEOR())
    throw InvalidArgument("EOR has been reached");
  
  if(have_saved_byte)
   {
    //std::cout<<"using saved byte "<<(int)saved_byte<<'\n';
    dt=saved_byte;
    have_saved_byte=0;
    if(dt!=BLOB_TYPE_EOR&&!internal_data)
      throw InvalidArgument("Session::getAnyData() Current load is not set");
   }
  else
   {
    if(!internal_data)
      throw InvalidArgument("Session::getAnyData() Current load is not set");
    dt=frontierRSBlob_getByte(rs,&ec);
   }

  //std::cout<<"Intermediate type prefix "<<(int)dt<<'\n';
  if(ec!=FRONTIER_OK) {
    throw LogicError("getAnyData() failed while getting type", ec);
  }
  last_field_type=dt;
   
  if(dt&BLOB_BIT_NULL)
   {
    //std::cout<<"The field is NULL\n";
    buf->isNull=1;
    buf->t=dt&(~BLOB_BIT_NULL);
    buf->v.str.s=0;
    buf->v.str.p=NULL;
    return 0;
   }
  //std::cout<<"Extracted type prefix "<<(int)dt<<'\n';
  
  char *p;
  int len;  
  
  switch(dt)
   {
    case BLOB_TYPE_BYTE: buf->set(frontierRSBlob_getByte(rs,&ec)); break;
    case BLOB_TYPE_INT4: buf->set(frontierRSBlob_getInt(rs,&ec)); break;
    case BLOB_TYPE_INT8: buf->set(frontierRSBlob_getLong(rs,&ec)); break;
    case BLOB_TYPE_FLOAT: buf->set((float)frontierRSBlob_getFloat(rs,&ec)); break;
    case BLOB_TYPE_DOUBLE: buf->set(frontierRSBlob_getDouble(rs,&ec)); break;
    case BLOB_TYPE_TIME: buf->set(frontierRSBlob_getLong(rs,&ec)); break;
    case BLOB_TYPE_ARRAY_BYTE:
      len=frontierRSBlob_getInt(rs,&ec);
      if(ec!=FRONTIER_OK)
	throw LogicError("can not get byte array length", ec);
      if(len<0)
        throw LogicError("negative byte array length", ec);
      p=frontierRSBlob_getByteArray(rs,len,&ec); 
      if(ec==FRONTIER_OK)
       {
	// save the next byte and replace it with a C string terminator
	// this avoids having to make another copy of the data
        saved_byte=frontierRSBlob_getByte(rs,&ec);
        //std::cout<<"saving byte "<<(int)saved_byte<<'\n';
        have_saved_byte=1;
        p[len]=0;
        buf->set(len,p);
       }
      break;
    case BLOB_TYPE_EOR:
      buf->setEOR();
      break;
    default: 
      //std::cout<<"Unknown type prefix "<<(int)dt<<'\n';
      throw InvalidArgument("unknown type prefix");
   }
  if(ec!=FRONTIER_OK)
    throw LogicError("can not get AnyData value", ec);
  return 0;
 }

void Session::clean()
 {
  if(internal_data&&isEOF())
   {
    int ec=FRONTIER_OK;
    /* delete as soon as possible because RSBlob may hold large buffer */
    frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);
    internal_data=NULL;
    if(ec!=FRONTIER_OK)
      throw LogicError("can not get AnyData value", ec);
   }
 }
 
 
BLOB_TYPE Session::nextFieldType()
 {
  if(have_saved_byte)
   {
    //std::cout<<"nextFieldType using saved byte "<<(int)saved_byte<<'\n';
    return saved_byte;
   }

  if(!internal_data) {
    throw InvalidArgument("Session::nextFieldType() Current load is not set");
  }
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt=frontierRSBlob_checkByte(rs,&ec);
  if(ec!=FRONTIER_OK) {
    throw LogicError("getAnyData() failed while checking type", ec);
  }
  
  return dt;
 }
 
 
int Session::getInt()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  return ad.getInt();
 }


long Session::getLong()
 {
  AnyData ad;
  if(getAnyData(&ad)) return -1;
  
  if(sizeof(long)==8) return ad.getLongLong();  
  return ad.getInt();
 }
 

long long Session::getLongLong()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  
  return ad.getLongLong();  
 }


double Session::getDouble()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;  
   
  return ad.getDouble();
 }
 
 
float Session::getFloat()
 {
  AnyData ad;
  if(getAnyData(&ad)) return -1;  
   
  return ad.getFloat();
 } 

 
long long Session::getDate()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  
  return ad.getLongLong();   
 }
 
 
std::string* Session::getString()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return NULL;
   
  return ad.getString();
 }
 
 
std::string* Session::getBlob()
 {
  return getString();
 }

 
 
void Session::assignString(std::string *s)
 {
  AnyData ad;

  if(getAnyData(&ad))
   {
    *s="";
    return;
   }

  ad.assignString(s);
 }



int Session::next()
 {
  AnyData ad;

  if(!first_row)
   {
    first_row=1;
    return !(isEOF());
   }

  while(1)
   {
    if(isEOF()) return 0;
    if(getAnyData(&ad,0)) return 0;
    if(isEOF()) return 0;
    if(ad.isEOR()) return 1;
   }
 }

 
 
Session::~Session()
 {
  int ec;
  if(internal_data) {frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);internal_data=NULL;}
  if(uri) delete uri;
 }


std::vector<unsigned char>* Session::getRawAsArrayUChar()
 {
  std::string *blob=getBlob();
  int len=blob->size();
  std::vector<unsigned char> *ret=new std::vector<unsigned char>(len);
  const char *s=blob->c_str();
  for(int i=0;i<len;i++)
   {
    ret->operator[](i)=s[i];
   }
  delete blob;
  return ret;
 }


std::vector<int>* Session::getRawAsArrayInt()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    throw InvalidArgument("Blob size is not multiple of 4 - can not convert to int[]");
   }
  int len=blob->size()/4;
  std::vector<int> *ret=new std::vector<int>(len);
  const char *s=blob->c_str();
  for(int i=0;i<len;i++)
   {
    ret->operator[](i)=frontier_n2h_i32(s+i*4);
   }
  delete blob;
  return ret;
 }
 
   
std::vector<float>* Session::getRawAsArrayFloat()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    throw InvalidArgument("Blob size is not multiple of 4 - can not convert to float[]");
   }
  int len=blob->size()/4;
  std::vector<float> *ret=new std::vector<float>(len);
  const char *s=blob->c_str();
  for(int i=0;i<len;i++)
   {
    ret->operator[](i)=frontier_n2h_f32(s+i*4);
   }
  delete blob;
  return ret;
 }


std::vector<double>* Session::getRawAsArrayDouble()
 {
  std::string *blob=getBlob();
  if(blob->size()%8) 
   {
    delete blob; 
    throw InvalidArgument("Blob size is not multiple of 8 - can not convert to double[]");
   }
  int len=blob->size()/8;
  std::vector<double> *ret=new std::vector<double>(len);
  const char *s=blob->c_str();
  for(int i=0;i<len;i++)
   {
    ret->operator[](i)=frontier_n2h_d64(s+i*8);
   }
  delete blob;
  return ret; 
 }


std::vector<long>* Session::getRawAsArrayLong()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    throw InvalidArgument("Blob size is not multiple of 4 - can not convert to int32[]");
   }
  int len=blob->size()/4;
  std::vector<long> *ret=new std::vector<long>(len);
  const char *s=blob->c_str();
  for(int i=0;i<len;i++)
   {
    ret->operator[](i)=frontier_n2h_i32(s+i*4);
   }
  delete blob;
  return ret; 
 }

DataSource::DataSource(const std::string& server_url,const std::string* proxy_url)
 : Connection(server_url, proxy_url), Session(this)
 {
 }

DataSource::DataSource(const std::list<std::string>& serverUrlList,
  const std::list<std::string>& proxyUrlList) 
 : Connection(serverUrlList, proxyUrlList), Session(this)
 {
 }
 
DataSource::~DataSource()
 {
 // just a placeholder; parents automatically called
 }

