import sys
import string

MAGIC            = "\x89" + "CBF"
MAJOR            = 0
MINOR            = 0

VERSION          = "\x01"

INTEGER_N        = "\x02"
INTEGER_P        = "\x03"
FLOAT_NN         = "\x04"
FLOAT_NP         = "\x05"
FLOAT_PN         = "\x06"
FLOAT_PP         = "\x07"
FLOAT_INF        = "\x08"
FLOAT_NAN        = "\x09"
OPAQUE           = "\x0A"
NULL             = "\x0B"
LIST             = "\x0C"
DICTIONARY       = "\x0D"

DEFINE_REFERENCE = "\x0E"
REFERENCE        = "\x0F"

ATTRIBUTES       = "\x12"

tag = {
    INTEGER_N        : "INTEGER_N:",
    INTEGER_P        : "INTEGER_P:",
    FLOAT_NN         : "FLOAT_NN:",
    FLOAT_NP         : "FLOAT_NP:",
    FLOAT_PN         : "FLOAT_PN:",
    FLOAT_PP         : "FLOAT_PP:",
    FLOAT_INF        : "FLOAT_INF\n",
    FLOAT_NAN        : "FLOAT_NAN\n",
    OPAQUE           : "OPAQUE:",
    NULL             : "NULL\n",
    LIST             : "LIST:",
    DICTIONARY       : "DICTIONARY:",

    REFERENCE        : "REFERENCE:",
}

class LDOBinaryDumper:

    def dump(self, stream):
        self.read = stream.read
        magic = self.read(len(MAGIC))
        print "MAGIC: ",
        self.dump_bytes(magic)
        print
        if (magic != MAGIC):
            print "***ERROR: incorrect magic"
            exit
        a_byte = self.read(1)
        if (a_byte == VERSION):
            print "VERSION: ",
            self.decode_ber()
            print ".",
            self.decode_ber()
            print
        else:
            print "***ERROR: missing VERSION"
            exit

        self.decode_item(0)

    def decode_item(self, depth):
        id = ""
        key = self.read(1)
	if key == DEFINE_REFERENCE:
            print "  " * depth + "DEFINE REFERENCE: ",
	    self.decode_ber()
            print
	    key = self.read(1)

	if key == ATTRIBUTES:
            print "  " * depth + "ATTRIBUTES: "
            self.decode_item()
	    key = self.read(1)

        try:
            print "  " * depth + tag[key],
        except KeyError:
	    print "***ERROR unknown field tag: " + hex(ord(key))
            sys.exit()

        depth = depth + 1
        if ( key == LIST or key == DICTIONARY or key == OPAQUE ):
            length = self.decode_ber()
            print

        if ( key == LIST ):
            for k in range(length):
                self.decode_item(depth)

        if ( key == DICTIONARY ):
            for k in range(length):
                if k > 0:
                    print
                self.decode_item(depth)
                self.decode_item(depth)

        if ( key == OPAQUE ):
            bytes = self.read(length)
            print "  " * depth,
            self.dump_bytes(bytes)
            print

        if ( key == INTEGER_N or key == INTEGER_P ):
            self.decode_ber()
            print

        if ( key == FLOAT_NN or key == FLOAT_NP
             or key == FLOAT_PN or key == FLOAT_PP ):
            self.decode_ber()
            print "EXP:",
            self.decode_ber
            print

        if ( key == REFERENCE ):
            self.decode_ber()

    def decode_ber(self):
	d = 0
        a_byte = self.read(1)
        #self.dump_bytes(a_byte)
        #print " ",
	octet = ord(a_byte)
	while octet & 0x80:
	    d = (d << 7) + (octet & 0x7f)
            a_byte = self.read(1)
            #self.dump_bytes(a_byte)
            #print " ",
	    octet = ord(a_byte)
        d = (d << 7) + octet
        # print "== " + str(d)
        print str(d),
	return d

    def dump_bytes(self, bytes):
        n = len(bytes)
        for k in range(n):
            if (ord(bytes[k]) >= 32 and ord(bytes[k]) <= 127):
                print bytes[k],
            else:
                print "\\%o" % (ord(bytes[k])),

if __name__ == '__main__':
    LDOBinaryDumper().dump(sys.stdin)
