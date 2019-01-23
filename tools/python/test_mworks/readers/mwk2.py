from __future__ import division, print_function, unicode_literals
import sqlite3

import msgpack

try:
    buffer
except NameError:
    buffer = bytes


class MWK2Reader(object):

    def __init__(self, filename):
        self._conn = sqlite3.connect(filename)
        self._unpacker = msgpack.Unpacker(raw=False)

    def close(self):
        self._conn.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()

    def __iter__(self):
        for code, time, data in self._conn.execute('SELECT * FROM events'):
            if not isinstance(data, buffer):
                yield (code, time, data)
            else:
                self._unpacker.feed(data)
                try:
                    while True:
                        yield (code, time, self._unpacker.unpack())
                except msgpack.OutOfData:
                    pass
