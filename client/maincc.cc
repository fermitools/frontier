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

  SvxBeamPosition(frontier::DataSource& ds)
   {
    CID=ds.getLong();
    CHANNELID=ds.getLong();
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
    FLAG0=ds.getLong();
    FLAG1=ds.getLong();
    SPARE0=ds.getDouble();
    SPARE1=ds.getDouble();
    SPARE2=ds.getDouble();
    SPARE3=ds.getDouble();
   }
 };


int main(int argc, char **argv)
 {
  try
   {
    frontier::init();

    frontier::DataSource ds("lynx.fnal.gov",8080,"/Frontier/","");

    frontier::Request req1("svxbeamposition","1",frontier::BLOB,"cid","316011");

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req1);
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


