import os
import sys
try:
    if sys.platform == 'win32':
        os.add_dll_directory(os.path.join(os.path.dirname(__file__), "dll_win32"))
        from ._pyquietlibwin32 import lib, ffi
    else:
        from ._pyquietlibposix import lib, ffi
except ImportError as im:
    print(f'Error importing from pyquielib binary: '
          f'path {im.path}, name {im.name}, imported from {im.name_from}',
          file=sys.stderr, flush=True)
    raise im

profile_file = os.path.join(os.path.dirname(__file__), "quiet-profiles.json")
__version__ = '0.1.4'
__all__ = ["lib", "ffi", "profile_file"]
