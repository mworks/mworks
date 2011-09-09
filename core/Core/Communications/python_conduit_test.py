from mworks.conduit import *
import time

server = IPCServerConduit("test_resource")
client = IPCClientConduit("test_resource")

# define some dummy functions
counter_a = 0
counter_b = 0

def increment_a(evt):
    global counter_a
    counter_a += evt.data
    print("counter_a = %f" % counter_a)
    
def increment_b(evt):
    global counter_b
    counter_b += evt.data
    print("counter_b = %f" % counter_b)

server.initialize()
client.initialize()

# have the server and client register different codes
# for the event "test"
server.register_local_event_code(4, "test")
time.sleep(0.2)

client.register_local_event_code(6, "test")
time.sleep(0.2)


server.register_callback_for_name("test", increment_a)
time.sleep(0.2)

client.register_callback_for_name("test", increment_b)
time.sleep(0.2)

# from the server's perspective, 4 == test
server.send_data(4, 2.0)
time.sleep(1)

assert(counter_b == 2.0)

client.send_data(6, 1.5)
time.sleep(1)

assert(counter_a == 1.5)

print("Client codec: %s" % client.codec)
print("Client reverse codec: %s" % client.reverse_codec)

print("Server codec: %s" % server.codec)
print("Server reverse codec: %s" % server.reverse_codec)


server.finalize()
client.finalize()
