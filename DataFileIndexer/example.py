import sys; sys.path.append("/Library/Application Support/MonkeyWorks/Scripting/Python")
import monkeyworks.data as d

#test = d.MWKFile("/Documents/MonkeyWorks/Data/wftest1.mwk")
test = d.MWKFile("/Documents/MonkeyWorks/Data/ettest2.mwk")
test.open()

print("Min time: %d, Max time: %d" % (test.minimum_time, test.maximum_time))

print("getting all events")
all_events = test.get_events()

print("n events = %d" % len(all_events))

# MWKFile.get_events:
# codes can be a list of event codes, event names, or a mixed list of both
# if codes is not specified, all codes are retrieved
# time_range specifies the range (in us) to search for events
# if not specified, it defaults to [file.minimum_time file.maximum_time]
print("getting specific events")
e = test.get_events(codes=["#announceMessage", 6], time_range=[139087374, 150000000])

reverse_codec = test.reverse_codec
codec = test.codec

print(reverse_codec)
for evt in e:
    print("Code = %d (%s), Time = %d, Value = %s\n" % (evt.code, codec[evt.code], evt.time, evt.value))
