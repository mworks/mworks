import sqlite3
import zlib

import msgpack


class MWK2Reader:

    _compressed_text_type_code = 1
    _compressed_msgpack_stream_type_code = 2

    def __init__(self, filename):
        self._conn = sqlite3.connect(filename)

    def close(self):
        self._conn.close()

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()

    @staticmethod
    def _decompress(data):
        return zlib.decompress(data, -15)

    def __iter__(self):
        unpacker = msgpack.Unpacker(strict_map_key=False)
        for code, time, data in self._conn.execute('SELECT * FROM events'):
            if not isinstance(data, bytes):
                yield (code, time, data)
            else:
                unpacker.feed(data)
                for obj in unpacker:
                    if isinstance(obj, msgpack.ExtType):
                        if obj.code == self._compressed_text_type_code:
                            obj = self._decompress(obj.data).decode('utf-8')
                        elif (obj.code ==
                              self._compressed_msgpack_stream_type_code):
                            unpacker.feed(self._decompress(obj.data))
                            continue
                    yield (code, time, obj)
