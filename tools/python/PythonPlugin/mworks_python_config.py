from importlib.machinery import ExtensionFileLoader, ModuleSpec
import os
import sys

from _mworkscore import _StaticExtensionModuleFinder


__all__ = ()


#
# Add finder for statically-linked extension modules
#

class StaticExtensionModuleFinder(_StaticExtensionModuleFinder):

    def __init__(self):
        super().__init__()
        self.path = self.get_executable_path()

    def find_spec(self, fullname, path, target=None):
        if self.have_init_func_for_name(fullname.split('.')[-1]):
            return ModuleSpec(fullname,
                              ExtensionFileLoader(fullname, self.path),
                              origin = self.path)

sys.meta_path.append(StaticExtensionModuleFinder())


#
# Tell OpenSSL where to find the list of trusted root certificates
#

os.environ['SSL_CERT_FILE'] = os.path.join(os.path.dirname(sys.path[0]),
                                           'cacert.pem')
