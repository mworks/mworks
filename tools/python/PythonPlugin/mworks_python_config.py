from importlib.machinery import ExtensionFileLoader, ModuleSpec
import sys

from mworkscore import _StaticExtensionModuleFinder


__all__ = ()


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

# Need to import numpy.core.multiarray here.  Otherwise, the C _import_array
# function won't be able to find it.
import numpy.core.multiarray
