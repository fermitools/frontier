#! /usr/bin/env python
#
# This script is used to run a test which loads the squid cache and
# tests the performance 
# as a function of the number of objects in the cache.

"""squidcache - a test script using the ProtoSix servlet and client.

SYNOPSIS
  squidcache [options] host port table_list_pickle_file numcalls

  
  DESCRIPTION

  This program does teh following:
   1. read in table and cid info 
   2. every Nth time, request an old object 
   3. call maintest.cc < url port table cid > time, duration, error 
   4. geturl information from the squid cgi location 
   5. collect all information and write to log 
   6. go to 2. 


  -h, --help
    Print this help and exit.

  -v, --version
    Print the version information and exit.

  -o, --output=OUTFILE
    Send output to the file OUTFILE. By default, output is written to
    the standard output.

  -w, --wordy
    Outputs the data recieved back from the server.

EXAMPLES:

    $ ./squidcache.py cdfdbfrontier.fnal.gov 8000 cdf_cid_table_list_1.pkl 1

    $Id$
"""

__version__ = "$Revision:"

import os, sys, getopt
from time import time,sleep
from urllib import urlopen
from socket import gethostname, gethostbyname
from pickle import *
from commands import *
from string import *

def usage():
    print __doc__

def processCommandLine(opts, args):
    outfile = sys.stdout
    wordy = 0
    for o, a in opts:
        if o in ("-h", "--help"):
            usage()
            sys.exit()
        if o in ("-v", "--version"):
            print "client5", __version__
            sys.exit()
        if o in ("-o", "--output"):
            outfile = file(a, "w")
        if o in ("-w", "--wordy"):
            wordy = 1

    if not args or len(args) < 4:
        print "client5: not enough arguments"
        usage()
        sys.exit(1)

    host     = args[0]
    port     = args[1]
    table_file = file(args[2],"r")
    numcalls = int(args[3])
    return (outfile, host, port, table_file, numcalls, wordy)


#def makeURL(host, port, table, idx, expID, tid, pid):
#    return "http://%s:%d/ProtoFive/ProtoFive?fcn=1&table=%s&id=%d&exp=%d&trans=%d&procid=%d" % \
#           (host, port, table, idx, expID, tid, pid)

def timeInMillis():
    return long(1000*time())

def doTransaction(outfile, host, port, table, index, expid, transid, pid, ipaddr
, wordy):
    url = makeURL(host, port, table, index, expid, transid, pid)
    start = timeInMillis()
    #print "\n%s\n" % url
    f = urlopen(url)
    data = f.read()
    f.close()
    stop = timeInMillis()
    size = len(data)
    duration = stop - start
    record = '%d\t%d\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n' % \
             ( expid, pid, ipaddr, transid, \
               index, size, start, stop, duration )
    outfile.write(record)
    if wordy:
        outfile.write("\n" + data)
def get_squid_info():
    print "get_squid_info called"
    #url from where to get status information (needs to be updated by hand before running script)
    url="http://lynx.fnal.gov/cgi-bin/cachemgr.cgi?host=localhost&port=3128&operation=info"
    #get status information from cachemgr.cgi
    #sleep of 1 second added because squid is too busy and returns empty.
    #Tried 0.5 seconds but it failed
    sleep(1) # have to sleep 
    f = urlopen(url)
    info = f.read()
    f.close()

    # order of data is: ('Swap(kB) Mem(kB) 5minCPU(%) 60minCPU(%) #Entries on-disk')
    #parse needed infomation from cachemgr.cgi output buffer
    #needed lines are:
    # Storage Swap size:
    # Storage Mem size:
    # CPU Usage, 5 minute avg:
    # CPU Usage, 60 minute avg:
    # ## StoreEntries
    # ## on-disk objects

    line=info
    #print line
    tmpvar = find(line, "Storage Swap size")
    print "tempvar",tmpvar
    print line[tmpvar+19:find(line, " KB", tmpvar)]
    storage_swap_size=atol( line[tmpvar+19:find(line, " KB", tmpvar)])

    tmpvar = find(line, "Storage Mem size:")
    storage_memory_size = atol( line[tmpvar+18:find(line, " KB", tmpvar)])

    tmpvar = find(line, "CPU Usage, 5 minute avg:")
    cpu_5min_agerage = atof(line[tmpvar+25:find(line, "%", tmpvar)])

    tmpvar = find(line, "CPU Usage, 60 minute avg:")
    cpu_60min_agerage =  atof(line[tmpvar+26:find(line, "%", tmpvar)])

    tmpvar = find(line, "StoreEntries")
    store_entries =atol( line[rfind(line," ",tmpvar-10,tmpvar-2)+1:tmpvar-1])

    tmpvar = find(line, "on-disk objects")
    on_disk_objects = atol(line[rfind(line," ",tmpvar-10,tmpvar-2)+1:tmpvar-1])
    #print "storage_swap_size,storage_memory_size,cpu_5min_agerage,cpu_60min_agerage,store_entries,on_disk_objects"
    #print storage_swap_size,storage_memory_size,cpu_5min_agerage,cpu_60min_agerage,store_entries,on_disk_objects
    return (storage_swap_size,storage_memory_size,cpu_5min_agerage,cpu_60min_agerage,store_entries,on_disk_objects)

def run(outfile, host, port, table_file, numcalls, wordy):
    pid = os.getpid()
    ipaddr = gethostbyname(gethostname())
    # read in the pickled cids
    #
    #
    u=Unpickler(table_file)
    cids=u.load()
    tablelist=cids.keys()
    cid_key_count=0
    num_calls=0
    cid_key_exists=1
    control=0
    #
    # loop over cid key elements
    #
    while cid_key_exists:
        # loop over all tables
        cid_key_exists=0
        
        
        #
        # Every 100th cid_key, fetch the first one as a control
        #
        if ((cid_key_count % 100)== 0)& (control==0)& (cid_key_count !=0):
            control=1
            cid_key=0
            cid_status="old"
        else:
            control=0
            cid_status="new"
            cid_key=cid_key_count
            
            
        for table in tablelist:
            cid_list_of_lists=cids[table]
            if (len(cid_list_of_lists)>cid_key) & ((num_calls<numcalls) | (numcalls==-1)):
                cid_key_exists=1
                num_calls=num_calls+1
                cid_list=cid_list_of_lists[cid_key]
                cid=cid_list[0]
                print "table,cid:",cid_key,table,cid,cid_status
                #
                # call the client program
                #
                run_maintest="../../client/maintest %s %s %s cid %s"%(host, port, table, cid)
                print "run_maintest:",run_maintest
                maintest_info=getstatusoutput(run_maintest)
                print "maintest status, output:",maintest_info
                if maintest_info[0]>0:
                    print "ERROR from maintest"
                else:                    #
                    info=maintest_info[1]
                    start_time=atol(info[8:find(info,"\nfinish")])
                    end_time=atol(info[find(info,"finish:")+8:find(info,"\nURL")])
                    duration=end_time-start_time
                    num_records=info[find(info,"records")+7:]
                    # Get data from squid through the web server
                    #
                    squid_info=get_squid_info()
                    #
                    # Write info to output file:
                    # Begin_time duration obj_size table_name cid cid_status squid_size squid_entries
                    #
                    word="%d\t%d\t%s\t%s\t%d\t%s\t%d\t%d\t%4.1f\t%4.1f\t%d\t%d\n"%(start_time,duration,num_records,table,cid,cid_status,squid_info[0],squid_info[1],squid_info[2],squid_info[3],squid_info[4],squid_info[5])
                    #print word
                    outfile.write(word)
        if control == 0:
            cid_key_count=cid_key_count+1
        
                        

if __name__ == "__main__":
    try:
        opts, args = getopt.getopt(sys.argv[1:],
                                   "hvwo:",
                                   ["help",
                                    "version",
                                    "wordy",
                                    "output="])
        (outfile, host, port, table_file, numcalls, wordy)=\
                  processCommandLine(opts, args)

    except getopt.GetoptError, e:
        print "squidcache:", e
        usage()
        sys.exit(1)

    #print "run level:(outfile, host, port, table_file, numcalls, wordy)",\
    #       outfile, host, port, table_file, numcalls, wordy
    outfile.write("start_time\tdur\trecords\ttable\tcid\tstatus\tswap\tmemory\tcpu_5min\tcpu_60min\tentries\tdisk_objects\n")
    run(outfile, host, port, table_file, numcalls, wordy)
    outfile.close()
