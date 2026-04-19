from ctypes import CDLL
from pathlib import Path
from sys import platform

if platform == "darwin":
    extension = "dylib"
else:
    extension = "so"

# The shared object file we are using the access the c functions via ctypes
path = (Path(__file__).parent / f"../../build_gs/interfaces/libobc-gs-interface.{extension}").resolve()
interface = CDLL(str(path))
