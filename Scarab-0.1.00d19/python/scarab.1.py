import Scarab
import sys
import time
import string
import regex

if len(sys.argv) < 4:
	raise "usage error", "usage: scarab URL OBJECT METHOD [ARGS ...]"

scarab_url = sys.argv[1]
object = sys.argv[2]
method = sys.argv[3]

# Convert the argument list into procedure parameters
arglist = []
for arg in sys.argv[4:]:
	if regex.match("^\s*['\"p({]", arg) != -1:
		arglist.append(eval(arg))
	else:
		arglist.append(arg)

# open a connection to the server
connection = Scarab.connect_to(scarab_url)

# call the method on the remote server
result = connection.call(object, method, arglist)
print result
