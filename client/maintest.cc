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

#include <sys/time.h>


void print_time(const char *msg)
 {
  struct timeval tv;
  
  int ret=gettimeofday(&tv,NULL);
  if(ret) std::cout<<"Can not get current time :-(\n";
  
  std::cout<<msg<<tv.tv_sec;
  int msec=tv.tv_usec/1000;
  if(msec<10) std::cout<<"00";
  else if(msec<100) std::cout<<"0";
  std::cout<<msec<<'\n';
 } 


int main(int argc, char **argv)
 {   
  if(argc!=6)
   {
    std::cout<<"Usage: "<<argv[0]<<" host port object_name key_name key_value"<<'\n';
    exit(1);
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1],atoi(argv[2]),"/Frontier/","");
    
    //ds.setReload(1);

    frontier::Request req(argv[3],"1",frontier::BLOB,argv[4],argv[5]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req);
    print_time("start:  ");
    ds.getData(vrq);
    print_time("finish: ");

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


