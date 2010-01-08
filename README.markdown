DDC:

As of January 2010, DataFileIndexer is the only project in this repository that is actually known to work.  In particular, it can be used to index and read files into Python.  I didn't write any of this code, aside from going into the indexer project and trying salvage something workable.  It is still awkwardly factored, but it definitely works; on a future release it would be nice to clean up with the structure of this code.  

Other groups will probably want to read data files in Matlab; I have happily not opened Matlab in many months, and I have no idea of whether the code in MatlabDataReader actually works.  I also have no idea what DataFileReader and DataFileReader_legacy were for.