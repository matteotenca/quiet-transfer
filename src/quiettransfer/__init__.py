"""
        Quiet-Transfer - a tool to transfer files encoded in audio
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
import json
import os
import sys

try:
    if sys.platform.find("win32") >= 0:
        os.add_dll_directory(os.path.join(os.path.dirname(__file__), "dll_win32"))
        from ._quiettransferwin32 import lib, ffi # type: ignore
    elif sys.platform.find("linux") >= 0:
        from ._quiettransferposix import lib, ffi # type: ignore
    elif sys.platform.find("darwin") >= 0:
        from ._quiettransfermacos import lib, ffi # type: ignore
    else:
        raise OSError()
except OSError as e:
    raise OSError from e

from .Reader import CompressFile
from .Send import SendFile
from .Receive import ReceiveFile

profile_file = os.path.join(os.path.dirname(__file__), "quiet-profiles.json")

profile = json.load(open(profile_file))
protocols = profile.keys()

__version__ = "0.2.7"
# noinspection PyUnresolvedReferences
__all__ = ["lib", "ffi", "profile_file", "protocols", "SendFile", "ReceiveFile", "__version__", "CompressFile"]
