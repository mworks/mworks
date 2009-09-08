import monkeyworks.data as d

test = d.MWKFile("/Documents/MonkeyWorks/Data/data_file_test.mwk/data_file_test.mwk")
test.open()

print("Min time: %d, Max time: %d" % (test.minimum_time, test.maximum_time))

# MWKFile.get_events:
# codes can be a list of event codes, event names, or a mixed list of both
# if codes is not specified, all codes are retrieved
# time_range specifies the range (in us) to search for events
# if not specified, it defaults to [file.minimum_time file.maximum_time]
e = test.get_events(codes=["#announceMessage", 6], time_range=[139087374, 150000000])

reverse_codec = test.reverse_codec
codec = test.codec

print(reverse_codec)
for evt in e:
    print("Code = %d (%s), Time = %d, Value = %s\n" % (evt.code, codec[evt.code], evt.time, evt.value))
