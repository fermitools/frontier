/*
 * frontier client pescalib standalone test program
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
    address=ds.getInt();
    gain=ds.getDouble();
    sourcemean=ds.getDouble();
    sourcerms=ds.getDouble();
    sourcenorm=ds.getDouble();
    lasernorm=ds.getDouble();
    linearity1=ds.getDouble();
    linearity2=ds.getDouble();
    linearity3=ds.getDouble();
    attenuation1=ds.getDouble();
    attenuation2=ds.getDouble();
    attenuation3=ds.getDouble();
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
  for(int i=0;i<1;i++)
   {
    //std::cout<<i<<'\n';
    do_main(argc,argv);
   }
 }

 
int do_main(int argc, char **argv)
 {
  char *cid;
 
#ifdef FNTR_USE_EXCEPTIONS 
  try
   {
#endif //FNTR_USE_EXCEPTIONS
    frontier::init();
    
    frontier::CDFDataSource ds;
    
    //ds.setReload(1);

    if(argc==2) cid=argv[1];
    else cid="91271";
    
    for(int n=0;n<1;n++)
     {    
      frontier::Request req1("PESCalib:1",frontier::BLOB,"cid",cid);

      std::vector<const frontier::Request*> vrq;
      vrq.insert(vrq.end(),&req1);
      ds.getData(vrq); 

      ds.setCurrentLoad(1);
    
      int nrec=ds.getRecNum();
      std::cout<<"CID <"<<cid<<"> nrec "<< nrec<<'\n';
    
      std::vector<PESCalib*> v_sbp(nrec);
      for(int i=0;i<nrec;i++)
       {
        v_sbp[i]=new PESCalib(ds);
        v_sbp[i]->print();
       }
    
       // Do some usefull things here ...

      // Clean
      for(int i=0;i<nrec;i++) delete v_sbp[i]; 
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


