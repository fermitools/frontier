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
  printf("Usage: %s [-r] [-n repeat] server_url -o object key value [{key value}] [{-o object key value [{key value}] }]\n",argv[0]);
  exit(1);
 }
 
 
int do_main(frontier::DataSource &ds,int obj1_ind,int argc, char **argv)
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
    frontier::Request* req=new frontier::Request(arg_p[0],"1",frontier::BLOB,arg_p[1],arg_p[2]);
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
  ds.getData(vrq);
  t2=print_time("finish: ");
    
  for(unsigned int i=1;i<=vrq.size();i++)	// XXX
   {
    ds.setCurrentLoad(i);
    int nrec=ds.getRecNum();
    printf("Payload %d records %d bsize %d\n",i,nrec,ds.getRSBinarySize());
    delete vrq[i-1];
   }
  printf("Duration %lld ms\n",t2-t1);
    
  return 1;
 }

 
int main(int argc, char **argv)
 {
  int i;
  int refresh=0;
  int repeat=1;
  char *server=0;
  int obj1_ind=0;
  
  for(i=1;i<argc;i++)
   {
    if(strcmp(argv[i],"-r")==0)
     {
      if(server) usage(argv);
      refresh=1;
      continue;
     }
    if(strncmp(argv[i],"-n",2)==0)
     {
      if(server) usage(argv);
      if(i+1>=argc) usage(argv);
      repeat=atoi(argv[i+1]);
      ++i;
      continue;
     }
    if(server) 
     {
      obj1_ind=i;
      break;
     }
    server=argv[i];
   }
  if(!server || !obj1_ind) usage(argv);
  if(strncmp(argv[obj1_ind],"-o",2)) usage(argv);
   
#ifdef FNTR_USE_EXCEPTIONS      
  try
   {
#endif //FNTR_USE_EXCEPTIONS   
    frontier::init();
    frontier::CDFDataSource ds(server);
    if(refresh)
     {
      printf("Refresh the cache.\n");
      ds.setReload(1);
     }
    
    for(i=1;i<=repeat;i++)
     {
      printf("Cycle %d of %d... ",i,repeat);
      do_main(ds,obj1_ind,argc,argv);
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
   
 }
 
