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
import argparse
import binascii
import io
import json
import os
import struct
import sys
import time
import zlib
from io import FileIO
from pathlib import Path
from typing import Optional, Any

# noinspection PyPackageRequirements
import sounddevice as sd # type: ignore
# noinspection PyPackageRequirements
import soundfile as sf # type: ignore

import quiettransfer


class ReceiveFile:

    def __init__(self, args: Optional[argparse.Namespace] = None,
                 output: Optional[str] = "-", overwrite: bool = False, dump: Optional[str] = None,
                 protocol: str = "audible", input_wav: Optional[str] = None,
                 file_transfer: bool = False, zlb: bool = False) -> None:

        self._lib = quiettransfer.lib
        self._ffi = quiettransfer.ffi
        self._profile_file = quiettransfer.profile_file

        self._script = True if args is not None else False

        if args is not None:
            # called from command line
            self._output_file_name = args.output
            self._overwrite = args.overwrite
            self._protocol = args.protocol
            self._input_wav = args.input_wav
            self._file_transfer = args.file_transfer
            self._dump = args.dump
            self._zlb = args.zlib
        else:
            # called from module
            self._output_file_name = output
            self._overwrite = overwrite
            self._protocol = protocol
            self._input_wav = input_wav
            self._file_transfer = file_transfer
            self._dump = dump
            self._zlb = zlb

        self._output = None

        if self._output_file_name == "-":
            if self._script:
                if sys.stdout is not None and getattr(sys.stdout, "buffer", None) is not None:
                    self._output = sys.stdout.buffer
                else:
                    sys.stdout = io.TextIOWrapper(open(os.devnull, "wb", buffering=0), encoding='utf-8')
                    self._output = sys.stdout.buffer
            else:
                raise ValueError("No output file specified.")

        self._decompressor = zlib.decompressobj() if self._zlb else None
        self._output_file_fw: Optional[FileIO] = None
        self._input_wav_fw = None
        self._dump_wav_fw = None
        self._stream: Optional[sd.RawInputStream] = None
        self._d = None
        self._samplerate = 44100

    def receive_file(self) -> int:
        return self._receive_file_generic()

    def _print_msg(self, msg: str, **kwargs: Any) -> None:
        if self._script:
            print(msg, flush=True, file=sys.stderr, **kwargs)

    def _receive_file_generic(self) -> int:
        done = False
        total = 0
        first = True
        size = -1
        t = float(0)
        crc32: str = ""
        c: bytes

        try:
            if self._output_file_name and self._output_file_name != "-":
                if (Path(self._output_file_name).is_file() and self._overwrite) or (not Path(self._output_file_name).exists()) and (not Path(self._output_file_name).is_dir()):
                    self._output_file_fw = open(self._output_file_name, "b+w", buffering=0)
                    self._output = self._output_file_fw
                elif Path(self._output_file_name).exists():
                    raise IOError(f"Output file {self._output_file_name} exists!")
            if self._output is None:
                raise IOError(f"Output file is stdout but it does not exists!")
            if self._dump:
                self._dump_wav_fw = sf.SoundFile(self._dump, "wb", samplerate=self._samplerate, channels=1, format='WAV', subtype="FLOAT")
            if self._input_wav:
                if Path(self._input_wav).is_file():
                    self._input_wav_fw = sf.SoundFile(self._input_wav, "rb")
                else:
                    raise IOError(f"Input wav file {self._input_wav} not found.")
            else:
                self._stream = sd.RawInputStream(dtype="float32", channels=1, samplerate=float(self._samplerate), blocksize=4096)
                self._stream.start()
            write_buffer_size = 16 * 1024
            write_buffer = self._ffi.new(f"uint8_t[{write_buffer_size}]")
            opt = self._lib.quiet_decoder_profile_filename(self._profile_file.encode(), self._protocol.encode())
            self._d = self._lib.quiet_decoder_create(opt, self._samplerate)
            while not done:
                ttt = time.time()
                if self._input_wav_fw is not None:
                    sound_data = self._input_wav_fw.buffer_read(16 * 1024, 'float32')
                elif self._stream is not None:
                    sound_data, overflowed = self._stream.read(16 * 1024)
                else:
                    raise ValueError(f"\nERROR: Can't read sound data!")
                if self._dump_wav_fw is not None:
                    self._dump_wav_fw.buffer_write(sound_data, 'float32')
                    self._dump_wav_fw.flush()
                read_size = int(len(sound_data) / self._ffi.sizeof("quiet_sample_t"))
                sound_data_ctype = self._ffi.from_buffer("quiet_sample_t *", sound_data)
                self._lib.quiet_decoder_consume(self._d, sound_data_ctype, read_size)
                decoded_size = self._lib.quiet_decoder_recv(self._d, write_buffer, write_buffer_size)
                tttt = time.time() - ttt
                if decoded_size < 0:
                    continue
                elif decoded_size == 0:
                    # continue
                    self._print_msg(f"\nDecoded size is zero.")
                    done = True
                else:
                    if self._lib.quiet_decoder_checksum_fails(self._d):
                        raise ValueError(f"\nERROR: Checksum failed at block {total}")
                    if self._decompressor:
                        c = self._decompressor.decompress(self._ffi.buffer(write_buffer)[0:decoded_size])
                        decoded_size = len(c)
                    else:
                        c = self._ffi.buffer(write_buffer)[0:decoded_size]
                    start = 0
                    if first and self._file_transfer:
                        t = time.time() - tttt
                        first = False
                        packed_size = c[0:4]
                        header_len = struct.unpack("=L", packed_size)[0]
                        json_string = c[4:4 + header_len][:]
                        js = json.loads(json_string)
                        size = js["size"]
                        crc32 = js["crc32"]
                        self._print_msg(f"Size: {size}")
                        self._print_msg(f"CRC32: {crc32}")
                        start = 4 + header_len
                        decoded_size -= start
                    self._output.write(c[start:start + decoded_size])
                    self._output.flush()
                    if self._file_transfer:
                        total += decoded_size
                        self._print_msg(f"Received: {total}  \r", end="")
                        if total == size:
                            done = True
                        elif total > size:
                            raise ValueError("ERROR: received file is too big.")
            self._lib.quiet_decoder_flush(self._d)
            while True:
                decoded_size = self._lib.quiet_decoder_recv(self._d, write_buffer, write_buffer_size)
                if self._lib.quiet_decoder_checksum_fails(self._d):
                    raise ValueError(f"\nERROR: Flushing, checksum failed at block {total}")
                if decoded_size < 0:
                    break
                if self._decompressor:
                    c = self._decompressor.decompress(self._ffi.buffer(write_buffer)[0:decoded_size])
                    decoded_size = len(c)
                else:
                    c = self._ffi.buffer(write_buffer)[0:decoded_size]
                self._output.write(c[0:decoded_size])
                self._output.flush()
            if self._file_transfer and self._output_file_fw is not None:
                tt = time.time() - t
                self._output.seek(0)
                crc32r: int = binascii.crc32(self._output.read())
                fixed_length_hex: str = f'{crc32r:08x}'
                self._print_msg("")
                if crc32 != fixed_length_hex:
                    self._print_msg(f"ERROR: CRC32 mismatch!")
                    raise ValueError(f"ERROR: File checksum failed!")
                else:
                    self._print_msg(f"CRC32 check passed.")
                self._print_msg(f"Time taken to decode waveform: {tt}")
                if tt > 0:
                    self._print_msg(f"Speed: {size / tt} B/s")
        except KeyboardInterrupt as ex:
            if self._script:
                self._print_msg(str(ex))
                return 1
            else:
                raise ex
        except ValueError as ex:
            if self._script:
                self._print_msg(str(ex))
                return 1
            else:
                raise ex
        except IOError as ex:
            if self._script:
                self._print_msg(str(ex))
                return 1
            else:
                raise ex
        except zlib.error as ex:
            if self._script:
                self._print_msg(str(ex))
                return 1
            else:
                raise ex
        except Exception as ex:
            raise ex
        finally:
            if self._output_file_fw is not None:
                self._output_file_fw.close()
            if self._dump_wav_fw is not None:
                self._dump_wav_fw.close()
            if self._d is not None:
                self._lib.quiet_decoder_destroy(self._d)
            if self._stream is not None:
                self._stream.stop()
                self._stream.close()
            if self._input_wav_fw is not None:
                self._input_wav_fw.close()
        return 0
