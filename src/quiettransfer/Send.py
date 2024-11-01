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
import io
import os
import sys
from io import FileIO, BufferedReader
# noinspection PyPackageRequirements
import sounddevice as sd # type: ignore
# noinspection PyPackageRequirements
import soundfile as sf # type: ignore
import time
from pathlib import Path
from typing import Optional, Any, BinaryIO, Union
import quiettransfer
from quiettransfer import CompressFile


class SendFile:

    def __init__(self, args: Optional[argparse.Namespace] = None,
                 input_file: str = "-", output_wav: Optional[str] = None,
                 protocol: str = "audible", file_transfer: bool = False, zlb: bool = False) -> None:

        self._lib = quiettransfer.lib
        self._ffi = quiettransfer.ffi
        self._profile_file = quiettransfer.profile_file
        self._fw: Optional[sf.SoundFile] = None
        self._stream: Optional[sd.RawOutputStream] = None
        self._script = True if args is not None else False
        self._fi: Union[FileIO, BufferedReader, None] = None
        self._input_data: Union[quiettransfer.CompressFile, BinaryIO, None] = None
        self._e = None
        self._trailing_silence = 1
        self._initial_silence = 1
        self._buf: Optional[quiettransfer.CompressFile] = None

        if args is not None:
            # called from command line
            self._input_file = args.input
            self._output_wav = args.output_wav
            self._protocol = args.protocol
            self._file_transfer = args.file_transfer
            self._zlb = args.zlib
        else:
            # called from module
            self._input_file = input_file
            self._output_wav = output_wav
            self._protocol = protocol
            self._file_transfer = file_transfer
            self._zlb = zlb

        self._samplerate = 44100

        if self._input_file == "-":
            if self._script:
                if sys.stdin is not None and getattr(sys.stdin, "buffer", None) is not None:
                    self._input_data = sys.stdin.buffer
                else:
                    sys.stdin = io.TextIOWrapper(open(os.devnull, "wb", buffering=0), encoding='utf-8')
                    self._input_data = sys.stdin.buffer
            else:
                raise ValueError("ERROR: No input file specified.")

    def send_file(self) -> int:
        return self._send_file()

    def _print_msg(self, msg: str, **kwargs: Any) -> None:
        if self._script:
            print(msg, flush=True, file=sys.stderr, **kwargs)

    def _write_data(self, data_buf: bytes) -> None:
        if isinstance(self._fw, sf.SoundFile):
            self._fw.buffer_write(data_buf, 'float32')
        elif isinstance(self._stream, sd.RawOutputStream):
            self._stream.write(data_buf)
        else:
            raise IOError("ERROR: Unknown output stream.")

    def _send_file(self) -> int:
        total = 0
        quiet_sample_t_size = self._ffi.sizeof("quiet_sample_t")
        try:
            opt = self._lib.quiet_encoder_profile_filename(self._profile_file.encode(), self._protocol.encode())
            self._e = self._lib.quiet_encoder_create(opt, self._samplerate)
            done = False
            block_len = 16 * 1024
            samplebuf_len = 16 * 1024
            samplebuf = self._ffi.new(f"quiet_sample_t[{samplebuf_len}]")
            if self._output_wav is not None:
                self._lib.quiet_encoder_clamp_frame_len(self._e, samplebuf_len)
                self._fw = sf.SoundFile(self._output_wav, 'w', channels=1, samplerate=self._samplerate,
                                        format='WAV', subtype="FLOAT")
            else:
                self._stream = sd.RawOutputStream(dtype="float32", channels=1, samplerate=float(self._samplerate), blocksize=4096)
                self._stream.start()
            if self._input_file and self._input_file != "-":
                p = Path(self._input_file)
                if p.is_file():
                    if self._file_transfer:
                        self._buf = quiettransfer.CompressFile(self._input_file, compress=self._zlb, is_script=self._script)
                        total -= self._buf.header_size
                        self._input_data = self._buf
                    else:
                        self._fi = open(self._input_file, "rb")
                        self._input_data = self._fi
                else:
                    raise IOError(f"ERROR: File {self._input_file} not found.")
            elif self._input_data is None:
                raise IOError(f"ERROR: Input file is stdin but it does not exists!")
            elif self._file_transfer:
                raise ValueError("ERROR: File transfer mode requires an input file.")

            self._write_data(b'0' * quiet_sample_t_size * self._samplerate * self._initial_silence)
            t = time.time()
            while not done:
                nread = self._input_data.read(block_len)
                if nread is None or len(nread) == 0:
                    break
                elif len(nread) < block_len:
                    done = True
                frame_len = self._lib.quiet_encoder_get_frame_len(self._e)
                for i in range(0, len(nread), frame_len):
                    frame_len = len(nread) - i if frame_len > (len(nread) - i) else frame_len
                    if self._lib.quiet_encoder_send(self._e, nread[i:i+frame_len], frame_len) < 0:
                        raise ValueError()
                if self._file_transfer:
                    total += len(nread)
                    self._print_msg(f"Sent: {total}    \r", end="")
                written = samplebuf_len
                while written == samplebuf_len:
                    written = self._lib.quiet_encoder_emit(self._e, samplebuf, samplebuf_len)
                    if written > 0:
                        self._write_data(self._ffi.buffer(samplebuf))
            tt = time.time() - t
            self._write_data(b'0' * quiet_sample_t_size * self._samplerate * self._trailing_silence)
            if isinstance(self._buf, CompressFile) and self._buf.size > 0:
                self._print_msg(f"\nTime taken to encode waveform: {tt}")
                self._print_msg(f"Speed: {(self._buf.size + self._buf.header_size) / tt} B/s")
        except KeyboardInterrupt:
            return 1
        except IOError as ex:
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
        except Exception as ex:
            if self._script:
                self._print_msg(str(ex))
                return 1
            else:
                raise ex
        finally:
            if self._buf is not None:
                self._buf.close()
            if self._fw is not None:
                self._fw.close()
            if self._stream is not None:
                self._stream.stop()
                self._stream.close()
            if self._fi is not None:
                self._fi.close()
            if self._e is not None:
                self._lib.quiet_encoder_destroy(self._e)
        return 0
