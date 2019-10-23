import enum
import os
import shutil

from . import _mworks


class ReservedEventCode(enum.IntEnum):

    RESERVED_CODEC_CODE = _mworks.RESERVED_CODEC_CODE
    RESERVED_SYSTEM_EVENT_CODE = _mworks.RESERVED_SYSTEM_EVENT_CODE
    RESERVED_COMPONENT_CODEC_CODE = _mworks.RESERVED_COMPONENT_CODEC_CODE
    RESERVED_TERMINATION_CODE = _mworks.RESERVED_TERMINATION_CODE


class IndexingException(IOError):
    pass


class MWKFile(_mworks._MWKFile):

    _default_min_time = -1 << 63
    _default_max_time = -(_default_min_time + 1)

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

    def get_events_iter(self, codes=(), time_range=(None, None)):
        if not codes:
            codes = []
        else:
            reverse_codec = self.reverse_codec
            codes = [reverse_codec.get(c, c) for c in codes]

        min_time, max_time = time_range
        if min_time is None:
            min_time = self._default_min_time
        if max_time is None:
            max_time = self._default_max_time

        self._select_events(codes, min_time, max_time)

        while True:
            evt = self._get_next_event()
            if evt is None:
                break
            yield evt

    def get_events(self, **kwargs):
        return list(self.get_events_iter(**kwargs))
    
    @property
    def codec(self):
        if self._codec is not None:
            return self._codec
    
        self._select_events([ReservedEventCode.RESERVED_CODEC_CODE],
                            self._default_min_time,
                            self._default_max_time)
        e = self._get_next_event()
        if e is None:
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


class _MWKWriter(_mworks._MWKWriter):
    pass


class _MWK2Writer(_mworks._MWK2Writer):

    def __init__(self, filename, pagesize=0):
        super().__init__(filename, pagesize)
