/*
 * frontier client test C++ main program that can load any query
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
#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"

#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

int do_main(int argc, char **argv);
static std::string escape_list="\\\'";
static std::string req_data="frontier_request:1:DEFAULT";


static void str_escape_quota(std::string *str)
 {
  std::string::size_type pos;
  
  pos=0;
  while(1)
   {
    pos=str->find_first_of(escape_list,pos);
    if(pos==str->npos) return;
    //std::cout<<"pos="<<pos<<'\n';
    str->insert(pos,1,'\\');    
    pos+=2;
   }
 }

 
int main(int argc, char **argv)
 {
  while(1)
   {
    do_main(argc,argv);
    break;
   }
 }

 
static void print_usage(char **argv)
 {
  std::cout<<"Usage: \n"<<argv[0]<<" -h\n\tPrint this info\n";
  std::cout<<"\n"<<argv[0]<<" [-r|-R] [-n] [-c N] [-F N] [-f file_name]\n";
  std::cout<<"  if -f file_name is missing, reads query from stdin\n";
  std::cout<<"  -r: request short time-to-live\n";
  std::cout<<"  -R: request forever time-to-live\n";
  std::cout<<"  -n: do not print data\n";
  std::cout<<"  -c N: repeat the query N count times\n";
  std::cout<<"  -F N: fork after Nth repetition\n";
 }
 
int do_main(int argc, char **argv)
 {
  //char vc;
  int vi;
  long long vl;
  float vf;
  double vd;
  std::string *vs=0;
  frontier::AnyData ad;
  char *file_name=0;
  int arg_ind;
  int ttl=2;
  int do_print=1;
  int repeat_count=1;
  int fork_count=0;
  int idx;
  std::string sql("");
  
  try
   {
    frontier::init();
    
    arg_ind=1;
    while(arg_ind<argc)
     {
      if(strcmp(argv[arg_ind],"-h")==0)
       {
        print_usage(argv);
        exit(0);
       }
      if(strcmp(argv[arg_ind],"-r")==0)
        ttl=1;
      else if(strcmp(argv[arg_ind],"-R")==0)
        ttl=3;
      else if(strcmp(argv[arg_ind],"-n")==0)
        do_print=0;
      else if(argc>(arg_ind+1) && strcmp(argv[arg_ind],"-c")==0)
       {
        repeat_count=atoi(argv[arg_ind+1]);
	arg_ind++;
       }
      else if(argc>(arg_ind+1) && strcmp(argv[arg_ind],"-F")==0)
       {
        fork_count=atoi(argv[arg_ind+1]);
	arg_ind++;
       }
      else if(argc>(arg_ind+1) && strcmp(argv[arg_ind],"-f")==0)
        file_name=argv[arg_ind+1];
      else if(!file_name && argc>arg_ind)
       {
        print_usage(argv);
        exit(1);
       }
      arg_ind++;
     }
     
    std::ifstream in_file;
    if(file_name)
     {
      in_file.open(file_name);
      if(!in_file.is_open())
       {
        std::cout<<"Can not open file "<<file_name<<'\n';
        exit(2);
       }
     }
    while(1)
     {
      std::string tmp;      
      if(file_name)
       {
        if(!in_file.good()) break;
        std::getline(in_file,tmp,'\n');
       }
      else
       {
        if(!std::cin.good()) break;
        std::getline(std::cin,tmp,'\n');       
       }
      sql+=tmp;
     }
    if(file_name) {in_file.close();}
    std::cout<<"Entered:\n"<<sql<<'\n';
    
    std::string param=frontier::Request::encodeParam(sql);
    std::cout<<"Param ["<<param<<"]\n";
          
    std::list<std::string> serverList;
    //serverList.push_back("http://lxfs6043.cern.ch:8080/Frontier3D");
    std::list<std::string> proxyList;
    //frontier::DataSource ds(serverList, proxyList);
    frontier::Connection con(serverList, proxyList);

    for(idx=0;idx<repeat_count;idx++)
     {
      if((fork_count>0)&&(idx==fork_count))
        fork();

      frontier::Session ses(&con);
      con.setTimeToLive(ttl);

      frontier::Request req(req_data,frontier::BLOB);
      req.addKey("p1",param);

      std::vector<const frontier::Request*> vrq;
      vrq.push_back(&req);
      ses.getData(vrq);

      ses.setCurrentLoad(1);
      
      int field_num=0;
      
      std::cout<<"\nObject fields:\n";
      
      ses.next();
      // MetaData consists of one record with field names.
      // Let's go over all fields:
      std::string name,type;
      
      while(!ses.isEOR()) {
	ses.assignString(&name);
	ses.assignString(&type);
	++field_num;
	std::cout<<field_num<<" "<<(name)<<" "<<(type)<<std::endl;
      }
	   
      int nrec=ses.getNumberOfRecords();
      std::cout<<"\nResult contains "<< nrec<<" objects.\n";
	  
      while(ses.next()) {
	if(!do_print)continue;
	for(int k=0;k<field_num;k++) {
	  ses.getAnyData(&ad);
	  switch(ad.type()) {
	    //case frontier::BLOB_TYPE_BYTE:       vc=ses.getByte(); break;
	    case frontier::BLOB_TYPE_INT4:       
	      vi=ad.getInt(); 
	      std::cout<<vi; 
	      break;
	    case frontier::BLOB_TYPE_INT8:       
	      vl=ad.getLongLong(); 
	      std::cout<<vl; 
	      break;
	    case frontier::BLOB_TYPE_FLOAT:      
	      vf=ad.getFloat(); 
	      std::cout<<vf; 
	      break;
	    case frontier::BLOB_TYPE_DOUBLE:     
	      vd=ad.getDouble(); 
	      std::cout<<vd; 
	      break;
	    case frontier::BLOB_TYPE_TIME:       
	      vl=ad.getLongLong(); 
	      std::cout<<vl; 
	      break;
	    case frontier::BLOB_TYPE_ARRAY_BYTE: 
	      if(!ad.getRawStrP()) {
		std::cout<<"NULL";
	      }
	      else if (ad.getRawStrS() == 0)
		std::cout<<"''"; 
	      else if (ad.getRawStrS() > 1000)
		std::cout<<'('<<ad.getRawStrS()<<" byte blob)"; 
	      else {
		vs=ad.getString(); 
		str_escape_quota(vs);
		std::cout<<'\''<<(*vs)<<'\''<<'('<<ad.getRawStrS()<<')'; 
		delete vs;
	      }
	      break;	  
	    default: 
	      std::cout<<"Error: unknown typeId "<<((int)(ad.type()))<<"\n"; 
	      exit(1);
	  }
	  if(k+1<field_num) {
	    std::cout<<" ";
	  }
	  ad.clean();
	}
	ad.clean();
	std::cout<<std::endl;
      }
      if(!ses.isEOF()) {
	std::cout<<"Error: must be EOF here\n";
	exit(1);
      }
    }
  }
  catch(const frontier::ConfigurationError& e) {
    std::cout << "Frontier configuration error caught: " << e.what() << std::endl;
    exit(1);
  }
  catch(const frontier::FrontierException& e) {
    std::cout << "Frontier exception caught: " << e.what() << std::endl;
    exit(1);
  }
  catch(std::exception& e)
   {
    std::cout << "Error: " << e.what() << "\n";
    exit(1);
   }
  catch(...)
   {
    std::cout << "Unknown exception\n";
    exit(1);
   }

  return 0;
 }


