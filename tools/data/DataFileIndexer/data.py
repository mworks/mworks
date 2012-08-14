#
#  mworks/data.py
#  DataFileIndexer
#
#  Created by David Cox on 8/17/09.
#  Copyright (c) 2009 Harvard University. All rights reserved.
#

from _data import Event, _MWKFile, _MWKStream
import os
import shutil


class FileNotLoadedException(Exception):
    pass
    
class NoValidCodecException(Exception):
    pass
    
class IndexingException(Exception):
    pass


Event.__module__ = __name__  # So help() thinks Event is part of this module


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

    def _prepare_events_iter(self, **kwargs):
        event_codes = []
    
        if "codes" in kwargs:
            reverse_codec = self.reverse_codec
            if not reverse_codec:
                raise NoValidCodecException
    
            event_codes = kwargs["codes"]

            for i, code in enumerate(event_codes):
                if isinstance(code, basestring) and (code in reverse_codec):
                    event_codes[i] = reverse_codec[code]
        
        if "time_range" in kwargs:
            time_range = kwargs["time_range"]
        else:
            time_range = [self.minimum_time, self.maximum_time]
    
        self._select_events(event_codes, time_range[0], time_range[1])

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
        if not self.loaded:
            raise FileNotLoadedException
        if self._codec is not None:
            return self._codec
    
        self._select_events([0], self.minimum_time, self.maximum_time)
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
        if not self.loaded:
            raise FileNotLoadedException
        if self._reverse_codec is not None:
            return self._reverse_codec
    
        rc = dict((v, k) for k, v in self.codec.iteritems())

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

    def read_event(self):
        result = self._read_event()
        if result.empty:
            result = None
        return result
