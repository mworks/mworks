from importlib.machinery import ExtensionFileLoader, ModuleSpec
import os
import sys

import _mworkspython


__all__ = ()


#
# Add finder for merged extension modules
#

class MergedExtensionModuleFinder:

    def __init__(self):
        self.path = _mworkspython.get_executable_path()

    def find_spec(self, fullname, path, target=None):
        if _mworkspython.have_init_func('PyInit_' + fullname.split('.')[-1]):
            return ModuleSpec(fullname,
                              ExtensionFileLoader(fullname, self.path),
                              origin = self.path)

sys.meta_path.append(MergedExtensionModuleFinder())


#
# Tell OpenSSL where to find the list of trusted root certificates
#

os.environ['SSL_CERT_FILE'] = os.path.join(os.path.dirname(sys.path[0]),
                                           'cacert.pem')
