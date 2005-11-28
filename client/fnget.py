#!/usr/bin/env python

# Simple python frontier client.
# encodes input sql query (standard encoding has to be slightly modified 
# for url safety), retrieves data, and decodes results
# 
#
# example of usage
# ./fnget.py --url=http://lxfs6043.cern.ch:8000/Frontier3D/Frontier 
#   --sql="select name,version from frontier_descriptors"
#
#
import sys
import urllib2
from xml.dom.minidom import parseString
import base64
import zlib 
import string
import curses.ascii
import time
import os.path

def usage():
  progName = os.path.basename(sys.argv[0])
  print "Usage:"
  print "  %s --url=<frontier url> --sql=<sql query> [--no-decode]" % progName
  print "     [--refresh-cache]"
  print " "

frontierUrl = None
frontierQuery = None
decodeFlag = True
refreshFlag = False
for a in sys.argv[1:]:
  arg = string.split(a, "=")
  if arg[0] == "--url":
    frontierUrl = arg[1]
  elif arg[0] == "--sql":
    frontierQuery = arg[1]
  elif arg[0] == "--no-decode":
    decodeFlag = False
  elif arg[0] == "--refresh-cache":
    refreshFlag = True
  else:
    print "Ignoring unrecognized argument: %s" % a

if frontierUrl is None or frontierQuery is None:
  usage()
  sys.exit(1)    
  

print "Using Frontier URL: ", frontierUrl
print "Query: ", frontierQuery
print "Decode results: ", decodeFlag
print "Refresh cache: ", refreshFlag

# encode query
encQuery = base64.binascii.b2a_base64(zlib.compress(frontierQuery,9)).replace("+", ".")

# frontier request
frontierRequest="%s?type=frontier_request:1:DEFAULT&encoding=BLOB&p1=%s" % (frontierUrl, encQuery)
print "\nFrontier Request:\n", frontierRequest 

# add refresh header if needed
request = urllib2.Request(frontierRequest)
if refreshFlag:
  request.add_header("pragma", "no-cache")

# start and time query
queryStart = time.localtime()
print "\nQuery started: ", time.strftime("%m/%d/%y %H:%M:%S", queryStart)
t1 = time.time()
result = urllib2.urlopen(request).read()
t2 = time.time()
queryEnd = time.localtime()
print "Query ended: ", time.strftime("%m/%d/%y %H:%M:%S", queryEnd)
print "Query time: %s [seconds]\n" % (t2-t1)

if decodeFlag:
  print "Query result:\n", result
  dom = parseString(result)
  dataList = dom.getElementsByTagName("data")
  # Control characters represent records, but I won't bother with that now,
  # and will simply replace those by space.
  for data in dataList:
    if data.firstChild is not None:

      row = base64.decodestring(data.firstChild.data)
      for c in [ '\x00', '\x01', '\x02', '\x03', '\x04', '\x06', '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d'  ]:
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
