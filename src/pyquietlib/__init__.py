import os
import sys

if sys.platform == 'win32':
    os.add_dll_directory(os.path.join(os.path.dirname(__file__), "dll_win32"))
    from ._pyquietlibwin32 import lib, ffi
else:
    from ._pyquietlibposix import lib, ffi
from .Send import SendFile
from .Receive import ReceiveFile

profile_file = os.path.join(os.path.dirname(__file__), "quiet-profiles.json")
__version__ = '0.2.0'
__all__ = ["lib", "ffi", "profile_file", "SendFile", "ReceiveFile"]
