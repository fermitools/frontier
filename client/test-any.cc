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
#include <frontier_client/frontier-cpp.h>

#include <iostream>
#include <stdexcept>

#ifndef FNTR_USE_EXCEPTIONS
#define CHECK_ERROR() do{if(ds.err_code){std::cout<<"ERROR:"<<ds.err_msg<<std::endl; exit(1);}}while(0)
#else
#define CHECK_ERROR() 
#endif //FNTR_USE_EXCEPTIONS

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
    
    if(argc<4)
     {
      std::cout<<"Usage: "<<argv[0]<<" object_name key_name key_val {key_name key_val}\n";
      exit(1);
     }
     
    //std::cout<<"Requesting \""<<argv[1]<<"\" key \""<<argv[2]<<"\" : \""<<argv[3]<<"\""<<std::endl;

    frontier::CDFDataSource ds;
    CHECK_ERROR();
    
    //ds.setReload(1);

    frontier::MetaRequest metareq(argv[1],"1",frontier::BLOB);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&metareq);
    ds.getData(vrq);
    CHECK_ERROR();

    ds.setCurrentLoad(1);
    CHECK_ERROR();
    
    int field_num=0;
    
    std::cout<<"\nObject fields:\n";
    
    frontier::AnyData ad;
    
    while(!ds.isEOR())
     {
      std::string *name=ds.getString();
      CHECK_ERROR();
      std::string *type=ds.getString();
      CHECK_ERROR();
      std::cout<<++field_num<<" "<<(*name)<<" "<<(*type)<<std::endl;
      delete type;
      delete name;
     }

    frontier::Request req(argv[1],"1",frontier::BLOB,argv[2],argv[3]);

    for(int i=4;i+1<argc;i+=2)
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
        
    for(int n=0;n<nrec;n++)
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
	      if(!vs) 
               {
                std::cout<<"NULL"; 
               }
              else
               {
                std::cout<<'\''<<(*vs)<<'\''; 
                delete vs;
               }
	      break;	  
	  default: std::cout<<"Error: unknown typeId "<<((int)(ad.type()))<<"\n"; exit(1);
	 }
	if(k+1<field_num) std::cout<<" ";
	ad.clean();
       }
      ds.getAnyData(&ad);
      CHECK_ERROR();
      if(!ad.isEOR())
       {
        std::cout<<"Error: must be EOR here\n";
	exit(1);
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


