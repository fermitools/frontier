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

#ifdef KCC_COMPILE
template class std::vector<double>;
template class std::vector<std::string>;
template class std::vector<unsigned char>;
template class std::vector<float>;
template class std::vector<int>;
template class std::vector<long>;
#endif //KCC_COMPILE


// This chunk of code below is ugly, but this is the way things 
// were done historically (C++/KCC and other junk); 
// here is C++ in all its glory (incompatibility on binary objects level). 
// So here is no even a tiny possibility to write a good code. I tried :-(
#ifdef FNTR_USE_EXCEPTIONS
#include <stdexcept>
#define RUNTIME_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=m; throw std::runtime_error(std::string(m)+std::string(": ")+frontier_error_desc(e));}while(0)
#define LOGIC_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=m; throw std::logic_error(std::string(m)+std::string(": ")+frontier_error_desc(e));}while(0)
#define RUNTIME_ERROR_NR(o,m,e) do{o->err_code=e; o->err_msg=m; throw std::runtime_error(std::string(m)+std::string(": ")+frontier_error_desc(e));}while(0)
#define LOGIC_ERROR_NR(o,m,e) do{o->err_code=e; o->err_msg=m; throw std::logic_error(std::string(m)+std::string(": ")+frontier_error_desc(e));}while(0)
#else
/*
#define RUNTIME_ERROR_NR(o,m,e) do{o->err_code=e; o->err_msg=m; return;}while(0)
#define LOGIC_ERROR_NR(o,m,e) do{o->err_code=e; o->err_msg=m;return;}while(0)
#define RUNTIME_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=m; return r;}while(0)
#define LOGIC_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=m;return r;}while(0)
*/
#define FN_ABORT(c,m) do{std::cout<<"Error #"<<(c)<<": "<<(m)<<". Abort.\n"; exit(c);}while(0)
#define RUNTIME_ERROR_NR(o,m,e) FN_ABORT(e,m)
#define LOGIC_ERROR_NR(o,m,e) FN_ABORT(e,m)
#define RUNTIME_ERROR(o,m,e,r) FN_ABORT(e,m)
#define LOGIC_ERROR(o,m,e,r) FN_ABORT(e,m)
#endif //USE_EXCEPTIONS

extern "C"
 {
#include <frontier.h>
#include <stdlib.h>
 };

using namespace frontier;

Request::~Request()
 {
  if(v_val) {delete v_val; v_val=NULL;}
  if(v_key) {delete v_key; v_key=NULL;}
 }


int frontier::init()
 {
  int ret;

  ret=frontier_init(malloc,free);
  //if(ret) RUNTIME_ERROR("Frontier initialization failed",ret);
  return ret;
 }


 
DataSource::DataSource(const std::string& server_url,const std::string* proxy_url)
 {
  int ec=FRONTIER_OK;
  const char *proxy_url_c=NULL;

  uri=NULL;
  internal_data=NULL;
  err_code=0;
  err_msg="";
  
  if(proxy_url) proxy_url_c=proxy_url->c_str();

  channel=frontier_createChannel(server_url.c_str(),proxy_url_c,&ec);
  if(ec!=FRONTIER_OK) RUNTIME_ERROR_NR(this,"Can not create frontier channel",ec);
  
 }

 
void DataSource::setReload(int reload)
 {
  frontier_setReload(channel,reload);
 }
 

void DataSource::getData(const std::vector<const Request*>& v_req)
 {
  int ec;

  if(uri) {delete uri; uri=NULL;}

  if(internal_data) {frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);internal_data=NULL;}

  std::ostringstream oss;
  char delim='?';
 
  for(std::vector<const Request*>::size_type i=0;i<v_req.size();i++)
   {
    oss << delim << "type=" << v_req[i]->obj_name << ':' << v_req[i]->v; delim='&';
    const char *enc;
    switch(v_req[i]->enc)
     {
      case BLOB: enc="BLOB"; break;
      default: LOGIC_ERROR_NR(this,"Unknown encoding requested",FRONTIER_EIARG);
     }
    oss << delim << "encoding=" << enc;
    oss << delim << v_req[i]->key1 << '=' << v_req[i]->val1;

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
  if(ec!=FRONTIER_OK) RUNTIME_ERROR_NR(this,"Can not get data",ec);
 }


void DataSource::setCurrentLoad(int n)
 {
  int ec=FRONTIER_OK;
  FrontierRSBlob *rsb=frontierRSBlob_get(channel,n,&ec);
  if(ec!=FRONTIER_OK) LOGIC_ERROR_NR(this,"Can not set current load",ec);
  internal_data=rsb;
  if(getCurrentLoadError()!=FRONTIER_OK) LOGIC_ERROR_NR(this,getCurrentLoadErrorMessage(),FRONTIER_EPAYLOAD);
 }

 
int DataSource::getCurrentLoadError() const
 {
  FrontierRSBlob *rsb=(FrontierRSBlob*)internal_data;
  return rsb->payload_error;
 }
 

const char* DataSource::getCurrentLoadErrorMessage() const
 {
  FrontierRSBlob *rsb=(FrontierRSBlob*)internal_data;
  return rsb->payload_msg; 
 }
 

unsigned int DataSource::getRecNum()
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,-1);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return rs->nrec;
 }

 
int DataSource::getAnyData(AnyData* buf)
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,-1);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt;
  
  /* Ignore EOR markers 
   * NOTE: EOR must never have BLOB_BIT_NULL
   */
  do
   {
    dt=frontierRSBlob_getByte(rs,&ec);
    //std::cout<<"Intermediate type prefix "<<(int)dt<<'\n';
    if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"getAnyData() failed while getting type",ec,-1);
   }while(dt==BLOB_TYPE_EOR);
   
  last_field_type=dt;
  //std::cout<<"Extracted type prefix "<<(int)dt<<'\n';
  
  if(dt&BLOB_BIT_NULL)
   {
    //std::cout<<"The field is NULL\n";
    buf->isNull=1;
    buf->t=dt&(~BLOB_BIT_NULL);
    return 0;
   }
  
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
       if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"can not get byte array length",ec,-1);
       if(len<0) LOGIC_ERROR(this,"negative byte array length",ec,-1);
       p=new char[len+1];
       frontierRSBlob_getArea(rs,p,len,&ec); 
       p[len]=0; // To emulate C string
       buf->set(len,p);
       break;
    default: 
         //std::cout<<"Unknown type prefix "<<(int)dt<<'\n';
         LOGIC_ERROR(this,"unknown type prefix",FRONTIER_EIARG,-1);
   }
  if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"can not get AnyData value",ec,-1);
  return 0;
 }
 
 
BLOB_TYPE DataSource::nextFieldType()
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,-1);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt=frontierRSBlob_checkByte(rs,&ec);
  if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"getAnyData() failed while checking type",ec,-1);
  
  return dt;
 }
 
 
int DataSource::isEOR() 
 {
  return (nextFieldType()==BLOB_TYPE_EOR);
 }
 

int DataSource::getInt()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  return ad.getInt();
 }


long DataSource::getLong()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  
  if(sizeof(long)==8) return ad.getLongLong();  
  return ad.getInt();
 }

long long DataSource::getLongLong()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  
  return ad.getLongLong();  
 }


double DataSource::getDouble()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;  
  return ad.getDouble();
 }
 
 
float DataSource::getFloat()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;  
  return ad.getFloat();
 } 

 
long long DataSource::getDate()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return -1;
  
  return ad.getLongLong();   
 }
 
 
std::string* DataSource::getString()
 {
  AnyData ad;
  
  if(getAnyData(&ad)) return NULL;
  return ad.getString();
 }
 
 
std::string* DataSource::getBlob()
 {
  return getString();
 }

 

DataSource::~DataSource()
 {
  int ec;
  if(internal_data) {frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);internal_data=NULL;}
  frontier_closeChannel(channel);
  if(uri) delete uri;
 }


std::vector<unsigned char>* CDFDataSource::getRawAsArrayUChar()
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


std::vector<int>* CDFDataSource::getRawAsArrayInt()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    LOGIC_ERROR(this,"Blob size is not multiple of 4 - can not convert to int[]",FRONTIER_EIARG,NULL);
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
 
   
std::vector<float>* CDFDataSource::getRawAsArrayFloat()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    LOGIC_ERROR(this,"Blob size is not multiple of 4 - can not convert to float[]",FRONTIER_EIARG,NULL);
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


std::vector<double>* CDFDataSource::getRawAsArrayDouble()
 {
  std::string *blob=getBlob();
  if(blob->size()%8) 
   {
    delete blob; 
    LOGIC_ERROR(this,"Blob size is not multiple of 8 - can not convert to double[]",FRONTIER_EIARG,NULL);
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


std::vector<long>* CDFDataSource::getRawAsArrayLong()
 {
  std::string *blob=getBlob();
  if(blob->size()%4) 
   {
    delete blob; 
    LOGIC_ERROR(this,"Blob size is not multiple of 4 - can not convert to int32[]",FRONTIER_EIARG,NULL);
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

