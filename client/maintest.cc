/*
 * frontier client test C++ main program
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

#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <sys/time.h>


long long print_time(const char *msg)
 {
  struct timeval tv;
  long long res;
  
  int ret=gettimeofday(&tv,NULL);
  if(ret) printf("Can not get current time :-(\n");
  
  res=((long long)(tv.tv_sec))*1000+((long long)(tv.tv_usec))/1000;
  printf("%s %lld\n",msg,res);
  
  return res;
 } 


static void usage(char **argv)
 {
  printf("Usage: %s [-r] [-n count] [-s server] -o object:v[:m] key value [{key value}] [{-o object:v[:m] key value [{key value}] }]\n",argv[0]);
  printf("\t -r        - refresh cache on each request\n");
  printf("\t -n count  - repeat 'count' times\n");
  printf("\t -s server - use this 'server' instead one from environment variables\n");
  printf("\nFrontier Client test program, see http://lynx.fnal.gov/ntier-wiki/ for details\n");
  exit(1);
 }
 
 
int do_main(frontier::DataSource *ds,int obj1_ind,int argc, char **argv)
 {
  long long t1,t2;   
  std::vector<const frontier::Request*> vrq;
  char **arg_p=argv+(obj1_ind+1); // Skipping first "-o"
  
  while(*arg_p)
   {
    for(int i=0;i<3;i++)
     {
      char *p=arg_p[i];
      if(!p) usage(argv);
      if(strncmp(p,"-o",2)==0) usage(argv);
     }
    frontier::Request* req=new frontier::Request(arg_p[0],frontier::BLOB,arg_p[1],arg_p[2]);
    arg_p+=3;
    while(*arg_p && strncmp(*arg_p,"-o",2))
     {
      if(!(*(arg_p+1)) || strncmp(*(arg_p+1),"-o",2)==0) usage(argv);
      req->addKey(*arg_p,*(arg_p+1));
      arg_p+=2;
     }
    if(*arg_p && strncmp(*arg_p,"-o",2)==0) ++arg_p;
    else if(*arg_p && strncmp(*arg_p,"-o",2)!=0) usage(argv);
    vrq.insert(vrq.end(),req);
   }
  t1=print_time("start:  ");
  ds->getData(vrq);
  t2=print_time("finish: ");
    
  for(unsigned int i=1;i<=vrq.size();i++)	// XXX
   {
    ds->setCurrentLoad(i);
    int nrec=ds->getRecNum();
    printf("Payload %d records %d bsize %d\n",i,nrec,ds->getRSBinarySize());
    delete vrq[i-1];
   }
  printf("Duration %lld ms\n",t2-t1);
    
  return 1;
 }

 
int main(int argc, char **argv)
 {
  int arg_ind;
  int refresh=0;
  int repeat=1;
  char *server=0;
  int opt_num=0;
  
  for(arg_ind=1;arg_ind<argc && strcmp(argv[arg_ind],"-o");arg_ind++)
   {
    if(strcmp(argv[arg_ind],"-r")==0)
     {
      if(opt_num) usage(argv);
      opt_num=1;
      refresh=1;
      continue;
     }
    if(strncmp(argv[arg_ind],"-n",2)==0)
     {
      if(opt_num>1) usage(argv);
      if(arg_ind+1>=argc || *(argv[arg_ind+1])=='-') usage(argv);
      opt_num=2;
      repeat=atoi(argv[arg_ind+1]);
      ++arg_ind;
      continue;
     }
    if(strncmp(argv[arg_ind],"-s",2)==0)
     {
      if(opt_num>2) usage(argv);
      if(arg_ind+1>=argc || *(argv[arg_ind+1])=='-') usage(argv);
      opt_num=3;
      server=argv[arg_ind+1];
      arg_ind+=2; // Last option before objects
      break;
     }    
   }
  
  if(arg_ind>=argc || strncmp(argv[arg_ind],"-o",2)) usage(argv);
   
#ifdef FNTR_USE_EXCEPTIONS      
  try
   {
#endif //FNTR_USE_EXCEPTIONS   
    frontier::init();
    frontier::DataSource *ds;
    
    if(server)
     {
      ds=new frontier::DataSource(server);
     }
    else
     {
      ds=new frontier::DataSource();
     }
    if(refresh)
     {
      printf("Refresh the cache.\n");
      ds->setReload(1);
     }
    
    for(int i=1;i<=repeat;i++)
     {
      printf("Cycle %d of %d... ",i,repeat);
      do_main(ds,arg_ind,argc,argv);
     }
    delete ds;
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
   
 }
 
