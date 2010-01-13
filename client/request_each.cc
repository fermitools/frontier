/*
 * frontier client test C++ main program
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
#include <frontier-cpp.h>

#include <iostream>
#include <sstream>
#include <stdexcept>
 

int main(int argc, char **argv)
 {
  if(argc!=3)
   {
    std::cout<<"Usage: "<<argv[0]<<" server_url object_name"<<'\n';
    exit(1);
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1]);
    
    //ds.setReload(1);

    frontier::Request req("frontier_get_cid_list","1",frontier::BLOB,"table_name",argv[2]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    int nrec=ds.getRecNum();
    std::cout<<"Got "<<nrec<<" records back."<<'\n';
    
    std::vector<int> v_cid;
    
    for(int i=0;i<nrec;i++)
     {
      int cid=ds.getLongLong();
      v_cid.insert(v_cid.end(),cid);
     }
     
    for(int i=0;i<nrec;i++)
     {
      std::ostringstream oss;
      oss<<v_cid[i];
      std::string cid=oss.str();
      std::cout<<argv[2]<<" "<<(i+1)<<" of "<<nrec<<'\t'<<cid;      
      frontier::Request req1("calibrunlists","1",frontier::BLOB,"cid",cid);
      frontier::Request req2(argv[2],"1",frontier::BLOB,"cid",cid);
      std::vector<const frontier::Request*> vrq1;
      vrq1.insert(vrq1.end(),&req1);
      vrq1.insert(vrq1.end(),&req2);
      ds.getData(vrq1); 
      ds.setCurrentLoad(2);      
      std::cout<<" nrec "<<ds.getRecNum()<<'\n';
     }
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
 }


