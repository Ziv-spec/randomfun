import socket, sys
from struct import *

# debug = ["io", "ignore_not_impl"] # "dummy_io", "rw"
debug = ["ignore_not_impl"]


def _dbg(f, name=None):
    if not name:
        name = f.__name__

    def wrap(self, *a, **k):
        print(f"DBG: {name}: enter:")
        print(a)
        print(k)
        r = f(self, *a, **k)
        print(f"exit: {r}")
        return r

    wrap._dummy = True
    return wrap


def _not_impl(name):
    def wrap(self, *a, **k):
        if "ignore_not_impl" in debug:
            return None
        else:
            raise NotImplementedError(f"{name} not implemented")

    return _dbg(wrap, name)


class EnetSocket(object):
    def __init__(self, host, port=8080):
        self._host = host
        self._port = port
        self._sock = None
        self._open()
        self.sta = self.err = self.cnt = 0

    def _open(self):
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # self._sock.connect((self._host, self._port))


    def _send(self, string):
        return self._sock.send(string)

    def _recv(self, length):
        s = b""
        while len(s) < length:
            s += self._sock.recv(length - len(s))
        return s

    def _close(self):
        self._sock.close()
        self._sock = None

    if "dummy_io" in debug:
        _open = lambda self: None
        _send = lambda self, s: sys.stderr.write(f"DBG: > {s}")
        _recv = lambda self, len: input(f"DBG: < #{len}:")[:len]
        _close = lambda self: None

    if "io" in debug:
        _open = _dbg(_open)
        _recv = _dbg(_recv)
        _send = _dbg(_send)
        _close = _dbg(_close)

    _headfmt = "!H H"

    def _read_frags(self, many=False):
        while True:
            header = self._recv(calcsize(self._headfmt))
            flags, num = unpack(self._headfmt, header)
            if not many or not flags:
                yield self._recv(num)
            else:
                break
            if not many:
                break

    def _read(self, many=False):
        return b"".join(self._read_frags(many))

    def _write(self, string):
        return self._send(string)

    _respfmt = "!H H 4x L"

    def _sresp(self):
        ret = self._read()
        self.sta, self.err, self.cnt = unpack(
            self._respfmt, ret[: calcsize(self._respfmt)]
        )
        return ret[calcsize(self._respfmt) :]

    def _scmd(self, id, argsfmt="", *args):
        # assert calcsize("!B" + argsfmt) == 12
        # pad to 12 bytes
        argsfmt += "%dx" % (12 - calcsize("!B" + argsfmt))
        tw = pack("!B" + argsfmt, *((id,) + args)); print(tw)
        # self._write(pack("!B" + argsfmt, *((id,) + args)))
        return self._sresp()

    def ibdev(self, pad, sad=0, tmo=13, eot=1, eos=0):
        if sad != 0:
            pad |= 0x80
        self._scmd(
            0x07, "BBBBBBBB", 1, 0, 0x40 | eot, pad, sad, eos, 0, tmo
        )  # , "\x02\x04\x00")
        self._scmd(
            0x07, "BBBBBBBB", 0, 0x5C, 0x40 | 1, 0, 0, 0, 0, 13
        )  # , "\x02\x04\x00")
        self.ibonl(1)

    def ibask(self, cfg):
        self._scmd(0x4E, "B", cfg)
        #      "\x00\x00\x10\x00\x00\x00\x40\x63\x16\x40")
        return self.err

    def ibconfig(self, cfg, val):
        self._scmd(0x06, "B B", cfg, val)

    #      "\x08\x00\x00\x00\x00\x00\x54\x00\x00")
    # return self.err # prevval

    def ibwait(self, mask=0):
        self._scmd(0x22, "B H", 0x54, mask)

    #      "\x20\xe1\x05\x08\xb4\xe0\x05\x08")

    def ibrsp(self):
        (stb,) = unpack("!B", self._scmd(0x19))
        #      "\x63\x16\x40\xc0\x58\x16\x40\x40\x63\x16\x40"))
        return stb

    def ibonl(self, val=0):
        self._scmd(0x12, "B", val)

    #      "\x00\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")

    def ibclr(self):
        self._scmd(0x04)

    #      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")

    def ibeos(self, val):
        self._scmd(0x08, "H", val)

    #      "\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")

    def ibeot(self, val):
        self._scmd(0x09, "B", val)

    #      "\x00\x00\x20\xe1\x05\x08\xb3\xe0\x05\x08")

    def iblines(self):
        (lines,) = unpack("!H", self._scmd(0x0D))
        #      "\x63\x16\x40\xc0\x58\x16\x40\x40\x63\x16\x40"))
        return lines

    def ibln(self, pad, sad=0):
        if sad != 0:
            pad |= 0x80
        (listen,) = unpack("!H", self._scmd(0x0F, "B B", pad, sad))
        #      "\x00\xf0\x38\x06\x08\x03\x00\x00\x00"))
        return listen

    def ibloc(self):
        self._scmd(0x10)

    #      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")

    def ibtmo(self, tmo):
        self._scmd(0x1F, "B", tmo)

    #      "\x00\x00\x20\xe1\x05\x08\xae\xe0\x05\x08")

    def ibtrg(self):
        self._scmd(0x20)

    #      "\xf5\xff\xbf\x14\xf5\xff\xbf\xa9\x8f\x04\x08")

    def ibcac(self, val=1):
        self._scmd(0x03, "B", val)

    #      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")

    def ibgts(self, val=1):
        self._scmd(0x0A, "B", val)

    #      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")

    def ibrsc(self, val=1):
        self._scmd(0x18, "B", val)

    #      "\x00\x00\x20\xe1\x05\x08\xb1\xe0\x05\x08")

    def ibsic(self):
        self._scmd(0x1C)

    #      "\xe1\x05\x08\xb1\xe0\x05\x08\x88\xf5\xff\xbf")

    def ibwrt(self, string):
        self._scmd(0x23, "3s I", b"\x05\x05\x08", len(string))
        #     "\x00\x54\x00\x00")
        self._write(string)
        self._sresp()

    def ibrd(self, num):
        self._scmd(0x16, "3s I", b"\x00\x00\x00", num)
        #     "\x40\x63\x16\x40")
        ret = self._read(many=True)
        self._sresp()
        return ret

    if "rw" in debug:
        ibwrt = _dbg(ibwrt)
        ibrd = _dbg(ibrd)

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


class EnetLib(object):
    def __init__(self, host, port=5000):
        self._host = host
        self._port = port
        self._uds = {0: None}

    def _wrap_ud(self, name):
        def wrapped(ud, *a, **ka):
            res = getattr(self._uds[ud], name)(*a, **ka)
            if res == None:
                return self._uds[ud].sta
            else:
                return self._uds[ud].sta, res

        return wrapped

    # TODO: cfg
    def ibfind(self, name):
        if name[:3] == "dev":
            pad = int(name[3:])
        else:
            raise ValueError("configuration not yet implemented. use devX")
        return self.ibdev(pad)

    def ibdev(self, *a, **ka):
        ud = max(self._uds.keys()) + 1
        self._uds[ud] = EnetSocket(self._host, self._port)
        self._uds[ud].ibdev(*a, **ka)
        return ud

    def ibonl(self, ud, val):
        self._uds[ud].ibonl(val)
        sta = self._uds[ud].sta
        if not val:
            del self._uds[ud]
        return sta

    def ibsta(self):
        return 0

    def iberr(self):
        return 0

    def ibcntl(self):
        return 0

    ibcnt = ibcntl

    def __getattr__(self, name):
        return self._wrap_ud(name)


class GPIBENET:
    def __init__(self, host, addr) -> None:
        self.l = EnetLib(host)
        self.ud = self.l.ibdev(pad=addr, sad=0, tmo=10, eot=1, eos=0)

    def write(self, s):
        self.l.ibwrt(self.ud, s)

    def read(self):
        status, resp = self.l.ibrd(self.ud, 4096)
        return resp

    def query(self, s):
        self.write(s)
        return self.read()

if __name__ == "__main__":
    host =  socket.gethostbyname("localhost") # "192.168.100.97"
    print(host)
    d = GPIBENET(host, 16)
    d.query(b"*IDN?").decode()

