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


class TOFTacParm
 {
  public:
   int geomid;
   int parmType;
   double par0;
   double par1;
   double par2;
   double par3;
   std::vector<float> *cov;

  TOFTacParm(frontier::CDFDataSource& ds)
   {
    geomid=ds.getInt();
    parmType=ds.getInt();
    par0=ds.getDouble();
    par1=ds.getDouble();
    par2=ds.getDouble();
    par3=ds.getDouble();
    cov=ds.getRawAsArrayFloat();
   }
   
  ~TOFTacParm()
   {
    delete cov;
   }
   
  void print()
   {
    unsigned i;
    
    std::cout<<geomid<<' ';
    std::cout<<parmType<<' ';
    std::cout<<par0<<' ';
    std::cout<<par1<<' ';
    std::cout<<par2<<' ';
    std::cout<<par3<<" [";
    
    for(i=0;i<cov->size();i++) std::cout<<cov->operator[](i)<<' ';
    
    std::cout<<']';
   }
 };

int do_main(int argc, char **argv);

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
 
#ifdef FNTR_USE_EXCEPTIONS 
  try
   {
#endif //FNTR_USE_EXCEPTIONS
    frontier::init();
    
    if(argc!=2)
     {
      std::cout<<"Usage: "<<argv[0]<<" cid_num\n";
      exit(1);
     }

    frontier::CDFDataSource ds;
    
    ds.setReload(1);

    frontier::Request req1("TOFTacParm","1",frontier::BLOB,"cid",argv[1]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req1);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    
    int nrec=ds.getRecNum();
    std::cout<<"CID <"<<argv[1]<<"> nrec "<< nrec<<'\n';
    
    std::vector<TOFTacParm*> v_sbp(nrec);
    for(int i=0;i<nrec;i++)
     {
      v_sbp[i]=new TOFTacParm(ds);
      v_sbp[i]->print();
      std::cout<<'\n';
     }
    
    // Do some usefull things here ...

    // Clean
    for(int i=0;i<nrec;i++) delete v_sbp[i]; 
    
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


