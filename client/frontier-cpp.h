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
#ifndef __HEADER_H_FRONTIER_FRONTIER_CPP_H_
#define __HEADER_H_FRONTIER_FRONTIER_CPP_H_

#include <string>
#include <vector>

namespace frontier{

enum encoding_t {BLOB};

class DataSource;

class Request
 {
  private:
   friend class DataSource;
   std::string obj_name;
   std::string v;
   encoding_t enc;
   std::string key1;
   std::string val1;
   std::vector<std::string> *v_key;
   std::vector<std::string> *v_val;

  public:
   explicit 
   Request(const std::string& name,
           const std::string& version,
           const encoding_t& encoding,
           const std::string& key,
           const std::string& value):
          obj_name(name),v(version),enc(encoding),key1(key),val1(value),v_key(NULL),v_val(NULL){};
   void addKey(const std::string& key,const std::string& value);
   virtual ~Request();
 };


int init();

enum BLOB_TYPE
 {
  BLOB_TYPE_NONE=-1,
  BLOB_TYPE_BYTE=0,
  BLOB_TYPE_INT4=1,
  BLOB_TYPE_INT8=2,
  BLOB_TYPE_FLOAT=3,
  BLOB_TYPE_DOUBLE=4,
  BLOB_TYPE_TIME=5,
  BLOB_TYPE_ARRAY_BYTE=6
 };

class AnyData
 {
  private:
   union
    {
     long long i8;
     double d;
     struct{unsigned int s;char *p;}str;    
     int i4;
     float f;    
     char b;
    } v;
   BLOB_TYPE t;  // The data type   
  
  public:
   explicit AnyData(): t(BLOB_TYPE_NONE){}
   void set(int i4){t=BLOB_TYPE_INT4;v.i4=i4;}
   void set(long long i8){t=BLOB_TYPE_INT8;v.i8=i8;}
   void set(float f){t=BLOB_TYPE_FLOAT;v.f=f;}
   void set(double d){t=BLOB_TYPE_DOUBLE;v.d=d;}
   void set(long long t,int time){t=BLOB_TYPE_TIME;v.i8=t;}
   void set(unsigned int size,char *ptr){t=BLOB_TYPE_ARRAY_BYTE;v.str.s=size;v.str.p=ptr;}
   const BLOB_TYPE type(){return t;}
   ~AnyData(){if(t==BLOB_TYPE_ARRAY_BYTE && v.str.p) {delete[] v.str.p; v.str.p=NULL;}} // Thou art warned!!!
   
   int getInt();
   long long getLongLong();
   float getFloat();
   double getDouble();
   std::string* getString();
 };
 

class DataSource
 {
  private:
   unsigned long channel;
   std::string *uri;
   void *internal_data;

   int getAnyData(AnyData* buf);
   
  public:
   int err_code;
   std::string err_msg;
   
   explicit DataSource(const std::string& server_url="",const std::string* proxy_url=NULL);
   
   void setReload(int reload);
   void getData(const std::vector<const Request*>& v_req);
   void setCurrentLoad(int n);
   int getCurrentLoadError() const;
   const char* getCurrentLoadErrorMessage() const;
   unsigned int getRecNum();
   int getInt();
   long getLong();
#ifndef KCC_COMPILE
   // Any better idea?
   long long getLongLong();
#endif //KCC_COMPILE
   double getDouble();
   std::string *getString();
   std::string *getBlob();
   
   virtual ~DataSource();
 };


class CDFDataSource:public virtual DataSource
 {
  public:
   explicit CDFDataSource(const std::string& server_url="",const std::string* proxy_url=NULL):DataSource(server_url,proxy_url){}
   
   std::vector<unsigned char> *getRawAsArrayUChar();
   std::vector<int> *getRawAsArrayInt();
   std::vector<float> *getRawAsArrayFloat();
   std::vector<double> *getRawAsArrayDouble();
   std::vector<long> *getRawAsArrayLong();
 };
 
 

}; // namespace frontier


#endif //__HEADER_H_FRONTIER_FRONTIER_CPP_H_

