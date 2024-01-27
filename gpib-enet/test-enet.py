#!/usr/bin/python
# 
# -*- coding: utf8 -*-
#
# test-enet -- 
#
# Copyright (C) 2005 Robert Jordens <jordens@debian.org>
#
# This may be used and distributed according to the terms
# of the GNU General Public License.
#
# $Id$
# arch-tag: acc2a56e-ea5d-4a47-ba55-ffacd359270b
#
# TODO: 
#
#  * The "ns" padding bytes in the 12-byte command can most probably be
#    replaced by "nx".
#  * ud switching
#  * all the _not_impl()s
#  * non-blocking IO

import socket, sys
from struct import *

debug_io_dummy = 1<<0
debug_io = 1<<1
debug_impl = 1<<2
#debug = debug_io_dummy | debug_io
debug = [debug_io, debug_impl]

def _dbg(f):
  def wrap(self, *a, **k):
    print "DBG: %s in: %s %s" % (f.__name__, `a`, `k`)
    r = f(self, *a, **k)
    print "DBG: %s out: %s" % (f.__name__, `r`)
    return r
  wrap.__dummy = True
  return wrap

def _not_impl(name):
  def wrap(self, *a, **k):
    if debug_impl in debug:
      return None
    else:
      raise NotImplementedError, "%s not implemented", name
  return _dbg(wrap)


class GpibEnet(object):
  def __init__(self, host, port=5000):
    self._host = host
    self._port = port
    self._sta = self._err = self._cnt = 0
    self._uds = {0: None}
    self._current_ud = None
    self._open()

  def _open(self):
    self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    self._socket.connect((self._host, self._port))
    
  def _send(self, string):
    return self._socket.send(string)
  
  def _recv(self, length):
    s = ""
    while len(s) < length:
      s += self._socket.recv(length-len(s))
    return s
    
  def _close(self):
    self._socket.close()

  if debug_io_dummy in debug:
    _open = lambda self: None
    _send = lambda self, s: sys.stderr.write("DBG: > %s" % `s`)
    _recv = lambda self, len: raw_input("DBG: < #%s:" % len)[:len]
    _close = lambda self: None
    
  if debug_io in debug:
    _open = _dbg(_open)
    _recv = _dbg(_recv)
    _send = _dbg(_send)
    _close = _dbg(_close)
   
  _headfmt = "!H H"
  def _read(self, header = None):
    if not header:
      header = self._recv(calcsize(self._headfmt))
    flag, num = unpack(self._headfmt, header)
    if flag != 0x0000:
      return None
    else:
      return self._recv(num)
    
  def _write(self, string):
    return self._send(string)
   
  def _scmd(self, id, argsfmt, *args):
    assert calcsize("!" + argsfmt) == 12 - 1
    self._write(pack("!B" + argsfmt, *((id,) + args)))
    return self._sresp()

  _respfmt = "!H H 4x L"
  def _sresp(self):
    ret = self._read()
    self._sta, self._err, self._cnt = unpack(self._respfmt, 
      ret[:calcsize(self._respfmt)])
    return ret[calcsize(self._respfmt):]

  def _check_ud(self, ud):
    if ud not in self._uds:
      raise ValueError, "%d invalid", ud
    if self._current_ud != ud:
      pass
      # TODO: select ud

  def ibdev(self, bna, pad, sad=0, tmo=13, eot=1, eos=0):
    if sad != 0: pad |= 0x80
    board_flags = 0
    self._scmd(0x07, "BBB BBBB B 3s", bna, board_flags, 
      0x40 | eot, pad, sad, eos, 0, tmo, "\x02\x04\x00")
    ud = max(self._uds.keys()) + 1
    self._uds[ud] = (bna, board_flags, pad, sad, eot, eos, tmo)
    self._current_ud = ud
    return ud

  def ibask(self, ud, cfg):
    self._check_ud(ud)
    self._scmd(0x4e, "B 10s", cfg,
      "\x00\x00\x10\x00\x00\x00\x40\x63\x16\x40")
    return self._sta, self._err
    
  def ibconfig(self, ud, cfg, val):
    self._check_ud(ud)
    self._scmd(0x4e, "B B 9s", cfg, val,
      "\x08\x00\x00\x00\x00\x00\x54\x00\x00")
    return self._sta, self._err

  def ibwait(self, ud, mask=0):
    self._check_ud(ud)
    self._scmd(0x22, "B H 8s", 0x54, mask,
      "\x20\xe1\x05\x08\xb4\xe0\x05\x08")
    return self._sta

  def ibrsp(self, ud):
    self._check_ud(ud)
    stb, = unpack("!B", self._scmd(0x19, "11s",
      "\x63\x16\x40\xc0\x58\x16\x40\x40\x63\x16\x40"))
    return self._sta, stb

  def ibonl(self, ud, val=0):
    self._check_ud(ud)
    self._scmd(0x12, "B 10s", val,
      "\x00\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")
    if not val:
      del self._uds[ud]
      self._current_ud = None
    return self._sta

  def ibclr(self, ud):
    self._check_ud(ud)
    self._scmd(0x04, "11s", 
      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")
    return self._sta

  def ibeos(self, ud, val):
    self._check_ud(ud)
    self._scmd(0x08, "H 9s", val,
      "\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")
    return self._sta

  def ibeot(self, ud, val):
    self._check_ud(ud)
    self._scmd(0x09, "B 10s", val,
      "\x00\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")
    return self._sta

  def iblines(self, ud):
    self._check_ud(ud)
    lines, = unpack("!H", self._scmd(0x0d, "11s", 
      "\x63\x16\x40\xc0\x58\x16\x40\x40\x63\x16\x40"))
    return self._sta, lines
    
  def ibln(self, pad, sad=0):
    if sad != 0: pad |= 0x80
    listen, = unpack("!H", self._scmd(0x0f, "B B 9s", pad, sad,
      "\x00\xf0\x38\x06\x08\x03\x00\x00\x00"))
    return self._sta, listen
    
  def ibloc(self, ud):
    self._check_ud(ud)
    self._scmd(0x10, "11s", 
      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")
    return self._sta
    
  def ibtmo(self, ud, tmo):
    self._check_ud(ud)
    self._scmd(0x1f, "B 10s", tmo,
      "\x00\x00\x20\xe1\x05\x08\xae\xe0\x05\x08")
    return self._sta
 
  def ibtrg(self, ud):
    self._check_ud(ud)
    self._scmd(0x20, "11s", 
      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")
    return self._sta
    
  def ibcac(self, ud, val=1):
    self._check_ud(ud)
    self._scmd(0x03, "B 10s", val,
      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")
    return self._sta
    
  def ibgts(self, ud, val=1):
    self._check_ud(ud)
    self._scmd(0x0a, "B 10s", val,
      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")
    return self._sta
    
  def ibrsc(self, ud, val=1):
    self._check_ud(ud)
    self._scmd(0x18, "B 10s", val,
      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")
    return self._sta

  def ibsic(self, ud):
    self._check_ud(ud)
    self._scmd(0x1c, "11s", 
      "\xe1\x05\x08\xb1\xe0\x05\x08\x88\xf5\xff\xbf")
    return self._sta
    
  def ibwrt(self, ud, string):
    self._check_ud(ud)
    self._scmd(0x23, "3s I 4s", "\x05\x05\x08", len(string), 
      "\x00\x54\x00\x00")
    self._write(string)
    self._sresp()
    return self._sta
    
  def ibrd(self, ud, num):
    self._check_ud(ud)
    head = self._scmd(0x16, "3s I 4s", "\x00\x00\x00", num,
      "\x40\x63\x16\x40")
    ret = ""
    while True:
      frag = self._read(head)
      if frag == None:
        break
      else:
        ret += frag
        head = None
    return self._sta, ret

  ibbna = _not_impl("ibbna")
  ibcmd = _not_impl("ibcmd")
  ibcmda = _not_impl("ibcmda")
  ibwrta = _not_impl("ibwrta")
  ibdiag = _not_impl("ibdiag")
  ibdma = _not_impl("ibdma")
  ibevent = _not_impl("ibevent")
  ibfind = _not_impl("ibfind")
  ibist = _not_impl("ibist")
  ibllo = _not_impl("ibllo")
  ibpad = _not_impl("ibpad")
  ibpct = _not_impl("ibpct")
  ibpoke = _not_impl("ibpoke")
  ibppc = _not_impl("ibppc")
  ibrda = _not_impl("ibrda")
  ibrdf = _not_impl("ibrdf")
  ibrdkey = _not_impl("ibrdkey")
  ibrpp = _not_impl("ibrpp")
  ibrsv = _not_impl("ibrsv")
  ibsad = _not_impl("ibsad")
  ibsgnl = _not_impl("ibsgnl")
  ibsre = _not_impl("ibsre")
  ibsrq = _not_impl("ibsrq")
  ibstop = _not_impl("ibstop")
  ibwrta = _not_impl("ibwrta")
  ibwrtf = _not_impl("ibwrtf")
  ibwrtkey = _not_impl("ibwrtkey")
  ibxtrc = _not_impl("ibxtrc")
 

if __name__ == "__main__":
    nienet_host = "qo-hpf-gpib1.ethz.ch"
    l = GpibEnet(nienet_host)
    ud = l.ibdev(bna=1, pad=13)
    print l.ibrsp(ud)
    print l.iblines(ud)
    print l.ibtrg(ud)
    print l.ibask(ud, 1)
    print l.ibln(13)
    print l.ibln(11)
    print l.ibwrt(ud, "ID?;")
    print l.ibrd(ud, 10)
    print l.ibwrt(ud, "SET?;")
    print `l.ibrd(ud, 640)`
    print l.ibwrt(ud, "DSTB;")
    print `l.ibrd(ud, 4096)`
    print l.ibbna(ud)
