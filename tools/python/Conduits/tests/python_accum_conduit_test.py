from mworks.conduit import *
import time

server = IPCServerConduit("test_resource")
client = IPCAccumClientConduit("test_resource", "start", "stop",["test"])


received_data = []

def bundle_callback(evts):
    print("I got %d events" % len(evts))
    for i, evt in enumerate(evts):
        print("event[%d].data = %d" % (i, evt.data))
        received_data.append(evt.data)
    

server.initialize()
client.initialize()

# have the server and client register different codes
# for the event "test"
START = 10
STOP = 11
TEST = 12

server.register_local_event_code(START, "start")
server.register_local_event_code(STOP, "stop")
server.register_local_event_code(TEST, "test")

client.register_local_event_code(13, "start")
client.register_local_event_code(14, "stop")
client.register_local_event_code(15, "test")
time.sleep(0.2)



client.register_bundle_callback(bundle_callback)
time.sleep(1)


a = 5
print "sending start..."
server.send_data(START, 0)
server.send_data(TEST, a)
a = a+1
server.send_data(TEST, a)
a = a+1
time.sleep(1)
server.send_data(STOP, 0)
print "sending stop"
time.sleep(2)

assert received_data == [5, 6]

server.finalize()
client.finalize()
