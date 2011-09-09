# Echo has one method, `echo'.  `echo' returns it's first argument
# back to the caller.

class Echo:

	def echo(self, string):
		return string

# Echo server program
import Scarab

server = Scarab.server_on()
server.globals['root'] = Echo()

print "Server listening on: " + server.url

server.run_loop()
