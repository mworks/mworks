import sys; sys.path.append("/Library/Application Support/MWorks/Scripting/Python")

from mworks.conduit import *
import time

# connect as a client to a remote IPC conduit (assumed to be attached to the "live" MW server)
conduit = IPCClientConduit("python_test_conduit")

counter_a = 0
counter_b = 0

def increment_by_one():
    counter_a += 1

def increment_by_two():
    counter_b += 2

conduit.initialize()

conduit.register_callback_for_code(4, increment_by_one)
conduit.register_callback_for_name("#announceBlock", increment_by_two)

# let it be known that on this side of the conduit, 10 = #announceBlock
conduit.register_local_event_code(10, "#announceBlock")

time.sleep(4)

conduit.finalize()

assert counter_a == 3
assert counter_b == 6

conduit.send_data(10, counter_a)
