import sys; sys.path.append("/Library/Application Support/MonkeyWorks/Scripting/Python")
import monkeyworks.data as d

#filename = "Documents/MonkeyWorks/Data/ettest1.mwk/ettest1.mwk"
filename = "/Documents/MonkeyWorks/Data/wftest1.mwk/ettest1.mwk"

stream = d.MWKStream("ldobinary:file://" + filename)
stream.open()

print("Reading...")
ev = stream.read_event()

while ev is not None:
    print("Got event: code = %d, time = %d, value = %s" % (ev.code, ev.time, ev.value))
    ev = stream.read_event()

stream.close()