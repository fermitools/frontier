/*
 * frontier client C++ API header
 * 
 * Author: Sergey Kosyakov
 *
 * $Header$
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
#ifndef __HEADER_H_FRONTIER_FRONTIER_CPP_H_
#define __HEADER_H_FRONTIER_FRONTIER_CPP_H_

#include <string>
#include <vector>
#include <list>

extern "C"
 {
#include "frontier_client/frontier.h"
 }

namespace frontier{

enum encoding_t {BLOB};

class Session;

class Request
 {
  protected:
   friend class Session;
   std::string obj_name;
   encoding_t enc;
   std::vector<std::string> *v_key;
   std::vector<std::string> *v_val;
   int is_meta;

  public:
   Request(const std::string& name,
           const encoding_t& encoding):
          obj_name(name),enc(encoding),v_key(NULL),v_val(NULL),is_meta(0){};
   
   virtual void addKey(const std::string& key,const std::string& value);   
   virtual ~Request();   
   
   static std::string encodeParam(const std::string &value);   
   // set the zip level of retrieved data
   // level 0 is off, level 1 is fast, level 5 is normal, level 9 is best
   // default is 5
   static void setRetrieveZipLevel(int level);
 };

 
class MetaRequest : public Request
 {
  public:
   MetaRequest(const std::string& name,
               const encoding_t& encoding):Request(name,encoding) {is_meta=1;}
   virtual void addKey(const std::string&,const std::string&){}
   virtual ~MetaRequest(){}
 };

 
int init();
// loglevel can be "nolog", "error", "info" or "warning" (which are equivalent),
//  or anything else (which is treated as "debug")
// each level includes all messages at lower levels
int init(const std::string& logfilename, const std::string& loglevel);

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

const char *getFieldTypeName(BLOB_TYPE t);  

class Session;
 
class AnyData
 {
  private:
   friend class Session;
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
   int type_error;   
   BLOB_TYPE t;  // The data type
   Session *sessionp;
   int unused1;  // for binary compatibility -- available for reuse

   int castToInt();
   long long castToLongLong();
   float castToFloat();
   double castToDouble();
   std::string* castToString();
      
  public:     
   AnyData()
     :isNull(0)
     ,type_error(FRONTIER_OK)
     ,t(BLOB_TYPE_NONE)
     {sessionp=0;unused1=0;}
   
   long long getRawI8() const {return v.i8;}
   double getRawD() const {return v.d;}
   const char *getRawStrP() const {return v.str.p;}
   unsigned getRawStrS() const {return v.str.s;}
   int getRawI4() const {return v.i4;}
   float getRawF() const {return v.f;}
   char getRawB() const {return v.b;}
      
   inline void set(int i4){t=BLOB_TYPE_INT4;v.i4=i4;type_error=FRONTIER_OK;}
   inline void set(long long i8){t=BLOB_TYPE_INT8;v.i8=i8;type_error=FRONTIER_OK;}
   inline void set(float f){t=BLOB_TYPE_FLOAT;v.f=f;type_error=FRONTIER_OK;}
   inline void set(double d){t=BLOB_TYPE_DOUBLE;v.d=d;type_error=FRONTIER_OK;}
   inline void set(long long t,int /*time*/){t=BLOB_TYPE_TIME;v.i8=t;type_error=FRONTIER_OK;}
   inline void set(unsigned int size,char *ptr){t=BLOB_TYPE_ARRAY_BYTE;v.str.s=size;v.str.p=ptr;type_error=FRONTIER_OK;}
   inline void setEOR(){t=BLOB_TYPE_EOR;type_error=FRONTIER_OK;}
   inline BLOB_TYPE type() const{return t;}
   inline int isEOR() const{return (t==BLOB_TYPE_EOR);}

   inline int getInt(){if(isNull) return 0;if(t==BLOB_TYPE_INT4) return v.i4; return castToInt();}
   inline long long getLongLong(){if(isNull) return 0; if(t==BLOB_TYPE_INT8 || t==BLOB_TYPE_TIME) return v.i8; return castToLongLong();}
   inline float getFloat(){if(isNull) return 0.0; if(t==BLOB_TYPE_FLOAT) return v.f; return castToFloat();}
   inline double getDouble(){if(isNull) return 0.0;if(t==BLOB_TYPE_DOUBLE) return v.d; return castToDouble();}
   std::string* getString();
   void assignString(std::string *s);
   void clean();
   ~AnyData();
 };
 

class Session;
 
class Connection
 {
  private:
   friend class Session;
   unsigned long channel;

  public:
   
   explicit Connection(const std::string& server_url="",const std::string* proxy_url=NULL);

   // This constructor allows initialization with multiple server/proxies.
   explicit Connection(const std::list<std::string>& serverUrlList, const std::list<std::string>& proxyUrlList);

   virtual ~Connection();
  
   // Set cache time to live for following requested objects.
   // 1=short, 2=long, 3=forever.  Default 2.
   void setTimeToLive(int ttl);

   // Deprecated interface: 0 -> setTimeToLive(2), !0 -> setTimeToLive(1)
   void setReload(int reload);

   // Set default parameters for later-created Connections.
   //  "logicalServer" is the default logical server URL.  Any time
   //  that URL is requested, it will be ignored and instead any other
   //  servers or proxies specified will be used.  "parameterList" is
   //  a concatenated list of parenthesized keyword=value pairs where
   //  keyword is serverurl, proxyurl, or retrieve-ziplevel.  (To be
   //  precise, parameterList may also be just a server URL; this
   //  string and any Connection serverURL other than the
   //  logicalServer are treated exactly the same, as either a single
   //  server if there's no parentheses or as a list of keyword=value
   //  pairs if there are parentheses).  These are added after any
   //  servers or proxies passed to the Connection constructor.
   static void setDefaultParams(const std::string& logicalServer,
   		const std::string& parameterList);
 };
   
class Session
 {
  private:
   unsigned long channel;
   std::string *uri;
   void *internal_data;
   BLOB_TYPE last_field_type;
   char have_saved_byte;
   unsigned char saved_byte;
   char first_row;
   int num_records;

  public:
   
   explicit Session(Connection *myconnection);

   virtual ~Session();

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
   // Benchmarking had shown that inlining functions below does not improve performance
   int getAnyData(AnyData* buf,int not_eor=1);
   int getInt();
   long getLong();
   long long getLongLong();
   double getDouble();
   float getFloat();
   long long getDate();
   std::string *getString();
   std::string *getBlob();
   
   void assignString(std::string *s);
   
   std::vector<unsigned char> *getRawAsArrayUChar();
   std::vector<int> *getRawAsArrayInt();
   std::vector<float> *getRawAsArrayFloat();
   std::vector<double> *getRawAsArrayDouble();
   std::vector<long> *getRawAsArrayLong();   
   
   // Current pyload meta info
   unsigned int getRecNum();
   unsigned int getNumberOfRecords();
   unsigned int getRSBinarySize();
   unsigned int getRSBinaryPos();
   BLOB_TYPE lastFieldType(){return last_field_type;} // Original type of the last extracted field
   BLOB_TYPE nextFieldType(); // Next field type. THIS METHOD DOES NOT CHANGE DS POSITION !!!
   inline int isEOR(){return (nextFieldType()==BLOB_TYPE_EOR);}  // End Of Record. THIS METHOD DOES NOT CHANGE DS POSITION !!!
   inline int isEOF(){return (getRSBinarySize()==getRSBinaryPos());} // End Of File
   int next();
   void clean(); // for use from AnyData::clean()
 };

// DataSource is a combined Session & Connection, for backward compatibility
class DataSource: public Connection, public Session
 {
  public:
   
   explicit DataSource(const std::string& server_url="",const std::string* proxy_url=NULL);

   // This constructor allows initialization with multiple server/proxies.
   explicit DataSource(const std::list<std::string>& serverUrlList, const std::list<std::string>& proxyUrlList);

   virtual ~DataSource();
 };
  
} // namespace frontier


#endif //__HEADER_H_FRONTIER_FRONTIER_CPP_H_

