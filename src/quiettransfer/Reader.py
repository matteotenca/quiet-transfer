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
import io
import json
import struct
import sys
import zlib
from pathlib import Path
from typing import Optional, Any


class CompressFile(io.BytesIO):

    def __init__(self, input_file: str, compress: bool = False, is_script: bool = True):
        super().__init__()
        self._compressor_active = compress
        self.is_script = is_script
        self._input_file = input_file
        self._compressor = zlib.compressobj(level=9) if compress else None
        self._buf = io.BytesIO()
        self.header_size = 0
        self._pointer = 0
        self.crc = ""

        p = Path(self._input_file)
        s = p.stat()
        self._size = s.st_size
        self.size = self._size
        crc = 0
        with open(self._input_file, 'rb', 65536) as ins:
            for x in range(int((self._size / 65536)) + 1):
                crc = zlib.crc32(ins.read(65536), crc)
        fixed_length_hex = '%08x' % (crc & 0xFFFFFFFF)
        self.crc = fixed_length_hex
        self._print_msg(f"Size: {self._size}")
        self._print_msg(f"CRC32: {fixed_length_hex}")
        header = {"size": self._size, "crc32": fixed_length_hex}
        nread = json.dumps(header).encode("utf-8")
        self.header_size = len(nread)
        sz = struct.pack("=L", self.header_size)
        self._buf.write(sz)
        self._buf.write(nread)
        self._buf.seek(0)
        self.header_size += len(sz)
        self._input_stream = open(self._input_file, "rb")
        self._closed = False


    def read(self, size: Optional[int] = None) -> bytes:
        data = b''
        if self._pointer >= (self._size + self.header_size) or self._closed:
            return data
        if self._compressor is not None and size is not None and size < 1024:
            size = 1024
        delta = self.header_size - self._pointer
        if size is not None:
            if delta <= 0:
                if self._compressor_active and self._compressor is not None:
                    while True:
                        data_raw = self._input_stream.read(size)
                        if data_raw:
                            self._pointer += len(data_raw)
                            data = self._compressor.compress(data_raw)
                            if not data:
                                continue
                            break
                        elif self._compressor_active:
                            data = self._compressor.flush()
                            self._compressor_active = False
                            break
                        else:
                            data = b''  # EOF mar
                            break
                else:
                    data = self._input_stream.read(size)
                    self._pointer += len(data)
            else:
                if size <= delta:
                    if self._compressor_active and self._compressor is not None:
                        while True:
                            data_raw = self._buf.read(size)
                            if data_raw:
                                self._pointer += len(data_raw)
                                data = self._compressor.compress(data_raw)
                                if not data:
                                    continue
                                break
                            elif self._compressor_active:
                                data = self._compressor.flush()
                                self._compressor_active = False
                                break
                            else:
                                data = b''  # EOF mar
                                break
                    else:
                        data = self._buf.read(size)
                        self._pointer += len(data)
                elif size > delta:
                    data_header = self._buf.read(delta)
                    self._pointer += delta
                    if self._compressor_active and self._compressor is not None:
                        data_header_compressed = self._compressor.compress(data_header)
                        data_file = b''
                        while True:
                            data_raw = self._input_stream.read(size - delta)
                            if data_raw:
                                self._pointer += len(data_raw)
                                data_file = self._compressor.compress(data_raw)
                                if not data_file:
                                    continue
                                break
                            elif self._compressor_active:
                                data_file = self._compressor.flush()
                                self._compressor_active = False
                                break
                            else:
                                # EOF mar
                                break
                        data = data_header_compressed + data_file
                    else:
                        data_file = self._input_stream.read(size - delta)
                        self._pointer += len(data_file)
                        data += data_header + data_file
        else:
            if self._compressor_active and self._compressor is not None:
                data_header = b''
                if delta > 0:
                    data_header = self._compressor.compress(self._buf.read(delta))
                    self._pointer += delta
                data_file = b''
                while True:
                    data_raw = self._input_stream.read()
                    if data_raw:
                        self._pointer += len(data_raw)
                        data_file = self._compressor.compress(data_raw)
                        if not data_file:
                            continue
                        break
                    elif self._compressor_active:
                        data_file = self._compressor.flush()
                        self._compressor_active = False
                        break
                    else:
                        # EOF mar
                        break
                data = data_header + data_file
            else:
                if delta > 0:
                    data = self._buf.read(delta)
                    self._pointer += delta
                data_file = self._input_stream.read()
                self._pointer += len(data_file)
                data += data_file
        return data

    def gflush(self) -> bytes:
        data = b''
        if self._compressor_active and self._compressor is not None:
            data = self._compressor.flush(zlib.Z_FULL_FLUSH)
            self._compressor_active = False
        return data

    def close(self) -> None:
        if not self._closed:
            self._input_stream.close()
            self._buf.close()
            self._closed = True
            super().close()

    def _print_msg(self, msg: str, **kwargs: Any) -> None:
        if self.is_script:
            print(msg, flush=True, file=sys.stderr, **kwargs)
