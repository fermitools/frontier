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

// Enum sucks
typedef unsigned char BLOB_TYPE;
const BLOB_TYPE BLOB_BIT_NULL=(1<<7);

const BLOB_TYPE BLOB_TYPE_NONE=BLOB_BIT_NULL;
const BLOB_TYPE BLOB_TYPE_BYTE=0;
const BLOB_TYPE BLOB_TYPE_INT4=1;
const BLOB_TYPE BLOB_TYPE_INT8=2;
const BLOB_TYPE BLOB_TYPE_FLOAT=3;
const BLOB_TYPE BLOB_TYPE_DOUBLE=4;
const BLOB_TYPE BLOB_TYPE_TIME=5;
const BLOB_TYPE BLOB_TYPE_ARRAY_BYTE=6;
const BLOB_TYPE BLOB_TYPE_EOR=7;
  

class DataSource;
 
// You are not going to use this class directly
class AnyData
 {
  private:
  friend class DataSource;	// I love C++ :-)
   union
    {
     long long i8;
     double d;
     struct{char *p;unsigned int s;}str;    
     int i4;
     float f;    
     char b;
    } v;
   int isNull;   // I do not use "bool" here because of compatibility problems [SSK]   
   BLOB_TYPE t;  // The data type
  
  public:
   explicit AnyData(): isNull(0),t(BLOB_TYPE_NONE){}
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
   BLOB_TYPE last_field_type;

   int getAnyData(AnyData* buf);
   
  public:
   int err_code;
   std::string err_msg;
   
   explicit DataSource(const std::string& server_url="",const std::string* proxy_url=NULL);
   
   // If reload!=0 then all requested objects will be refreshed at all caches
   // New object copy will be obtained directly from server
   void setReload(int reload);
   
   // Get data for Requests
   void getData(const std::vector<const Request*>& v_req);
   
   // Each Request generates a payload. Payload numbers started with 1.
   // So, to get data for the first Request call setCurrentLoad(1)
   void setCurrentLoad(int n);
   
   // Check error for this particular payload.
   // If error happes for any payload that payload and all subsequent payloads (if any) are empty
   int getCurrentLoadError() const;
   // More detailed (hopefully) error explanation.
   const char* getCurrentLoadErrorMessage() const;
   
   // Data fields extractors
   // These methods change DS position to the next field
   int getInt();
   long getLong();
   long long getLongLong();
   double getDouble();
   float getFloat();
   long long getDate();
   std::string *getString();
   std::string *getBlob();
   
   // Meta info
   unsigned int getRecNum();
   BLOB_TYPE lastFieldType(){return last_field_type;} // Original type of the last extracted field
   BLOB_TYPE nextFieldType(); // Next field type. THIS METHOD DOES NOT CHANGE DS POSITION !!!
   int isEOR();  // End Of Record. THIS METHOD DOES NOT CHANGE DS POSITION !!!
   
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

