from __future__ import print_function
'''Frontier Client thin binding.

Please do not use this module: this module is internal, intended to simplify
the calls to the frontier_client C library made by the frontier module.
This could be replaced with a compiled version for performance.
'''

__author__ = 'Miguel Ojeda'
__copyright__ = 'Copyright 2013, CERN'
__credits__ = ['Miguel Ojeda']
__license__ = 'Unknown'
__maintainer__ = 'Miguel Ojeda'
__email__ = 'mojedasa@cern.ch'


import ctypes
import logging


logger = logging.getLogger(__name__)

# Set initial level to WARN.  This so that log statements don't occur in
# the absense of explicit logging being enabled.
if logger.level == logging.NOTSET:
    logger.setLevel(logging.WARN)


libfc = ctypes.cdll.LoadLibrary('libfrontier_client.so.2')
libfc.frontierRSBlob_open.argtypes = [ctypes.c_int64, ctypes.c_void_p]
libfc.frontierRSBlob_open.restype = ctypes.c_void_p # not a NUL-terminated string
libfc.frontierRSBlob_close.restype = None
libfc.frontierRSBlob_payload_error.argtypes = [ctypes.c_void_p]
libfc.frontierRSBlob_payload_msg.restype = ctypes.c_char_p
libfc.frontierRSBlob_getByte.argtypes = [ctypes.c_void_p]
libfc.frontierRSBlob_getByte.restype = ctypes.c_byte # c_char
libfc.frontierRSBlob_getInt.argtypes = [ctypes.c_void_p]
libfc.frontierRSBlob_getInt.restype = ctypes.c_int32 # c_int
libfc.frontierRSBlob_getLong.restype = ctypes.c_int64 # c_longlong
libfc.frontierRSBlob_getDouble.restype = ctypes.c_double
libfc.frontierRSBlob_getFloat.restype = ctypes.c_float
libfc.frontierRSBlob_getByteArray.argtypes = [ctypes.c_void_p]
libfc.frontierRSBlob_getByteArray.restype = ctypes.c_void_p # not a NUL-terminated string
libfc.frontierRSBlob_getRecNum.argtypes = [ctypes.c_void_p]
libfc.frontierRSBlob_getRecNum.restype = ctypes.c_int64
libfc.frontier_createChannel.restype = ctypes.c_int64
libfc.frontier_closeChannel.argtypes = [ctypes.c_int64]
libfc.frontier_closeChannel.restype = None
libfc.frontier_getErrorMsg.restype = ctypes.c_char_p
libfc.frontier_getRawData.argtypes = [ctypes.c_int64]


FRONTIER_OK              =  0
FRONTIER_EIARG           = -1  # Invalid argument passed
FRONTIER_EMEM            = -2  # mem_alloc failed
FRONTIER_ECFG            = -3  # config error
FRONTIER_ESYS            = -4  # system error
FRONTIER_EUNKNOWN        = -5  # unknown error
FRONTIER_ENETWORK        = -6  # error while communicating over network
FRONTIER_EPROTO          = -7  # protocol level error (e.g. wrong response)
FRONTIER_ESERVER         = -8  # server error (may be cached for short time)
FRONTIER_ECONNECTTIMEOUT = -9  # connection timeout


# The C library should expose all this somehow, e.g. getTypeByte("INT8")
BLOB_BIT_NULL        = 1 << 7 # mask
BLOB_TYPE_BYTE       = 0
BLOB_TYPE_INT4       = 1
BLOB_TYPE_INT8       = 2
BLOB_TYPE_FLOAT      = 3
BLOB_TYPE_DOUBLE     = 4
BLOB_TYPE_TIME       = 5
BLOB_TYPE_ARRAY_BYTE = 6
BLOB_TYPE_EOR        = 7
BLOB_TYPE_NONE       = BLOB_BIT_NULL # same as the mask


class FrontierClientError(Exception):
    def __init__(self, retcode, message):
        self.args = (retcode, message)


def frontier_mem_free(address):
    '''frontier_free calls a custom allocator (and requires using frontier_malloc).
    
    This is what the C++ binding does instead.
    '''

    ctypes.CFUNCTYPE(None, ctypes.c_void_p)(ctypes.c_void_p.in_dll(libfc, 'frontier_mem_free').value)(address)


def frontier_init():
    logger.debug('frontier_client.frontier_init()')
    retcode = libfc.frontier_init(None, None)
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())


def frontier_createChannel(serverURL = None, proxyURL = None):
    logger.debug('frontier_client.frontier_createChannel(serverURL = %s, proxyURL = %s)', repr(serverURL), repr(proxyURL))
    retcode = ctypes.c_int(FRONTIER_OK)
    channel = libfc.frontier_createChannel(serverURL.encode(), proxyURL, ctypes.byref(retcode))
    retcode = retcode.value
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())
    logger.debug('frontier_client.frontier_createChannel(serverURL = %s, proxyURL = %s) = %s', repr(serverURL), repr(proxyURL), channel)
    return channel


def frontier_closeChannel(channel):
    logger.debug('frontier_client.frontier_closeChannel(channel = %s)', channel)
    libfc.frontier_closeChannel(channel)


def fn_gzip_str2urlenc(string, decode=False):
    logger.debug('frontier_client.fn_gzip_str2urlenc(string = %s)', repr(string))
    buf = ctypes.c_void_p()
    buflen = libfc.fn_gzip_str2urlenc(string.encode(), len(string), ctypes.byref(buf))
    if buflen < 0:
        raise FrontierClientError(None, 'Impossible to encode.')
    s = ctypes.string_at(buf.value, buflen)
    frontier_mem_free(buf.value)
    return s.decode() if decode else s


def frontier_getRawData(channel, uri):
    logger.debug('frontier_client.frontier_getRawData(channel = %s, uri = %s)', channel, repr(uri))
    retcode = libfc.frontier_getRawData(channel, uri.encode())
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())


def frontierRSBlob_open(channel, oldrsb, n):
    logger.debug('frontier_client.frontierRSBlob_open(channel = %s, oldrsb = %s, n = %s)', channel, oldrsb, n)
    retcode = ctypes.c_int(FRONTIER_OK)
    rsb = libfc.frontierRSBlob_open(channel, oldrsb, n, ctypes.byref(retcode))
    retcode = retcode.value
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())
    retcode = libfc.frontierRSBlob_payload_error(rsb)
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontierRSBlob_payload_msg(rsb))
    return rsb


def frontierRSBlob_close(rsb):
    logger.debug('frontier_client.frontierRSBlob_close(rsb = %s)', rsb)
    retcode = ctypes.c_int(FRONTIER_OK)
    libfc.frontierRSBlob_close(rsb, ctypes.byref(retcode))
    retcode = retcode.value
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())


def frontierRSBlob_getRecNum(rsb):
    logger.debug('frontier_client.frontierRSBlob_getRecNum(rsb = %s)', rsb)
    return libfc.frontierRSBlob_getRecNum(rsb)


def _build_frontierRSBlob_get(f):
    def wrapper(rsb):
        retcode = ctypes.c_int(FRONTIER_OK)
        result = f(rsb, ctypes.byref(retcode))
        retcode = retcode.value
        if retcode != FRONTIER_OK:
            raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())
        return result

    return wrapper


frontierRSBlob_getByte = _build_frontierRSBlob_get(libfc.frontierRSBlob_getByte)
frontierRSBlob_checkByte = _build_frontierRSBlob_get(libfc.frontierRSBlob_checkByte)
frontierRSBlob_getInt = _build_frontierRSBlob_get(libfc.frontierRSBlob_getInt)
frontierRSBlob_getLong = _build_frontierRSBlob_get(libfc.frontierRSBlob_getLong)
frontierRSBlob_getDouble = _build_frontierRSBlob_get(libfc.frontierRSBlob_getDouble)
frontierRSBlob_getFloat = _build_frontierRSBlob_get(libfc.frontierRSBlob_getFloat)


def frontierRSBlob_getByteArray(rsb, decode=False):
    buflen = frontierRSBlob_getInt(rsb)
    retcode = ctypes.c_int(FRONTIER_OK)
    buf = libfc.frontierRSBlob_getByteArray(rsb, buflen, ctypes.byref(retcode))
    retcode = retcode.value
    if retcode != FRONTIER_OK:
        raise FrontierClientError(retcode, libfc.frontier_getErrorMsg())
    s = ctypes.string_at(buf, buflen)
    return s.decode() if decode else s


frontier_init()

