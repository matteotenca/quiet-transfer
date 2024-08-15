"""
        Pyquietlib - a tool to transfer files encoded in audio
        Copyright (C) 2024 Matteo Tenca

        This program is free software: you can redistribute it and/or modify
        it under the terms of the GNU General Public License as published by
        the Free Software Foundation, either version 3 of the License, or
        (at your option) any later version.

        This program is distributed in the hope that it will be useful,
        but WITHOUT ANY WARRANTY; without even the implied warranty of
        MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
        GNU General Public License for more details.

        You should have received a copy of the GNU General Public License
        along with this program.  If not, see <https://www.gnu.org/licenses/>.
"""
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
