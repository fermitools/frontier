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


void init();


class DataSource
 {
  private:
   std::string host;
   int port;
   std::string app_path;
   std::string proxy;
   unsigned long channel;
   std::string *url;
   void *internal_data;

  public:
   explicit 
   DataSource(const std::string& host_name,
              int port_number,
              const std::string& application_path,
              const std::string& proxy_url);
   void getData(const std::vector<const Request*>& v_req);
   void setCurrentLoad(int n);
   unsigned int getRecNum();
   int getInt();
   long long getLong();
   double getDouble();
   std::string *getString();
   std::string *getBlob();
   virtual ~DataSource();
 };


class CDFDataSource:public virtual DataSource
 {
  public:
   explicit
   CDFDataSource(const std::string& host_name,
                 int port_number,
                 const std::string& application_path,
                 const std::string& proxy_url):DataSource(host_name,port_number,application_path,proxy_url){}
   std::vector<unsigned char> *getRawAsArrayUChar();
   std::vector<int> *getRawAsArrayInt();
   std::vector<float> *getRawAsArrayFloat();
   std::vector<double> *getRawAsArrayDouble();
   std::vector<long long> *getRawAsArrayLong();
 };
 
 

}; // namespace frontier


#endif //__HEADER_H_FRONTIER_FRONTIER_CPP_H_

