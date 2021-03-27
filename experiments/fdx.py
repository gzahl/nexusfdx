# This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

from pkg_resources import parse_version
import kaitaistruct
from kaitaistruct import KaitaiStruct, KaitaiStream, BytesIO
from enum import Enum


if parse_version(kaitaistruct.__version__) < parse_version('0.9'):
    raise Exception("Incompatible Kaitai Struct Python API: 0.9 or later is required, but you have %s" % (kaitaistruct.__version__))

class Fdx(KaitaiStruct):

    class Bytetype(Enum):
        data = 0
        header = 1
    def __init__(self, _io, _parent=None, _root=None):
        self._io = _io
        self._parent = _parent
        self._root = _root if _root else self
        self._read()

    def _read(self):
        self.magic = self._io.read_bytes(1)
        if not self.magic == b"\x00":
            raise kaitaistruct.ValidationNotEqualError(b"\x00", self.magic, self._io, u"/seq/0")
        self.fdx = [None] * (100)
        for i in range(100):
            self.fdx[i] = Fdx.Package(self._io, self, self._root)


    class Package(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.abyte = []
            i = 0
            while True:
                _ = Fdx.Dualbyte(self._io, self, self._root)
                self.abyte.append(_)
                if _.type == Fdx.Bytetype.header:
                    break
                i += 1


    class Unknown(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.somebytes = self._io.read_bytes_full()


    class Dummy(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            pass


    class Dualbyte(KaitaiStruct):

        class Datatype(Enum):
            package41 = 65
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            _on = self.type
            if _on == Fdx.Bytetype.header:
                self._raw_payload = self._io.read_bytes(1)
                _io__raw_payload = KaitaiStream(BytesIO(self._raw_payload))
                self.payload = Fdx.Unknown(_io__raw_payload, self, self._root)
            elif _on == Fdx.Bytetype.data:
                self._raw_payload = self._io.read_bytes(1)
                _io__raw_payload = KaitaiStream(BytesIO(self._raw_payload))
                self.payload = Fdx.Unknown(_io__raw_payload, self, self._root)
            else:
                self.payload = self._io.read_bytes(1)
            self.type = KaitaiStream.resolve_enum(Fdx.Bytetype, self._io.read_bits_int_le(8))


    class Datatype(KaitaiStruct):
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.data = self._io.read_bytes(1)


    class Headerbyte(KaitaiStruct):

        class Packagetype(Enum):
            data = 0
            sender = 1
        def __init__(self, _io, _parent=None, _root=None):
            self._io = _io
            self._parent = _parent
            self._root = _root if _root else self
            self._read()

        def _read(self):
            self.type = self._io.read_bits_int_le(1) != 0
            self.payload = self._io.read_bits_int_le(7)



