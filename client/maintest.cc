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
  if(argc<6)
   {
    std::cout<<"Usage: "<<argv[0]<<" host port object_name key_name key_value [{object_name key_name key_value} ...]"<<'\n';
    exit(1);
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1],atoi(argv[2]),"/Frontier/","");
    
    //ds.setReload(1);
    
    std::vector<const frontier::Request*> vrq;
    for(int i=3;i+2<argc;i+=3)
     {
      frontier::Request* req=new frontier::Request(argv[i],"1",frontier::BLOB,argv[i+1],argv[i+2]);     
      vrq.insert(vrq.end(),req);
     }
    print_time("start:  ");
    ds.getData(vrq);
    print_time("finish: ");
    
    std::cout<<"URL <"<<ds.getUrl()<<">\n";

    for(int i=1;i<=vrq.size();i++)
     {
      ds.setCurrentLoad(i);
      int nrec=ds.getRecNum();
      std::cout<<"Payload "<<i<<" number of records "<<nrec<<'\n';
      delete vrq[i-1];
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


