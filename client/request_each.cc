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
#include <stdexcept>
 

int main(int argc, char **argv)
 {
  if(argc!=4)
   {
    std::cout<<"Usage: "<<argv[0]<<" host port object_name"<<'\n';
    exit(1);
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1],atoi(argv[2]),"/Frontier/","");
    
    //ds.setReload(1);

    frontier::Request req("frontier_get_cid_list","1",frontier::BLOB,"table_name",argv[3]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    int nrec=ds.getRecNum();
    std::cout<<"Got "<<nrec<<" records back."<<'\n';
    
    std::vector<long> v_cid;
    
    for(int i=0;i<nrec;i++)
     {
      long cid=ds.getLong();
      v_cid.insert(v_cid.end(),cid);
     }
     
    for(int i=0;i<nrec;i++)
     {
      std::ostringstream oss;
      oss<<v_cid[i];
      std::string cid=oss.str();
      std::cout<<argv[3]<<" "<<(i+1)<<" of "<<nrec<<'\t'<<cid;      
      frontier::Request req1("calibrunlists","1",frontier::BLOB,"cid",cid);
      frontier::Request req2(argv[3],"1",frontier::BLOB,"cid",cid);
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


