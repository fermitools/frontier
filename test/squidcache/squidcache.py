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
from time import time
from urllib import urlopen
from socket import gethostname, gethostbyname
from pickle import *
from commands import *

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
    squid_objects_memory=100
    squid_objects_disk=10000
    Squid_cache_size_disk=10000
    return (squid_objects_memory,squid_objects_disk,Squid_cache_size_disk)

def run(outfile, host, port, table_file, numcalls, wordy):
    pid = os.getpid()
    ipaddr = gethostbyname(gethostname())
    # read in the pickled cids
    #
    #
    u=Unpickler(table_file)
    cids=u.load()
    tablelist=cids.keys()
    cid_key=0
    cid_key_exists=1
    #
    # loop over cid key elements
    #
    while cid_key_exists:
        # loop over all tables
        ##following 1 lines are for testing
        #tablelist=["CALL1Peds3"]
        for table in tablelist:
            cid_list_of_lists=cids[table]
            cid_list=cid_list_of_lists[cid_key]
            cid=cid_list[0]
            print "table,cid:",table,cid
            #
            # call the client program
            #
            run_maintest="../client/maintest %s %s %s cid %s"%(host, port, table, cid)
            print "run_maintest:",run_maintest
            maintest_info=getstatusoutput(run_maintest)
            print "maintest status, output:",maintest_info
            #
            # Get data from squid through the web server
            #
            squid_info=get_squid_info()
            #
            # Write info to output file
            #
            cid_key_exists=0
#>>> x[x.keys()[1]]
#[[33259L], [71345L]]
#>>> x[x.keys()[1]][1]
#[71345L]
#>>> x[x.keys()[1]][1][0]
#71345L
#>>> y=x[x.keys()[1]][1][0]
#>>> print y
#71345
#    -outfile.write("expid\tpid\taddr\ttransid\ttype\tsize\tstart\tstop\tdur\n")
#    open
#    for transid in range(0, numcalls):
#        doTransaction(outfile, host, port, table, index, expid, transid, pid, ipaddr, wordy)

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

    print "run level:(outfile, host, port, table_file, numcalls, wordy)",\
           outfile, host, port, table_file, numcalls, wordy
    run(outfile, host, port, table_file, numcalls, wordy)
