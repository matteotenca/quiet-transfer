import sys

from setuptools import setup


pdata = {
    "": ["*.json"],
    "pyquietlib.dll_win32": ["*.dll"]
    } if sys.platform == "win32" else {"": ["*.json"]}

setup(
    name="pyquietlib",
    cffi_modules=["compile_cffi.py:ffibuilder"],
    package_dir={"": "src"},
    package_data=pdata,
)

