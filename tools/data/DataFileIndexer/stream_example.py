import os
import sys; sys.path.append("/Library/Application Support/MWorks/Scripting/Python")
import mworks.data as d

#filename = os.path.expanduser("~/Documents/MWorks/Data/ettest1.mwk/ettest1.mwk")
filename = os.path.expanduser("~/Documents/MWorks/Data/wftest1.mwk/ettest1.mwk")

stream = d.MWKStream("ldobinary:file://" + filename)
stream.open()

print("Reading...")
ev = stream.read_event()

while ev is not None:
    print("Got event: code = %d, time = %d, value = %s" % (ev.code, ev.time, ev.value))
    ev = stream.read_event()

stream.close()