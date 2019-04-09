.. _Current Event File Format:

Current Event File Format (MWK2)
================================


Description
-----------

MWorks' current event file format uses the file extension ``.mwk2``.

Each event file is a standalone `SQLite <https://sqlite.org>`_ database.  The database contains a single table (``events``) with three columns: ``code``, ``time``, and ``data``, corresponding to the three elements of an MWorks event.

In a given row of the ``events`` table, the ``code`` and ``time`` values are always integers, while the type of the ``data`` value can be any of SQLite's `supported datatypes <https://sqlite.org/datatype3.html>`_.  A ``data`` value of type NULL, INTEGER, REAL, or TEXT represents the data for a single event.  A  ``data`` value of type BLOB contains `MessagePack <https://msgpack.org>`_-encoded data for one or more events (all with the same code and time), in one of the following forms:

1. an uncompressed stream of MessagePack-encoded values, with each value of `MessagePack type <https://github.com/msgpack/msgpack/blob/master/spec.md#type-system>`_ nil, boolean, integer, float, string, binary, array, or map
2. a single MessagePack `extension type <https://github.com/msgpack/msgpack/blob/master/spec.md#extension-types>`_ value with type code 1, whose data is a `zlib-compressed <https://developer.apple.com/documentation/compression/compression_algorithm/compression_zlib>`_, `UTF-8 <https://en.wikipedia.org/wiki/UTF-8>`_-encoded string
3. a single MessagePack extension type value with type code 2, whose data is a zlib-compressed stream of MessagePack-encoded values (i.e. like (1) after decompression)


Example Code
------------

The following `Python <https://www.python.org>`_ code demonstrates the MWK2 format in detail by a implementing a simple reader for ``.mwk2`` files.  Apart from the `msgpack <https://pypi.org/project/msgpack/>`_ package, it requires only the Python `standard library <https://docs.python.org/3/library/index.html>`_:

.. literalinclude:: ../../../tools/python/test_mworks/readers/mwk2.py
   :language: python

The ``MWK2Reader`` class defined above can be used as follows:

.. code-block:: python

    with MWK2Reader('my_data.mwk2') as event_file:
        for code, time, data in event_file:
            # Process the current event
            ...
