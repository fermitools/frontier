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
#include <stdexcept>
 

int main(int argc, char **argv)
 {
  if(argc!=6)
   {
    std::cout<<"Usage: "<<argv[0]<<" host port object_name key_name key_value"<<'\n';
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1],atoi(argv[2]),"/Frontier/","");

    frontier::Request req(argv[3],"1",frontier::BLOB,argv[4],argv[5]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    int nrec=ds.getRecNum();
    std::cout<<"Got "<<nrec<<" records back."<<'\n';
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


