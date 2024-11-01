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
import sys
from cffi import FFI
from pathlib import Path

ffibuilder = FFI()
if sys.platform.find("win32") >= 0:
    ffi_include = Path().absolute().joinpath("include_win32").joinpath("quiet_cffi.h").as_posix()
    with open(ffi_include) as h_file:
        ffibuilder.cdef(h_file.read())
        ffibuilder.set_source("quiettransfer._quiettransferwin32",  # name of the output C extension
                              """
                                    #include "quiet.h"
                                    """,
                              libraries=["quiet", "jansson", "fec", "liquid"],
                              include_dirs=[Path().absolute().joinpath("include_win32").as_posix()],
                              library_dirs=[Path().absolute().joinpath("lib_win32").as_posix()],
                              )
elif sys.platform.find("linux") >= 0:
    ffi_include = Path().absolute().joinpath("include_posix").joinpath("quiet_cffi.h").as_posix()
    with open(ffi_include) as h_file:
        ffibuilder.cdef(h_file.read())
        ffibuilder.set_source("quiettransfer._quiettransferposix",  # name of the output C extension
                              """
                                    #include "quiet.h"
                                    """,
                              libraries=["quiet", "jansson", "fec", "liquid"],
                              include_dirs=[Path().absolute().joinpath("include_posix").as_posix()],
                              )
elif sys.platform.find("darwin") >= 0:
    ffi_include = Path().absolute().joinpath("include_macos").joinpath("quiet_cffi.h").as_posix()
    with open(ffi_include) as h_file:
        ffibuilder.cdef(h_file.read())
        ffibuilder.set_source("quiettransfer._quiettransfermacos",  # name of the output C extension
                              """
                                    #include "quiet.h"
                              """,
                              libraries=["quiet", "jansson", "fec", "liquid"],
                              include_dirs=[Path().absolute().joinpath("include_macos").as_posix(),
                                            Path("/tmp/build_deps/staging/usr/include").absolute().as_posix()],
                              library_dirs=[Path("/tmp/build_deps/staging/usr/lib").absolute().as_posix()],
                              )

if __name__ == "__main__":
    ffibuilder.compile(verbose=2)
