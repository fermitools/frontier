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
  //char vc;
  int vi;
  long long vl;
  float vf;
  double vd;
  std::string *vs;
  
#ifdef FNTR_USE_EXCEPTIONS 
  try
   {
#endif //FNTR_USE_EXCEPTIONS
    frontier::init();
    
    if(argc!=4)
     {
      std::cout<<"Usage: "<<argv[0]<<" object_name key_name key_val\n";
      exit(1);
     }
     
    std::cout<<"Requesting \""<<argv[1]<<"\" key \""<<argv[2]<<"\" : \""<<argv[3]<<"\""<<std::endl;

    frontier::CDFDataSource ds;
    
    //ds.setReload(1);

    frontier::Request req1(argv[1],"1",frontier::BLOB,argv[2],argv[3]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req1);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    
    int field_num=0;
    
    std::cout<<"\nObject field types:\n";
    
    frontier::AnyData ad;
    
    while(1)
     {
      ds.getAnyData(&ad);
      if(ad.isEOR()) break;
      std::cout<<++field_num<<" "<<frontier::getFieldTypeName(ad.type())<<std::endl;
      ad.clean();
     }
    
    int nrec=ds.getRecNum();
    std::cout<<"\nResult contains "<< nrec<<" objects.\n";
        
    ds.setCurrentLoad(1);

    for(int n=0;n<nrec;n++)
     {
      for(int k=0;k<field_num;k++)
       {
        ds.getAnyData(&ad);
        switch(ad.type())
         {
          //case frontier::BLOB_TYPE_BYTE:       vc=ds.getByte(); break;
          case frontier::BLOB_TYPE_INT4:       vi=ad.getInt(); std::cout<<vi; break;
          case frontier::BLOB_TYPE_INT8:       vl=ad.getLongLong(); std::cout<<vl; break;
          case frontier::BLOB_TYPE_FLOAT:      vf=ad.getFloat(); std::cout<<vf; break;
          case frontier::BLOB_TYPE_DOUBLE:     vd=ad.getDouble(); std::cout<<vd; break;
          case frontier::BLOB_TYPE_TIME:       vl=ad.getLongLong(); std::cout<<vl; break;
          case frontier::BLOB_TYPE_ARRAY_BYTE: vs=ad.getString(); std::cout<<(*vs); delete vs; break;	  
	  default: std::cout<<"Error: unknown typeId "<<((int)(ad.type()))<<"\n"; exit(1);
	 }
	if(k+1<field_num) std::cout<<" ";
	ad.clean();
       }
      ds.getAnyData(&ad);
      if(!ad.isEOR())
       {
        std::cout<<"Error: must be EOR here\n";
	exit(1);
       }
      ad.clean();
      std::cout<<std::endl;
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


