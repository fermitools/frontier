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

class SvxBeamPosition
 {
  public:
  long long CID;
  long long CHANNELID;
  double BEAMX;
  double BEAMY;
  double SLOPEX;
  double SLOPEY;
  double WIDTHX;
  double WIDTHY;
  double WIDTHXY;
  double BEAMZ;
  double WIDTHZ;
  double BEAMXRATE;
  double BEAMYRATE;
  double SLOPEXRATE;
  double SLOPEYRATE;
  double WIDTHXRATE;
  double WIDTHYRATE;
  double WIDTHXYRATE;
  double BEAMZRATE;
  double WIDTHZRATE;
  double MINIMZFIT;
  double WIDTHZFIT;
  double BSTARZFIT;
  double ZZEROZFIT;
  double MINIMZFITRATE;
  double WIDTHZFITRATE;
  double BSTARZFITRATE;
  double ZZEROZFITRATE;
  double EMITTXFIT;
  double BSTARXFIT;
  double ZZEROXFIT;
  double EMITTYFIT;
  double BSTARYFIT;
  double ZZEROYFIT;
  double EMITTXRATE;
  double BSTARXRATE;
  double ZZEROXRATE;
  double EMITTYRATE;
  double BSTARYRATE;
  double ZZEROYRATE;
  double FITCOV00;
  double FITCOV01;
  double FITCOV02;
  double FITCOV03;
  double FITCOV11;
  double FITCOV12;
  double FITCOV13;
  double FITCOV22;
  double FITCOV23;
  double FITCOV33;
  double STATISTICS0;
  double STATISTICS1;
  long long FLAG0;
  long long FLAG1;
  double SPARE0;
  double SPARE1;
  double SPARE2;
  double SPARE3;

  SvxBeamPosition(frontier::CDFDataSource& ds)
   {
    CID=ds.getLongLong();
    CHANNELID=ds.getLongLong();
    BEAMX=ds.getDouble();
    BEAMY=ds.getDouble();
    SLOPEX=ds.getDouble();
    SLOPEY=ds.getDouble();
    WIDTHX=ds.getDouble();
    WIDTHY=ds.getDouble();
    WIDTHXY=ds.getDouble();
    BEAMZ=ds.getDouble();
    WIDTHZ=ds.getDouble();
    BEAMXRATE=ds.getDouble();
    BEAMYRATE=ds.getDouble();
    SLOPEXRATE=ds.getDouble();
    SLOPEYRATE=ds.getDouble();
    WIDTHXRATE=ds.getDouble();
    WIDTHYRATE=ds.getDouble();
    WIDTHXYRATE=ds.getDouble();
    BEAMZRATE=ds.getDouble();
    WIDTHZRATE=ds.getDouble();
    MINIMZFIT=ds.getDouble();
    WIDTHZFIT=ds.getDouble();
    BSTARZFIT=ds.getDouble();
    ZZEROZFIT=ds.getDouble();
    MINIMZFITRATE=ds.getDouble();
    WIDTHZFITRATE=ds.getDouble();
    BSTARZFITRATE=ds.getDouble();
    ZZEROZFITRATE=ds.getDouble();
    EMITTXFIT=ds.getDouble();
    BSTARXFIT=ds.getDouble();
    ZZEROXFIT=ds.getDouble();
    EMITTYFIT=ds.getDouble();
    BSTARYFIT=ds.getDouble();
    ZZEROYFIT=ds.getDouble();
    EMITTXRATE=ds.getDouble();
    BSTARXRATE=ds.getDouble();
    ZZEROXRATE=ds.getDouble();
    EMITTYRATE=ds.getDouble();
    BSTARYRATE=ds.getDouble();
    ZZEROYRATE=ds.getDouble();
    FITCOV00=ds.getDouble();
    FITCOV01=ds.getDouble();
    FITCOV02=ds.getDouble();
    FITCOV03=ds.getDouble();
    FITCOV11=ds.getDouble();
    FITCOV12=ds.getDouble();
    FITCOV13=ds.getDouble();
    FITCOV22=ds.getDouble();
    FITCOV23=ds.getDouble();
    FITCOV33=ds.getDouble();
    STATISTICS0=ds.getDouble();
    STATISTICS1=ds.getDouble();
    FLAG0=ds.getLongLong();
    FLAG1=ds.getLongLong();
    SPARE0=ds.getDouble();
    SPARE1=ds.getDouble();
    SPARE2=ds.getDouble();
    SPARE3=ds.getDouble();
   }
 };


class CalTrigWeights
 {
  public:
  long long CID;
  long long ID;
  double TRIGSCL;
  std::vector<float> *ET_WEIGHT_CENT;
  std::vector<float> *ET_WEIGHT_WALL;
  std::vector<float> *ET_WEIGHT_PLUG;
  std::vector<int> *ET_FACTOR_CENT;
  std::vector<int> *ET_FACTOR_WALL;
  std::vector<int> *ET_FACTOR_PLUG;
  
  CalTrigWeights(frontier::CDFDataSource& ds)
   {
    CID=ds.getLongLong();
    ID=ds.getLongLong();
    TRIGSCL=ds.getDouble();
    ET_WEIGHT_CENT=ds.getRawAsArrayFloat();
    ET_WEIGHT_WALL=ds.getRawAsArrayFloat();
    ET_WEIGHT_PLUG=ds.getRawAsArrayFloat();
    ET_FACTOR_CENT=ds.getRawAsArrayInt();
    ET_FACTOR_WALL=ds.getRawAsArrayInt();
    ET_FACTOR_PLUG=ds.getRawAsArrayInt();    
   }
   
  virtual ~CalTrigWeights()
   {
    delete ET_WEIGHT_CENT;
    delete ET_WEIGHT_WALL;
    delete ET_WEIGHT_PLUG;
    delete ET_FACTOR_CENT;
    delete ET_FACTOR_WALL;
    delete ET_FACTOR_PLUG;    
   }
 };
 

int main(int argc, char **argv)
 {
  std::string server_url="";
  std::string *proxy_url=NULL;
 
  try
   {
    frontier::init();    

    if(argc>1) server_url=argv[1];
    if(argc>2) proxy_url=new std::string(argv[2]);
    
    frontier::CDFDataSource ds(server_url,proxy_url);
    if(proxy_url){delete proxy_url; proxy_url=NULL;}
    //frontier::CDFDataSource ds;
    
    ds.setReload(1);

    frontier::Request req1("svxbeamposition","1",frontier::BLOB,"cid","316011");
    frontier::Request req2("caltrigweights","1",frontier::BLOB,"cid","14319");

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req1);
    vrq.insert(vrq.end(),&req2);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);

    int nrec=ds.getRecNum();
    std::vector<SvxBeamPosition*> v_sbp(nrec);
    for(int i=0;i<nrec;i++)
     {
      v_sbp[i]=new SvxBeamPosition(ds);
      std::cout <<v_sbp[i]->CID<<','<<v_sbp[i]->CHANNELID<<'\n';
     }
    
    // Do some usefull things here ...

    // Clean
    for(int i=0;i<nrec;i++) delete v_sbp[i]; 
    
    ds.setCurrentLoad(2);

    nrec=ds.getRecNum();
    std::vector<CalTrigWeights*> v_ctw(nrec);
    for(int i=0;i<nrec;i++)
     {
      v_ctw[i]=new CalTrigWeights(ds);
      std::cout <<v_ctw[i]->CID<<','<<v_ctw[i]->ID<<std::endl;
     }        
    // Do some usefull things here ...
    
    for(int i=0;i<5;i++)
     {
      std::cout<<'\t'<<v_ctw[0]->ET_WEIGHT_CENT->operator[](i)<<'\n';
     }
    
    // Clean
    for(int i=0;i<nrec;i++) delete v_ctw[i];    
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


