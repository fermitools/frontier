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

#include <frontier_client/frontier-cpp.h>
#include <stdlib.h>
#include <sstream>

extern "C"
{
extern void *(*frontier_mem_alloc)(size_t size);
extern void (*frontier_mem_free)(void *ptr);
};

// This chunk of code below is ugly, but this is the way things 
// were done historically (C++/KCC and other junk); 
// here is C++ in all its glory (incompatibility on binary objects level). 
// So here is no even a tiny possibility to write a good code. I tried :-(

static std::string create_err_msg(const char *str)
 {
  return std::string(str)+std::string(": ")+std::string(frontier_getErrorMsg());
 }

#include <stdexcept>
#define RUNTIME_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=create_err_msg(m); throw std::runtime_error(o->err_msg);}while(0)
#define LOGIC_ERROR(o,m,e,r) do{o->err_code=e; o->err_msg=create_err_msg(m); throw std::logic_error(o->err_msg);}while(0)
#define RUNTIME_ERROR_NR(o,m,e) RUNTIME_ERROR(o,m,e,-1)
#define LOGIC_ERROR_NR(o,m,e) LOGIC_ERROR(o,m,e,-1)

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
    throw std::runtime_error(oss.str());
   }
   
  std::string ret(buf,len);
  frontier_mem_free(buf);
  return ret;
 }
 
 
 
int frontier::init()
 {
  int ret;

#ifdef FN_MEMORY_DEBUG
  ret=frontier_init(frontier_malloc,frontier_free);
#else  
  ret=frontier_init(malloc,free);
#endif //FN_MEMORY_DEBUG
  if(ret) throw std::runtime_error(create_err_msg("Frontier initialization failed"));
  return ret;
 }


 
DataSource::DataSource(const std::string& server_url,const std::string* proxy_url)
 {
  int ec=FRONTIER_OK;
  const char *proxy_url_c=NULL;
  first_row=0;
  
  init();
  
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
  oss<<"Frontier";
  char delim='?';
 
  for(std::vector<const Request*>::size_type i=0;i<v_req.size();i++)
   {
    const char *enc;
    switch(v_req[i]->enc)
     {
      case BLOB: enc="BLOB"; break;
      default: LOGIC_ERROR_NR(this,"Unknown encoding requested",FRONTIER_EIARG);
     }    
    if(v_req[i]->is_meta)
     {
      oss << delim << "meta=" << v_req[i]->obj_name; delim='&';
     }
    else
     {
      oss << delim << "type=" << v_req[i]->obj_name; delim='&';
     }
    oss << delim << "encoding=" << enc;
    
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
  first_row=0;
  FrontierRSBlob *rsb=frontierRSBlob_get(channel,n,&ec);
  if(ec!=FRONTIER_OK) LOGIC_ERROR_NR(this,"Can not set current load",ec);
  
  if(internal_data) frontierRSBlob_close(static_cast<FrontierRSBlob*>(internal_data),&ec); //Doesn't it look UGLY?
  
  internal_data=rsb;
  if(getCurrentLoadError()!=FRONTIER_OK) LOGIC_ERROR_NR(this,getCurrentLoadErrorMessage(),FRONTIER_EPROTO);
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
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,0);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return rs->nrec;
 }

// Get number of records.
unsigned int DataSource::getNumberOfRecords() {
  if(!internal_data) {
    LOGIC_ERROR(this, "Current load is not set", FRONTIER_EIARG, 0);
  }
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return rs->nrec;
}

 
unsigned int DataSource::getRSBinarySize()
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,0);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return rs->size;  
 }

 
unsigned int DataSource::getRSBinaryPos()
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,0);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  return rs->pos;  
 }
  
 
int DataSource::getAnyData(AnyData* buf,int not_eor)
 {
  buf->isNull=0;
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,-1);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt;
  
  if(not_eor && isEOR()) LOGIC_ERROR(this,"EOR has been reached",FRONTIER_EIARG,-1);
  
  dt=frontierRSBlob_getByte(rs,&ec);
  //std::cout<<"Intermediate type prefix "<<(int)dt<<'\n';
  if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"getAnyData() failed while getting type",ec,-1);
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
       if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"can not get byte array length",ec,-1);
       if(len<0) LOGIC_ERROR(this,"negative byte array length",ec,-1);
       p=new char[len+1];
       frontierRSBlob_getArea(rs,p,len,&ec); 
       p[len]=0; // To emulate C string
       buf->set(len,p);
       break;
    case BLOB_TYPE_EOR: buf->setEOR(); break;
    default: 
         //std::cout<<"Unknown type prefix "<<(int)dt<<'\n';
         LOGIC_ERROR(this,"unknown type prefix",FRONTIER_EIARG,-1);
   }
  if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"can not get AnyData value",ec,-1);
  return 0;
 }
 
 
BLOB_TYPE DataSource::nextFieldType()
 {
  if(!internal_data) LOGIC_ERROR(this,"Current load is not set",FRONTIER_EIARG,0);
  FrontierRSBlob *rs=(FrontierRSBlob*)internal_data;
  int ec=FRONTIER_OK;
  BLOB_TYPE dt=frontierRSBlob_checkByte(rs,&ec);
  if(ec!=FRONTIER_OK) LOGIC_ERROR(this,"getAnyData() failed while checking type",ec,0);
  
  return dt;
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

 
 
void DataSource::assignString(std::string *s)
 {
  AnyData ad;

  if(getAnyData(&ad))
   {
    *s="";
    return;
   }

  ad.assignString(s);
 }



int DataSource::next()
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

 
 
DataSource::~DataSource()
 {
  int ec;
  if(internal_data) {frontierRSBlob_close((FrontierRSBlob*)internal_data,&ec);internal_data=NULL;}
  frontier_closeChannel(channel);
  if(uri) delete uri;
 }


std::vector<unsigned char>* DataSource::getRawAsArrayUChar()
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


std::vector<int>* DataSource::getRawAsArrayInt()
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
 
   
std::vector<float>* DataSource::getRawAsArrayFloat()
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


std::vector<double>* DataSource::getRawAsArrayDouble()
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


std::vector<long>* DataSource::getRawAsArrayLong()
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

