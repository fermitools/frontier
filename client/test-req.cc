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
#include "frontier_client/frontier-cpp.h"
#include "frontier_client/FrontierException.hpp"

#include <iostream>
#include <fstream>
#include <stdexcept>

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
  std::cout<<"\n"<<argv[0]<<" [-r] -f file_name\n\tRead query from file_name\n";
  std::cout<<"\n"<<argv[0]<<" [-r] \n\tRead query from stdin\n";
 }
 
int do_main(int argc, char **argv)
 {
  //char vc;
  int vi;
  long long vl;
  float vf;
  double vd;
  std::string *vs;
  char *file_name=0;
  int arg_ind;
  int do_reload=0;
  std::string sql("");
  
  try
   {
    frontier::init();
    
    arg_ind=1;
    if(argc>=2)
     {
      if(strcmp(argv[1],"-h")==0)
       {
        print_usage(argv);
        exit(0);
       }
      if(strcmp(argv[1],"-r")==0)
       {
        do_reload=1;
        arg_ind=2;
       }
      if(argc>(arg_ind+1) && strcmp(argv[arg_ind],"-f")==0)
       {
        file_name=argv[arg_ind+1];
       }
      if(!file_name && argc>arg_ind)
       {
        print_usage(argv);
        exit(1);
       }
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
      sql+=tmp+'\n';
     }
    if(file_name) {in_file.close();}
    std::cout<<"Entered:\n"<<sql;
    
    std::string param=frontier::Request::encodeParam(sql);
    std::cout<<"Param ["<<param<<"]\n";
          
    std::list<std::string> serverList;
    serverList.push_back("http://lxfs6043.cern.ch:8080/Frontier3D");
    std::list<std::string> proxyList;
    //frontier::DataSource ds;        
    frontier::DataSource ds(serverList, proxyList);
    frontier::Request req(req_data,frontier::BLOB);
    req.addKey("p1",param);

    std::vector<const frontier::Request*> vrq;
    vrq.push_back(&req);
    ds.getData(vrq);

    ds.setCurrentLoad(1);
    
    int field_num=0;
    
    std::cout<<"\nObject fields:\n";
    
    frontier::AnyData ad;
    
    ds.next();
    // MetaData consists of one record with filed names.
    // Let's go over all fields:
    std::string name,type;
    
    while(!ds.isEOR()) {
      ds.assignString(&name);
      ds.assignString(&type);
      ++field_num;
      std::cout<<field_num<<" "<<(name)<<" "<<(type)<<std::endl;
    }
         
    int nrec=ds.getNumberOfRecords();
    std::cout<<"\nResult contains "<< nrec<<" objects.\n";
        
    while(ds.next()) {
      for(int k=0;k<field_num;k++) {
        ds.getAnyData(&ad);
        switch(ad.type()) {
          //case frontier::BLOB_TYPE_BYTE:       vc=ds.getByte(); break;
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
            vs=ad.getString(); 
            if(!vs) {
              std::cout<<"NULL";
            }
            else {
              str_escape_quota(vs);
              std::cout<<'\''<<(*vs)<<'\''; 
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
    if(!ds.isEOF()) {
      std::cout<<"Error: must be EOF here\n";
      exit(1);
    }
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


