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

/*
select 
address,
gain,
sourcemean,
sourcerms,
sourcenorm,
lasernorm,
linearity1,
linearity2,
linearity3,
attenuation1,
attenuation2,
attenuation3 
from pescalib where cid = ?
*/

class PESCalib
 {
  public:
   long long address;
   float gain;
   float sourcemean;
   float sourcerms;
   float sourcenorm;
   float lasernorm;
   float linearity1;
   float linearity2;
   float linearity3;
   float attenuation1;
   float attenuation2;
   float attenuation3;

  PESCalib(frontier::CDFDataSource& ds)
   {
    address=ds.getLongLong();
    gain=ds.getFloat();
    sourcemean=ds.getFloat();
    sourcerms=ds.getFloat();
    sourcenorm=ds.getFloat();
    lasernorm=ds.getFloat();
    linearity1=ds.getFloat();
    linearity2=ds.getFloat();
    linearity3=ds.getFloat();
    attenuation1=ds.getFloat();
    attenuation2=ds.getFloat();
    attenuation3=ds.getFloat();
   }
   
  void print()
   {
    std::cout<<address<<' ';
    std::cout<<gain<<' ';
    std::cout<<sourcemean<<' ';
    std::cout<<sourcerms<<' ';
    std::cout<<sourcenorm<<' ';
    std::cout<<lasernorm<<' ';
    std::cout<<linearity1<<' ';
    std::cout<<linearity2<<' ';
    std::cout<<linearity3<<' ';
    std::cout<<attenuation1<<' ';
    std::cout<<attenuation2<<' ';
    std::cout<<attenuation3<<'\n';
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
    
    //ds.setReload(1);

    frontier::Request req1("PESCalib","1",frontier::BLOB,"cid",argv[1]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req1);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    
    int nrec=ds.getRecNum();
    std::cout<<"CID <"<<argv[1]<<"> nrec "<< nrec<<'\n';
    
    std::vector<PESCalib*> v_sbp(nrec);
    for(int i=0;i<nrec;i++)
     {
      v_sbp[i]=new PESCalib(ds);
      v_sbp[i]->print();
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


