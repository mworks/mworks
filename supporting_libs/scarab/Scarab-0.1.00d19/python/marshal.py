from LDOBinary import *
import sys

p = LDOBinaryMarshaler(sys.stdout)
p.dump( { "testing" : [ 1, 2, 3.1 ] } )
