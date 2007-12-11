/*
 * frontier client test main program to get cids
 * 
 * Author: Sergey Kosyakov
 *
 * $Id$
 *
 *  Copyright (C) 2007  Fermilab
 *
 *  This program is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License
 *  as published by the Free Software Foundation, either version 3 of
 *  the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this program.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */
#include <frontier-cpp.h>

#include <iostream>
#include <stdexcept>
 

int main(int argc, char **argv)
 {
  if(argc!=3)
   {
    std::cout<<"Usage: "<<argv[0]<<" server_url object_name"<<'\n';
    exit(1);
   }
      
  try
   {
    frontier::init();

    frontier::CDFDataSource ds(argv[1]);
    
    //ds.setReload(1);

    frontier::Request req("frontier_get_cid_list","1",frontier::BLOB,"table_name",argv[2]);

    std::vector<const frontier::Request*> vrq;
    vrq.insert(vrq.end(),&req);
    ds.getData(vrq); 

    ds.setCurrentLoad(1);
    int nrec=ds.getRecNum();
    std::cout<<"Got "<<nrec<<" records back."<<'\n';
    
    for(int i=0;i<nrec;i++)
     {
      long cid=ds.getLongLong();
      std::cout<<cid<<'\n';
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


