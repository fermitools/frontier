#!/usr/bin/env python

#
# Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
# All rights reserved. 
#
# For details of the Fermitools (BSD) license see Fermilab-2009.txt or
#  http://fermitools.fnal.gov/about/terms.html
#

# Simple python frontier client.
# encodes input sql query (standard encoding has to be slightly modified 
# for url safety), retrieves data, and decodes results
# Author: Sinisa Veseli November 2005
# Update: Lee Lueking added --stats-only flag for perf testing
# Update: Dave Dykstra added version number into frontier id and other mods
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

frontierId = "fnget.py 1.8"

def usage():
  progName = os.path.basename(sys.argv[0])
  print "Usage:"
  print "  %s --url=<frontier url> --sql=<sql query> [--no-decode]" % progName
  print "     [--refresh-cache] [--retrieve-ziplevel=N] [--sign]"
  print " "

frontierUrl = None
frontierQuery = None
decodeFlag = True
refreshFlag = False
statsFlag = False
retrieveZiplevel = "zip"
signParam=""
for a in sys.argv[1:]:
  arg = string.split(a, "=")
  if arg[0] == "--url":
    frontierUrl = arg[1]
  elif arg[0] == "--sql":
    frontierQuery = string.join(arg[1:], "=")
  elif arg[0] == "--no-decode":
    decodeFlag = False
  elif arg[0] == "--refresh-cache":
    refreshFlag = True
  elif arg[0] == "--retrieve-ziplevel":
    level = string.join(arg[1:], "=")
    retrieveZiplevel="zip%s" % (level)
    if level == "0":
      retrieveZiplevel = ""
  elif arg[0] == "--sign":
    signParam="&sec=sig"
  elif arg[0] == "--stats-only":
    statsFlag = True
  else:
    print "Ignoring unrecognized argument: %s" % a

if frontierUrl is None or frontierQuery is None:
  usage()
  sys.exit(1)    
  
if statsFlag:
  pass
else:
  print "Using Frontier URL: ", frontierUrl
  print "Query: ", frontierQuery
  print "Decode results: ", decodeFlag
  print "Refresh cache: ", refreshFlag

# encode query
encQuery = base64.binascii.b2a_base64(zlib.compress(frontierQuery,9)).replace("+", ".").replace("\n","").replace("/","-").replace("=","_")

# frontier request
frontierRequest="%s/type=frontier_request:1:DEFAULT&encoding=BLOB%s&p1=%s%s" % (frontierUrl, retrieveZiplevel, encQuery, signParam)
if statsFlag:
  pass
else:
  print "\nFrontier Request:\n", frontierRequest 

# add refresh header if needed
request = urllib2.Request(frontierRequest)
if refreshFlag:
  request.add_header("pragma", "no-cache")

request.add_header("X-Frontier-Id", frontierId)

# start and time query
queryStart = time.localtime()
if statsFlag:
  pass
else:
  print "\nQuery started: ", time.strftime("%m/%d/%y %H:%M:%S %Z", queryStart)

def _under_24():
  import sys
  if sys.version_info[0] < 2: return True
  if sys.version_info[0] == 2:
    return sys.version_info[1] < 4
  return False

#---------------- define timeout on urllib2 socket ops -------------#
#  Adapted from http://code.google.com/p/timeout-urllib2/

from httplib import HTTPConnection as _HC
import socket
from urllib2 import HTTPHandler as _H

def sethttptimeout(timeout):
  """Use TimeoutHTTPHandler and set the timeout value.
  
  Args:
    timeout: the socket connection timeout value.
  """
  if _under_26():
    opener = urllib2.build_opener(TimeoutHTTPHandler(timeout))
    urllib2.install_opener(opener)
  else:
    raise Error("This python version has timeout builtin")

def _clear(sock):
  sock.close()
  return None

def _under_26():
  import sys
  if sys.version_info[0] < 2: return True
  if sys.version_info[0] == 2:
    return sys.version_info[1] < 6
  return False

class Error(Exception): pass

class HTTPConnectionTimeoutError(Error): pass

class TimeoutHTTPConnection(_HC):
  """A timeout control enabled HTTPConnection.
  
  Inherit httplib.HTTPConnection class and provide the socket timeout
  control.
  """
  _timeout = None

  def __init__(self, host, port=None, strict=None, timeout=None):
    """Initialize the object.

    Args:
      host: the connection host.
      port: optional port.
      strict: strict connection.
      timeout: socket connection timeout value.
    """
    _HC.__init__(self, host, port, strict)
    self._timeout = timeout or TimeoutHTTPConnection._timeout
    if self._timeout: self._timeout = float(self._timeout)

  def connect(self):
    """Perform the socket level connection.

    A new socket object will get built everytime. If the connection
    object has _timeout attribute, it will be set as the socket
    timeout value.

    Raises:
      HTTPConnectionTimeoutError: when timeout is hit
      socket.error: when other general socket errors encountered.
    """
    msg = "getaddrinfo returns an empty list"
    err = socket.error
    for res in socket.getaddrinfo(self.host, self.port, 0,
                                  socket.SOCK_STREAM):
      af, socktype, proto, canonname, sa = res
      try:
        try:
          self.sock = socket.socket(af, socktype, proto)
          if self._timeout: self.sock.settimeout(self._timeout)
          if self.debuglevel > 0:
            print "connect: (%s, %s)" % (self.host, self.port)
          self.sock.connect(sa)
        except socket.timeout, msg:
          err = socket.timeout
          if self.debuglevel > 0:
            print 'connect timeout:', (self.host, self.port)
          self.sock = _clear(self.sock)
          continue
        break
      except socket.error, msg:
        if self.debuglevel > 0:
          print 'general connect fail:', (self.host, self.port)
        self.sock = _clear(self.sock)
        continue
      break
    if not self.sock:
      if err == socket.timeout:
        raise HTTPConnectionTimeoutError, msg
      raise err, msg

class TimeoutHTTPHandler(_H):
  """A timeout enabled HTTPHandler for urllib2."""
  def __init__(self, timeout=None, debuglevel=0):
    """Initialize the object.

    Args:
      timeout: the socket connect timeout value.
      debuglevel: the debuglevel level.
    """
    _H.__init__(self, debuglevel)
    TimeoutHTTPConnection._timeout = timeout

  def http_open(self, req):
    """Use TimeoutHTTPConnection to perform the http_open"""
    return self.do_open(TimeoutHTTPConnection, req)

#---------------- end timeout on socket ops ----------------#

t1 = time.time()
if _under_24():
  print >> sys.stderr, "*WARNING:* no timeout available in python older than 2.4"
  result = urllib2.urlopen(request).read()
elif _under_26():
  sethttptimeout(10)
  result = urllib2.urlopen(request).read()
else:
  result = urllib2.urlopen(request,None,10).read()
t2 = time.time()
queryEnd = time.localtime()
if statsFlag:
  duration=(t2-t1)
  size=len(result)
  print duration,size,size/duration
else:
  print "Query ended: ", time.strftime("%m/%d/%y %H:%M:%S %Z", queryEnd)
  print "Query time: %s [seconds]\n" % (t2-t1)

if decodeFlag:
  print "Query result:\n", result
  dom = parseString(result)
  dataList = dom.getElementsByTagName("data")
  keepalives = 0
  # Control characters represent records, but I won't bother with that now,
  # and will simply replace those by space.
  for data in dataList:
    for node in data.childNodes:
      # <keepalive /> elements may be present, combined with whitespace text
      if node.nodeName == "keepalive":
	# this is of type Element
	keepalives += 1
        continue
      # else assume of type Text
      if node.data.strip() == "":
        continue
      if keepalives > 0:
	print keepalives, "keepalives received\n"
	keepalives = 0

      row = base64.decodestring(node.data)
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
