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
import sys
from io import FileIO

# noinspection PyPackageRequirements
import pyaudio
# noinspection PyPackageRequirements
import soundfile as sf # type: ignore
import time
from pathlib import Path
from typing import Optional, Any, Dict
import quiettransfer


class SendFile:

    def __init__(self, args: Optional[argparse.Namespace] = None,
                 input_file: Optional[str] = None, output_wav: Optional[str] = None,
                 protocol: str = "audible", file_transfer: bool = False) -> None:

        self._lib = quiettransfer.lib
        self._ffi = quiettransfer.ffi
        self._profile_file = quiettransfer.profile_file
        self._fw: Optional[sf.SoundFile] = None
        self._stream: Optional[pyaudio.Stream] = None
        self._win32 = True if sys.platform == "win32" else False
        self._script = True if args is not None else False
        self._p: Optional[pyaudio.PyAudio] = None
        self._fi: Optional[FileIO] = None
        self._input_data: Any
        self._e = None

        if args is not None:
            # called from command line
            self._input_file = args.input
            self._output_wav = args.output_wav
            self._protocol = args.protocol
            self._file_transfer = args.file_transfer
        else:
            # called from module
            self._input_file = input_file
            self._output_wav = output_wav
            self._protocol = protocol
            self._file_transfer = file_transfer

        self._samplerate = 44100

    def send_file(self) -> int:
        return self._send_file()

    def _print_msg(self, msg: str, **kwargs: Any) -> None:
        if self._script:
            print(msg, flush=True, file=sys.stderr, **kwargs)

    def _write_data(self, data_buf: bytes) -> None:
        if isinstance(self._fw, sf.SoundFile):
            self._fw.buffer_write(data_buf, 'float32')
        elif isinstance(self._stream, pyaudio.PyAudio.Stream):
            self._stream.write(data_buf)

    def _send_file(self) -> int:
        header: Optional[Dict[str, Any]] = None
        first = False
        size = -1
        buf: Optional[io.BytesIO]
        initial_silence = 1
        trailing_silence = 1
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
                self._p = pyaudio.PyAudio()
                self._stream = self._p.open(format=pyaudio.paFloat32, channels=1, rate=self._samplerate, output=True,
                                            frames_per_buffer=4096)
            self._write_data(b'0' * quiet_sample_t_size * self._samplerate * initial_silence)

            if self._input_file != "-":
                if Path(self._input_file).is_file():
                    if self._file_transfer:
                        s = Path(self._input_file).stat()
                        size = s.st_size
                        buf = io.BytesIO()
                        with open(self._input_file, "rb", buffering=0) as tfi:
                            buf.write(tfi.read())
                        buf.seek(0)
                        crc32: int = binascii.crc32(buf.getbuffer())
                        fixed_length_hex: str = f'{crc32:08x}'
                        buf.seek(0)
                        self._print_msg(f"Size: {size}")
                        self._print_msg(f"CRC32: {fixed_length_hex}")
                        header = {"size": size, "crc32": fixed_length_hex}
                        first = True
                        self._input_data = buf
                    else:
                        self._fi = open(self._input_file, "rb", buffering=0)
                        self._input_data = self._fi
                else:
                    raise IOError(f"File {self._input_file} not found.")
            else:
                self._input_data = sys.stdin.buffer
            total = 0
            t = time.time()
            while not done:
                if first and isinstance(header, dict):
                    nread = json.dumps(header).encode("utf-8")
                    first = False
                    total -= len(nread)
                else:
                    nread = self._input_data.read(block_len)
                    if nread is None:
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
            self._write_data(b'0' * quiet_sample_t_size * self._samplerate * trailing_silence)
            if size > 0:
                tt = time.time() - t - trailing_silence
                self._print_msg(f"\nTime taken to encode waveform: {tt}")
                self._print_msg(f"Speed: {size / tt} B/s")
        except KeyboardInterrupt:
            return 1
        except IOError as ex:
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
            if self._fw is not None:
                self._fw.close()
            if self._stream is not None:
                self._stream.stop_stream()
                self._stream.close()
            if self._p is not None:
                self._p.terminate()
            if self._fi is not None:
                self._fi.close()
            if self._e is not None:
                self._lib.quiet_encoder_destroy(self._e)
        return 0
