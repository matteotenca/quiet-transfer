import sys
from cffi import FFI
from pathlib import Path

ffibuilder = FFI()
if sys.platform == 'win32':
    ffi_include = Path().absolute().joinpath("include_win32").joinpath("quiet_portaudio_all_cleaned_cffi.h").as_posix()
    with open(ffi_include) as h_file:
        ffibuilder.cdef(h_file.read())
        ffibuilder.set_source("pyquietlib._pyquietlibwin32",  # name of the output C extension
                              """
                                    #include "portaudio.h"
                                    #include "quiet_portaudio_all_cleaned.h"
                                    
                                    """,
                              libraries=["quiet", "jansson", "fec", "liquid", "portaudio"],
                              include_dirs=[Path().absolute().joinpath("include_win32").as_posix()],
                              library_dirs=[Path().absolute().joinpath("lib_win32").as_posix()],
                              )
else:
    ffi_include = Path().absolute().joinpath("include_posix").joinpath("quiet_cffi.h").as_posix()
    with open(ffi_include) as h_file:
        ffibuilder.cdef(h_file.read())
        ffibuilder.set_source("pyquietlib._pyquietlibposix",  # name of the output C extension
                              """
                                    #include "portaudio.h"
                                    #include "quiet.h"
                                    #include "quiet-portaudio.h"
                                    """,
                              libraries=["quiet", "jansson", "fec", "liquid", "portaudio"],
                              include_dirs=[Path().absolute().joinpath("include_posix").as_posix()],
                              )

if __name__ == "__main__":
    ffibuilder.compile(verbose=2)
