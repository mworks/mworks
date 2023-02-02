import struct


class LDOError(Exception):
    pass


class LDOReader:

    MAGIC = b'\x89CBF\x01\x00\x00'

    INTEGER_N =  0x02
    INTEGER_P =  0x03
    OPAQUE =     0x0A
    NULL =       0x0B
    LIST =       0x0C
    DICTIONARY = 0x0D
    FLOAT =      0x11

    def __init__(self, file, string_encoding='utf-8'):
        magic = file.read(len(self.MAGIC))
        if not isinstance(magic, bytes):
            raise TypeError('file.read() must return binary data')
        if magic != self.MAGIC:
            raise LDOError('invalid magic')

        def _read(size):
            assert size > 0
            data = file.read(size)
            if not data:
                raise EOFError
            return data
        self._read = _read

        self._readers = {
            self.INTEGER_N: self._read_integer_n,
            self.INTEGER_P: self._read_integer_p,
            self.OPAQUE: self._read_opaque,
            self.NULL: self._read_null,
            self.LIST: self._read_list,
            self.DICTIONARY: self._read_dict,
            self.FLOAT: self._read_float,
            }

        self.string_encoding = string_encoding

    def _read_ord(self):
        return ord(self._read(1))

    def _read_ber(self):
        # Adapted from https://stackoverflow.com/questions/6776553/
        value = 0
        while True:
            tmp = self._read_ord()
            value = (value << 7) | (tmp & 0x7f)
            if tmp & 0x80 == 0:
                return value

    def _read_integer_n(self):
        return -(self._read_ber())

    def _read_integer_p(self):
        return self._read_ber()

    def _read_opaque(self):
        size = self._read_ber()
        value = self._read(size)

        # If value contains exactly one NUL, and that NUL is the last
        # byte in the array, then value is an encoded string.  Strip
        # the NUL and decode with the specified encoding.
        if value.find(b'\0') == len(value) - 1:
            value = value[:-1].decode(self.string_encoding)

        return value

    def _read_null(self):
        return None

    def _read_list(self):
        size = self._read_ber()
        return list(self.read() for _ in range(size))

    def _read_dict(self):
        size = self._read_ber()
        return dict((self.read(), self.read()) for _ in range(size))

    def _read_float(self):
        size = self._read_ber()  # Should always be 8
        return struct.unpack(b'<d', self._read(size))[0]

    def read(self):
        typecode = self._read_ord()
        try:
            return self._readers[typecode]()
        except KeyError:
            raise LDOError('invalid type code (0x%0.2X)' % typecode)


class MWKReader:

    def __init__(self, filename):
        self._fp = open(filename, 'rb')

    def close(self):
        self._fp.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()

    def __iter__(self):
        self._fp.seek(0)
        reader = LDOReader(self._fp)
        while True:
            try:
                event = reader.read()
                assert isinstance(event, list)
                assert len(event) in (2, 3)
                if len(event) == 2:
                    event.append(None)
                yield event
            except EOFError:
                break
