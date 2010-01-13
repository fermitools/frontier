#!/usr/bin/env python

#
# Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
# All rights reserved. 
#
# For details of the Fermitools (BSD) license see Fermilab-2009.txt or
#  http://fermitools.fnal.gov/about/terms.html
#

# Simple python frontier client that can do multiple queries per connection.
# encodes input sql query (standard encoding has to be slightly modified 
# for url safety), retrieves data, and decodes results
# Author: Sinisa Veseli November 2005
# Update: Lee Lueking added --stats-only flag for perf testing
# Update: Dave Dykstra modified to use HTTP/1.1 and multiple queries
# Update: Dave Dykstra modified to also support https
#
#
import sys
import httplib
from xml.dom.minidom import parseString
import base64
import zlib 
import string
import curses.ascii
import time
import os

frontierId = "fnget2.py 1.2"

if sys.version < '2.4':
  print 'fnget2: python version must be at least 2.4!'
  sys.exit(1)

def usage():
  progName = os.path.basename(sys.argv[0])
  print "Usage:"
  print "  %s --server=<frontier http[s] server> --path=<path on frontier server> \\" % progName
  print "    [--no-decode] [--refresh-cache]  [--retrieve-ziplevel=N] [--stats-only] \\"
  print "    --sql=<sql query> [--sql=<sql query> ...] "
  print "Example:"
  print "  %s --server=cmsfrontier.cern.ch:8000 --path=/FrontierInt/Frontier \\" % progName
  print "     --sql='select 1 from dual' --sql='select 2 from dual'"
  print "Example using a proxy:"
  print "  %s --server=cmsfrontier1.fnal.gov:3128 \\" % progName
  print "     --path=http://cmsfrontier.cern.ch:8000/FrontierInt/Frontier \\"
  print "     --sql='select 1 from dual' --sql='select 2 from dual'"
  print "Example using https:"
  print "  %s --server=https://localhost:8443 --path=/FrontierInt/Frontier \\" % progName
  print "     --sql='select 1 from dual' --sql='select 2 from dual'"
  print " "

# this was copied from dbsHttpService.py
def getKeyCert():
  """
  Gets the User key and cert if they exist otherwise throws an exception
  """

  # First presendence to HOST Certificate, RARE
  if os.environ.has_key('X509_HOST_CERT'):
    cert = os.environ['X509_HOST_CERT']
    key = os.environ['X509_HOST_KEY']
  
  # Second preference to User Proxy, very common
  elif os.environ.has_key('X509_USER_PROXY'):
    cert = os.environ['X509_USER_PROXY']
    key = cert

  # Third preference to User Cert/key combination
  elif os.environ.has_key('X509_USER_CERT'):
    cert = os.environ['X509_USER_CERT']
    key = os.environ['X509_USER_KEY']

  # Worst case, look for proxy at default location /tmp/x509up_u$uid
  else :
    uid = os.getuid()
    cert = '/tmp/x509up_u'+str(uid)
    key = cert

  #Set but not found
  if not os.path.exists(cert) or not os.path.exists(key):
    raise Exception("required cert/key not found at %s or %s for user '%s'" % (cert, key, os.getlogin()))

  # All looks OK, still doesn't guarantee validity etc.
  return key, cert
  

frontierServer = None
frontierPath = None
frontierQueries = []
decodeFlag = True
refreshFlag = False
statsFlag = False
retrieveZiplevel = "zip"
for a in sys.argv[1:]:
  arg = string.split(a, "=")
  if arg[0] == "--server":
    frontierServer = arg[1]
  elif arg[0] == "--path":
    frontierPath = arg[1]
  elif arg[0] == "--sql":
    frontierQueries.append(string.join(arg[1:], "="))
  elif arg[0] == "--no-decode":
    decodeFlag = False
  elif arg[0] == "--refresh-cache":
    refreshFlag = True
  elif arg[0] == "--retrieve-ziplevel":
    level = string.join(arg[1:], "=")
    retrieveZiplevel="zip%s" % (level)
    if level == "0":
      retrieveZiplevel = ""
  elif arg[0] == "--stats-only":
    statsFlag = True
  else:
    print "Ignoring unrecognized argument: %s" % a

if frontierServer is None or frontierPath is None or frontierQueries is []:
  usage()
  sys.exit(1)    
  
if statsFlag:
  pass
else:
  print "Using Frontier http server: ", frontierServer
  print "Using Frontier server path: ", frontierPath
  print "Decode results: ", decodeFlag
  print "Refresh cache: ", refreshFlag

#open the http or https server connection
if frontierServer[:8] == "https://":
  key, cert = getKeyCert()
  connection = httplib.HTTPSConnection(frontierServer[8:], None, key, cert)
elif frontierServer[:7] == "http://":
  connection = httplib.HTTPConnection(frontierServer[7:])
else:
  connection = httplib.HTTPConnection(frontierServer)

headers = {"X-Frontier-Id": frontierId}
# add refresh header if needed
if refreshFlag:
  headers["Pragma"] = "no-cache"


for frontierQuery in frontierQueries:
  print "\n-----\nQuery: ", frontierQuery
  # encode query
  encQuery = base64.binascii.b2a_base64(zlib.compress(frontierQuery,9)).replace("+", ".").replace("\n","").replace("/","-").replace("=","_")

  # frontier request
  frontierRequest="%s?type=frontier_request:1:DEFAULT&encoding=BLOB%s&p1=%s" % (frontierPath, retrieveZiplevel, encQuery)
  if statsFlag:
    pass
  else:
    print "\nFrontier Request:\n%s" % frontierRequest

  # start and time query
  queryStart = time.localtime()
  if statsFlag:
    pass
  else:
    print "\nQuery started: ", time.strftime("%m/%d/%y %H:%M:%S %Z", queryStart)

  t1 = time.time()
  connection.request("GET", frontierRequest, None, headers)
  response = connection.getresponse()

  result = response.read()
  t2 = time.time()
  queryEnd = time.localtime()
  if statsFlag:
    duration=(t2-t1)
    size=len(result)
    print duration,size,size/duration
  else:
    print "Query ended: ", time.strftime("%m/%d/%y %H:%M:%S %Z", queryEnd)
    print "Query time: %s [seconds]\n" % (t2-t1)

  print "Response:", response.status, response.reason
  for tuple in response.getheaders():
    print "  %s: %s" % (tuple[0], tuple[1])
  print

  if decodeFlag:
    print "Query result:\n", result
    dom = parseString(result)
    dataList = dom.getElementsByTagName("data")
    # Control characters represent records, but I won't bother with that now,
    # and will simply replace those by space.
    for data in dataList:
      if data.firstChild is not None:

	row = base64.decodestring(data.firstChild.data)
        if retrieveZiplevel != "":
          row = zlib.decompress(row)
        for c in [ '\x00', '\x01', '\x02', '\x03', '\x04', '\x05', '\x06', '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d', '\x1b', '\x17'  ]:
	  row = row.replace(c, ' ')

	print "\nFields: "
	endFirstRow = row.find('\x07')
	firstRow = row[:endFirstRow]
	for c in firstRow:
	  if curses.ascii.isctrl(c):
	    firstRow = firstRow.replace(c, '\n')
	print firstRow

	print "\nRecords:"
	pos = endFirstRow + 1
	while True:
	  newrow = row[pos:]
	  endRow = newrow.find('\x07')
	  if endRow < 0:
	    break
	  fixedRow = newrow[:endRow]
	  pos = pos + endRow + 1
	  fixedRow = fixedRow.replace('\n', '')
	  print fixedRow

connection.close()
