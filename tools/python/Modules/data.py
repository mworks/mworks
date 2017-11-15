import os
import shutil

from ._mworks import ReservedEventCode, _MWKFile, _MWKStream


class IndexingException(IOError):
    pass


class MWKFile(_MWKFile):

    def __init__(self, file_name):
        super(MWKFile, self).__init__(file_name)
        self._codec = None 
        self._reverse_codec = None

    def close(self):
        super(MWKFile, self).close()
        self._codec = None
        self._reverse_codec = None

    def __enter__(self):
        self.open()
        return self

    def __exit__(self, type, value, tb):
        self.close()

    @property
    def exists(self):
        return os.path.exists(self.file)

    def _prepare_events_iter(self, codes=(), time_range=(None, None)):
        if not codes:
            codes = []
        else:
            reverse_codec = self.reverse_codec
            codes = [reverse_codec.get(c, c) for c in codes]

        min_time, max_time = time_range
        if min_time is None:
            min_time = self.minimum_time
        if max_time is None:
            max_time = self.maximum_time

        self._select_events(codes, min_time, max_time)

    def get_events_iter(self, **kwargs):
        self._prepare_events_iter(**kwargs)
        while True:
            evt = self._get_next_event()
            if evt.empty:
                break
            yield evt

    def get_events(self, **kwargs):
        self._prepare_events_iter(**kwargs)
        return self._get_events()
    
    @property
    def codec(self):
        if self._codec is not None:
            return self._codec
    
        self._select_events([ReservedEventCode.RESERVED_CODEC_CODE],
                            self.minimum_time,
                            self.maximum_time)
        e = self._get_next_event()
        if e.empty:
            self._codec = {}
            return self._codec

        raw_codec = e.value
        codec = dict((key, raw_codec[key]["tagname"]) for key in raw_codec)
        
        self._codec = codec
        return codec
    
    @property
    def reverse_codec(self):
        if self._reverse_codec is not None:
            return self._reverse_codec
    
        rc = dict((v, k) for k, v in self.codec.items())

        self._reverse_codec = rc
        return rc
    
    def reindex(self):
        self.close()
        self.unindex()
        self.open()
    
    # erases all contents in the directory except the original mwk file.
    def _empty_dir(self):    # original DDC's unindex().
        if(os.path.isdir(self.file)):
            split_file_name = os.path.split(self.file)
            file_name = split_file_name[-1:][0]
            parent_path = os.pathsep.join(split_file_name[0:-1])
            
            true_mwk_file = os.path.join(self.file, file_name)
                
            #print "parent_path: ", parent_path
            #print "file_name: ", file_name
            #print "true_mwk_file; ", true_mwk_file
            
            aside_path =  os.path.join(parent_path, file_name + ".aside")
            
            os.rename(self.file, aside_path)
            #print "rename %s to %s" % ( self.file, aside_path)
            
            os.rename(os.path.join(aside_path, file_name), os.path.join(parent_path,file_name) )
            #print "rename %s to %s" % ( os.path.join(aside_path, file_name), os.path.join(parent_path,file_name) )
            
            shutil.rmtree(aside_path, True)        # del tree ignoring errors
            #print "remove %s" % aside_path
            
        else:
            raise IndexingException("Attempt to re-index a file that has not yet been indexed")

    def unindex(self, empty_dir=True):
        if empty_dir:   # erase all files except .mwk
            self._empty_dir()
            return True
        if not os.path.isdir(self.file): return False

        # only erase the .idx file
        file_name = os.path.basename(self.file)
        idx_file = os.path.join(self.file, file_name + '.idx')
        if os.path.isfile(idx_file):
            os.remove(idx_file)
            return True
        else:
            return False


class MWKStream(_MWKStream):

    @classmethod
    def _create_file(cls, filename):
        super(MWKStream, cls)._create_file(filename)
        return cls.open_file(filename, _writable=True)

    @classmethod
    def open_file(cls, filename, _writable=False):
        uri = ('ldobinary:file%s://%s' %
               (('' if _writable else '_readonly'), filename))
        stream = cls(uri)
        stream.open()
        return stream

    def __enter__(self):
        return self

    def __exit__(self, type, value, tb):
        self.close()

    def __iter__(self):
        while True:
            try:
                yield self._read_event()
            except EOFError:
                break

    def read_event(self):
        try:
            return self._read_event()
        except EOFError:
            pass
