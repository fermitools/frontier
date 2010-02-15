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
#include <frontier_client/frontier-cpp.h>

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <cstdlib>

#ifndef FNTR_USE_EXCEPTIONS
#define CHECK_ERROR() do{if(ds.err_code){std::cout<<"ERROR:"<<ds.err_msg<<std::endl; exit(1);}}while(0)
#else
#define CHECK_ERROR() 
#endif //FNTR_USE_EXCEPTIONS

int do_main(int argc, char **argv);

static std::string escape_list="\\\'";

static void str_escape_quota(std::string *str)
 {
  std::string::size_type pos;
  
  pos=0;
  while(1)
   {
    pos=str->find_first_of(escape_list,pos);
    if(pos>=str->size()) return;
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

 
int do_main(int argc, char **argv)
 {
  //char vc;
  int vi;
  long long vl;
  float vf;
  double vd;
  std::string *vs;
  int arg_name_ind;
  
#ifdef FNTR_USE_EXCEPTIONS 
  try
   {
#endif //FNTR_USE_EXCEPTIONS
    frontier::init();
    
    if(argc<2)
     {
      std::cout<<"Usage: "<<argv[0]<<" [-r] object_name:v[:m] {key_name key_val {key_name key_val}..}\n";
      exit(1);
     }
     
    //std::cout<<"Requesting \""<<argv[1]<<"\" key \""<<argv[2]<<"\" : \""<<argv[3]<<"\""<<std::endl;

    frontier::DataSource ds;
    CHECK_ERROR();
    
    if(strcmp(argv[1],"-r")==0) 
     {
      arg_name_ind=2;
      ds.setReload(1);
     }
    else
     {
      arg_name_ind=1;
      ds.setReload(0);
     }
    
    frontier::MetaRequest metareq(argv[arg_name_ind],frontier::BLOB);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&metareq);
    ds.getData(vrq);
    CHECK_ERROR();

    ds.setCurrentLoad(1);
    CHECK_ERROR();
    
    int field_num=0;
    
    std::cout<<"\nObject fields:\n";
    
    frontier::AnyData ad;
    
    // MetaData consists of one record with filed names.
    // Let's go over all fields:
    while(!ds.isEOR())
     {
      std::string *name=ds.getString();
      CHECK_ERROR();
      std::string *type=ds.getString();
      CHECK_ERROR();
      ++field_num;
      std::cout<<field_num<<" "<<(*name)<<" "<<(*type)<<std::endl;
      delete type;
      delete name;
     }

    frontier::Request req(argv[arg_name_ind],frontier::BLOB);

    for(int i=arg_name_ind+1;i+1<argc;i+=2)
     {
      req.addKey(argv[i],argv[i+1]);     
     }
    
    vrq[0]=&req;
    ds.getData(vrq);
    CHECK_ERROR();

    ds.setCurrentLoad(1);
    CHECK_ERROR();     
         
    int nrec=ds.getRecNum();
    CHECK_ERROR();
    std::cout<<"\nResult contains "<< nrec<<" objects.\n";
        
    while(ds.next())
     {
      for(int k=0;k<field_num;k++)
       {
        ds.getAnyData(&ad);
        CHECK_ERROR();
        switch(ad.type())
         {
          //case frontier::BLOB_TYPE_BYTE:       vc=ds.getByte(); break;
          case frontier::BLOB_TYPE_INT4:       vi=ad.getInt(); std::cout<<vi; break;
          case frontier::BLOB_TYPE_INT8:       vl=ad.getLongLong(); std::cout<<vl; break;
          case frontier::BLOB_TYPE_FLOAT:      vf=ad.getFloat(); std::cout<<vf; break;
          case frontier::BLOB_TYPE_DOUBLE:     vd=ad.getDouble(); std::cout<<vd; break;
          case frontier::BLOB_TYPE_TIME:       vl=ad.getLongLong(); std::cout<<vl; break;
          case frontier::BLOB_TYPE_ARRAY_BYTE: 
                   vs=ad.getString(); 
                   if(!vs) {std::cout<<"NULL";}
                   else
                    {
                     str_escape_quota(vs);
                     std::cout<<'\''<<(*vs)<<'\''; 
                     delete vs;
                    }
                   break;	  
          default: std::cout<<"Error: unknown typeId "<<((int)(ad.type()))<<"\n"; exit(1);
         }
        if(k+1<field_num) std::cout<<" ";
        ad.clean();
       }
      ad.clean();
      std::cout<<std::endl;
     }
    if(!ds.isEOF())
     {
      std::cout<<"Error: must be EOF here\n";
      exit(1);
     }
#ifdef FNTR_USE_EXCEPTIONS   
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
#endif //FNTR_USE_EXCEPTIONS   
  return 0;
 }

