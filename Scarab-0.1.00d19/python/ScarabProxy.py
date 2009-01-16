"""  $Id: ScarabProxy.py,v 1.1 2000/03/04 19:38:38 kmacleod Exp $

ScarabProxy implements surrogates for remote objects

"""

class ScarabProxy:

	def __init__(self, connection, object_name):
		self._connection_ = connection
		self._object_ = object_name

	def __repr__(self):
		# Prohibit call to __getattr__
		return '<Agent instance at %s>' % hex(id(self))[2:]

	__str__ = __repr__

	def __getinitargs__(self):
		return ()

	def __getstate__(self):
		"""Create a representation of the Proxy object that
		can be revived later, in another process, or on
		another client"""
		return None

	def __setstate__(self, state):
		"""Revive a Proxy that was created earlier, in another
		process, or on another client"""

	def __getattr__(self, attr):
		"""Normally, this might get an attribute from the
		remote object.  In this case though, we only support
		calls to the remote object, so we'll cache a Method
		object in the __dict__ for future use, and also return
		the Method object for this time."""
		forwarder = ScarabProxy.Method(self._connection_, self._object_, attr)
		self.__dict__[attr] = forwarder
		return forwarder

	def __setattr__(self, attr, value):
		"""Set an attribute in the remote object."""

	class Method:
		"""Represents a method on the remote object.

		Overloads the function call method to look like a function."""

		def __init__(self, connection, object, method):
			"""
			connection	the connection instance the
					method will use
			object	the identifier of the remote object
			method	the method to call on the remote object"""

			self._connection_ = connection
			self._object_ = object
			self._method_ = method

		def __call__(self, *args):
			"""This method enables calling instances as if
			they were functions. It will perform a remote procedure
			call on the server object the agent is connected to."""

			result = self._connection_.call(self._object_, self._method_, args)

			return result
