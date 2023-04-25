from collections import namedtuple
import enum
import os
import sqlite3
import zlib

import msgpack


class ReservedEventCode(enum.IntEnum):

    RESERVED_CODEC_CODE = 0
    RESERVED_SYSTEM_EVENT_CODE = 1
    RESERVED_COMPONENT_CODEC_CODE = 2
    RESERVED_TERMINATION_CODE = 3


class Event(namedtuple('_Event', ('code', 'time', 'data'))):

    __slots__ = ()

    @property
    def value(self):
        return self.data


class MWK2Reader:

    _compressed_text_type_code = 1
    _compressed_msgpack_stream_type_code = 2

    def __init__(self, filename):
        self._conn = sqlite3.connect(filename)
        self._conn.execute('PRAGMA locking_mode = EXCLUSIVE')

        self.codec = {}
        self.reverse_codec = {}

        raw_codecs = [data for (_, _, data) in
                      self.events([ReservedEventCode.RESERVED_CODEC_CODE])]
        if raw_codecs:
            assert all(map((lambda rc: rc == raw_codecs[0]), raw_codecs[1:])), \
                'MWK2 file contains conflicting codecs'
            self.codec = dict((code, props['tagname'])
                              for code, props in raw_codecs[0].items())
            self.reverse_codec = dict((tagname, code)
                                      for code, tagname in self.codec.items())

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()

    def __getattr__(self, name):
        # These attributes are expensive to compute, so do so only on demand,
        # and cache the results
        if name == 'num_events':
            value = sum(1 for _ in self)
        elif name == 'time_range':
            value = self._conn.execute('SELECT min(time), max(time) '
                                       'FROM events').fetchone()
        else:
            raise AttributeError(name)
        setattr(self, name, value)
        return value

    def __iter__(self):
        return self.events()

    def close(self):
        self._conn.close()

    @staticmethod
    def _decompress(data):
        return zlib.decompress(data, -15)

    def events(self, codes=(), min_time=None, max_time=None):
        where = []
        params = []

        if codes:
            where.append('code IN (?' + ', ?' * (len(codes) - 1)  + ')')
            params.extend([self.reverse_codec.get(c, c) for c in codes])
        if min_time is not None:
            where.append('time >= ?')
            params.append(min_time)
        if max_time is not None:
            where.append('time <= ?')
            params.append(max_time)

        sql = 'SELECT code, time, data FROM events'
        if where:
            sql += ' WHERE ' + ' AND '.join(where)

        unpacker = msgpack.Unpacker(strict_map_key=False)

        for code, time, data in self._conn.execute(sql, params):
            if not isinstance(data, bytes):
                yield Event(code, time, data)
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
                    yield Event(code, time, obj)


class MWKFile:

    def __init__(self, file_name):
        self._filename = file_name
        self._reader = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, type, value, tb):
        self.close()

    @property
    def file(self):
        return self._filename

    @property
    def exists(self):
        return os.path.exists(self._filename)

    @property
    def loaded(self):
        return (self._reader is not None)

    @property
    def valid(self):
        return self.loaded

    def _require_reader(self):
        if not self._reader:
            raise IOError('Data file is not open')

    @property
    def num_events(self):
        self._require_reader()
        return self._reader.num_events

    @property
    def minimum_time(self):
        self._require_reader()
        return self._reader.time_range[0]

    @property
    def maximum_time(self):
        self._require_reader()
        return self._reader.time_range[1]

    @property
    def codec(self):
        self._require_reader()
        return self._reader.codec

    @property
    def reverse_codec(self):
        self._require_reader()
        return self._reader.reverse_codec

    def open(self):
        if self._reader:
            raise IOError('Data file is already open')
        try:
            self._reader = MWK2Reader(self._filename)
        except sqlite3.OperationalError:
            raise RuntimeError('Cannot open data file')

    def close(self):
        if self._reader:
            self._reader.close()
            self._reader = None

    def get_events_iter(self, codes=(), time_range=(None, None)):
        self._require_reader()
        min_time, max_time = time_range
        if ((min_time is not None) and
            (max_time is not None) and
            (min_time > max_time)):
            raise RuntimeError('Minimum event time must be less than or equal '
                               'to maximum event time')
        return self._reader.events(codes, min_time, max_time)

    def get_events(self, **kwargs):
        return list(self.get_events_iter(**kwargs))

    def reindex(self):
        pass

    def unindex(self, empty_dir=True):
        pass
