"""
ScarabMarshal is derived from xml.marshal and pickle

"""

from StringIO import StringIO
#how about this instead:
#try:
#    import cStringIO
#    StringIO = cStringIO
#except ImportError:
#    import StringIO

PicklingError = "pickle.PicklingError"
UnpicklingError = "pickle.UnpicklingError"

class Marshaler:

    def dump(self, value):
        "Write the value"
        dict = { 'id':0 }
        self.m_init(dict)
        self._marshal(value, dict)
        self.m_finish(dict)

    # Entry point for marshaling.  This function gets the name of the 
    # type of the object being marshaled, and calls the
    # m_<typename> method.
    #
    # dict is a dictionary whose keys are used to store the IDs of
    # objects that have already been marshaled, in order to allow
    # writing a reference to them.
    #
    # XXX there should be some way to disable the automatic generation of
    # references to already-marshaled objects

    def _marshal(self, value, dict):
        t = type(value)
        i = str( id(value) )
        if dict.has_key( i ):
            self.m_reference(value, dict)
        else:
            if type(value) == type(1L): meth = 'm_long'
            else: meth = "m_" + type(value).__name__
            getattr(self,meth)(value, dict) 

    def m_init(self, dict):
        "Perform any initialization before writing first object"

    def m_finish(self, dict):
        "Perform any cleanup after writing last object"

    # The following types are specifically Python types, not any types
    # defined by lower-level marshaling modules.  When porting to other
    # languages, use the set of types defined by your language.

    def m_reference(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle references"

    def m_string(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle strings"

    def m_int(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle ints"

    def m_float(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle floats"

    def m_long(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle longs"

    def m_tuple(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle tuples"

    def m_list(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle lists"

    def m_dictionary(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle dictionaries"

    def m_None(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle nones"

    def m_complex(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle complexes"

    def m_code(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle code"

    def m_instance(self, value, dict):
        raise PicklingError, \
              self.__class__.__name__ + " can't pickle instances"

class Unmarshaler:

    def load(self):
        "Unmarshal one value from the stream"
        self.um_init()
        o = self._unmarshal()
        self.um_finish()
        return o

    def um_init(self):
        "Perform any initialization before reading first object"

    def um_finish(self):
        "Perform any cleanup after reading last object"

def test(load, loads, dump, dumps, test_values,
         do_assert = 1):
    # Try all the above bits of data
    import StringIO

    for item in test_values:
        s = dumps(item)
        print item, s
        output = loads(s)
        # Try it from a file
        file = StringIO.StringIO()
        dump(item, file)
        file.seek(0)
        output2 = load(file)

#        if do_assert:
#            assert item==output and item==output2 and output==output2

# Classes used in the test suite
class _A: 
    def __repr__(self): return '<A instance>'
class _B: 
    def __repr__(self): return '<B instance>'

def runtests(load, loads, dump, dumps):
    print "Testing marshaling..."

    L = [None, 1, pow(2,123L), 19.72, 1+5j, 
         "here is a string & a <fake tag>",
         (1,2,3), 
         ['alpha', 'beta', 'gamma'],
         {'key':'value', 1:2}
         ]
    test(load, loads, dump, dumps, L)

    instance = _A() ; instance.subobject = _B() 
    instance.subobject.list=[None, 1, pow(2,123L), 19.72, 1+5j, 
                             "here is a string & a <fake tag>"]
    # FIXME don't test recursion yet
    #instance.self = instance
    L = [ instance ]

    test(load, loads, dump, dumps, L, do_assert=0)

    # FIXME don't test recursion yet
    #recursive_list = [None, 1, pow(3,65L), {1:'spam', 2:'eggs'},
    #                  '<fake tag>', 1+5j ]
    #recursive_list.append( recursive_list )
    #test(load, loads, dump, dumps, [ recursive_list ], do_assert=0)

if __name__ == '__main__':
    runtests(load, loads, dump, dumps)
