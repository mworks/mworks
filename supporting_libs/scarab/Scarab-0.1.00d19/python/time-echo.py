import Scarab
import sys
import time
import string

if len(sys.argv) != 2 and len(sys.argv) != 3:
	raise "usage error", "usage: tim-echo URL [COUNT]"

connection = Scarab.connect_to(sys.argv[1])
if len(sys.argv) == 3:
    count = int(sys.argv[2])
else:
    count = 1000

t1 = time.time()
for ii in range(count):
	connection.call('root', 'echo', ['testing'])
total_time = time.time() - t1
ms = (float(total_time) / float(count)) * 1000.0
print "%d calls in %d seconds, %.2fms/call" % (count, total_time, ms )
