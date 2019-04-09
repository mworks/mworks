.. _Legacy Event File Format:

Legacy Event File Format (MWK)
==============================


Description
-----------

MWorks' legacy event file format uses the file extension ``.mwk``.

An MWK file is a simple stream of MWorks events.  Each event is encoded as a list in the `LDO format <https://github.com/mworks/mworks/blob/master/supporting_libs/scarab/Scarab-0.1.00d19/c/encoding_ldo.c>`_ defined by the `Scarab library <https://github.com/mworks/mworks/tree/master/supporting_libs/scarab/Scarab-0.1.00d19>`_.  The list contains the code, time, and data associated with the event.  (The only exception is the `RESERVED_TERMINATION_CODE <system event codes>` event that ends the event file, which has no associated data and is encoded as a two-element list.)

The first time an MWK file is opened using MWorks' standard `data analysis tools <Analyzing Experimental Data>`, it is converted into a directory containing both the original, unmodified MWK file and an index file.  For example::

    $ # Before opening
    $ ls -l data.mwk
    -rw-r--r--  1 cstawarz  staff  7953812 Aug  9  2018 data.mwk

    $ # After opening
    $ ls -l data.mwk/
    total 16400
    -rw-r--r--  1 cstawarz  staff  7953812 Aug  9  2018 data.mwk
    -rw-r--r--  1 cstawarz  staff      686 Aug 10  2018 data.mwk.idx

If the ``.mwk.idx`` file is deleted, the data tools will recreate it the next time the event file is opened.


Example Code
------------

The following `Python <https://www.python.org>`_ code demonstrates the MWK format in detail by a implementing a reader for ``.mwk`` files:

.. literalinclude:: ../../../tools/python/test_mworks/readers/mwk.py
   :language: python

The ``MWKReader`` class defined above can be used as follows:

.. code-block:: python

    with MWKReader('my_data.mwk') as event_file:
        for code, time, data in event_file:
            # Process the current event
            ...
