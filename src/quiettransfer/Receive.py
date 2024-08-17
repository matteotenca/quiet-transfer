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
import json
import sys
import time
from pathlib import Path
from typing import Optional, Any, BinaryIO

# noinspection PyPackageRequirements
import pyaudio
# noinspection PyPackageRequirements
import soundfile as sf

import quiettransfer


class ReceiveFile:

    def __init__(self, args: Optional[argparse.Namespace] = None,
                 output: Optional[str] = None, overwrite: bool = False, dump: Optional[str] = None,
                 protocol: str = "audible", input_wav: Optional[str] = None,
                 file_transfer: bool = False) -> None:

        self._lib = quiettransfer.lib
        self._ffi = quiettransfer.ffi
        self._profile_file = quiettransfer.profile_file

        self._win32 = True if sys.platform == "win32" else False
        self._script = True if args is not None else False

        if args is not None:
            # called from command line
            self._output = args.output
            self._overwrite = args.overwrite
            self._protocol = args.protocol
            self._input_wav = args.input_wav
            self._file_transfer = args.file_transfer
            self._dump = args.dump
        else:
            # called from module
            self._output = output
            self._overwrite = overwrite
            self._protocol = protocol
            self._input_wav = input_wav
            self._file_transfer = file_transfer
            self._dump = dump

        self._samplerate = 44100

    def receive_file(self) -> int:
        if self._win32:
            return self._receive_file_win32()
        else:
            return self._receive_file_posix()

    def _print_msg(self, msg: str, **kwargs: Any) -> None:
        if self._script:
            print(msg, flush=True, file=sys.stderr, **kwargs)

    def _receive_file_posix(self) -> int:
        return self._receive_file_win32()

    def _receive_file_win32(self) -> int:

        d = None
        done = False
        output: BinaryIO = sys.stdout.buffer
        output_fw: Optional[BinaryIO] = None
        # buf: Optional[io.BytesIO] = None
        total = 0
        first = True
        size = -1
        t = 0
        crc32: str = ""
        sample_rate = 44100
        p: Optional[pyaudio.PyAudio] = None
        stream = None
        dump_wav: Optional[sf.SoundFile] = None
        input_wav: Optional[sf.SoundFile] = None

        try:
            if self._output and self._output != "-":
                if (Path(self._output).is_file() and self._overwrite) or not Path(self._output).exists() and not Path(self._output).is_dir():
                    output_fw = open(self._output, "b+w", buffering=0)
                    output = output_fw
                elif Path(self._output).exists():
                    raise IOError(f"Output file {self._output} exists!")
            if self._dump:
                dump_wav = sf.SoundFile(self._dump, "wb", samplerate=sample_rate, channels=1, format='WAV', subtype="FLOAT")
            if self._input_wav:
                if Path(self._input_wav).is_file():
                    input_wav = sf.SoundFile(self._input_wav, "rb")
                else:
                    raise IOError(f"Input wav file {self._input_wav} not found.")
            else:
                p = pyaudio.PyAudio()
                stream = p.open(format=pyaudio.paFloat32, channels=1, rate=sample_rate, input=True,
                                frames_per_buffer=4096)
            write_buffer_size = 16 * 1024
            write_buffer = self._ffi.new(f"uint8_t[{write_buffer_size}]")
            opt = self._lib.quiet_decoder_profile_filename(self._profile_file.encode(), self._protocol.encode())
            d = self._lib.quiet_decoder_create(opt, sample_rate)
            while not done:
                if input_wav is not None:
                    sound_data = input_wav.buffer_read(16 * 1024, 'float32')
                else:
                    sound_data = stream.read(16 * 1024)
                if dump_wav is not None:
                    dump_wav.buffer_write(sound_data, 'float32')
                    dump_wav.flush()
                read_size = int(len(sound_data) / self._ffi.sizeof("quiet_sample_t"))
                sound_data_ctype = self._ffi.from_buffer("quiet_sample_t *", sound_data)
                self._lib.quiet_decoder_consume(d, sound_data_ctype, read_size)
                decoded_size = self._lib.quiet_decoder_recv(d, write_buffer, write_buffer_size)
                if decoded_size < 0:
                    continue
                elif decoded_size == 0:
                    # continue
                    self._print_msg(f"\nDecoded size is zero.")
                    done = True
                else:
                    if self._lib.quiet_decoder_checksum_fails(d):
                        raise ValueError(f"\nERROR: Checksum failed at block {total}")
                    if first and self._file_transfer:
                        first = False
                        json_string = self._ffi.buffer(write_buffer)[0:decoded_size][:]
                        js = json.loads(json_string)
                        size = js["size"]
                        crc32 = js["crc32"]
                        self._print_msg(f"Size: {size}")
                        self._print_msg(f"CRC32: {crc32}")
                        t = time.time()
                        # buf = io.BytesIO()
                    else:
                        output.write(self._ffi.buffer(write_buffer)[0:decoded_size])
                        output.flush()
                        if self._file_transfer:
                            total += decoded_size
                            self._print_msg(f"Received: {total}  \r", end="")
                            # buf.write(self._ffi.buffer(write_buffer)[0:decoded_size])
                            # buf.flush()
                            if total == size:
                                done = True
                            elif total > size:
                                raise ValueError("ERROR: too big.")
            self._lib.quiet_decoder_flush(d)
            while True:
                decoded_size = self._lib.quiet_decoder_recv(d, write_buffer, write_buffer_size)
                if self._lib.quiet_decoder_checksum_fails(d):
                    raise ValueError(f"\nERROR: Flushing, checksum failed at block {total}")
                if decoded_size < 0:
                    break
                output.write(self._ffi.buffer(write_buffer)[0:decoded_size])
                output.flush()
                # if buf is not None:
                #     buf.write(self._ffi.buffer(write_buffer)[0:decoded_size])
                #     buf.flush()
            if self._file_transfer and output_fw is not None:
                tt = time.time() - t
                output.seek(0)
                # crc32r: int = binascii.crc32(buf.getbuffer())
                crc32r: int = binascii.crc32(output.read())
                fixed_length_hex: str = f'{crc32r:08x}'
                self._print_msg("")
                if crc32 != fixed_length_hex:
                    self._print_msg(f"ERROR: CRC32 mismatch!")
                    raise ValueError(f"ERROR: File checksum failed!")
                else:
                    self._print_msg(f"CRC32 check passed.")
                self._print_msg(f"Time taken to decode waveform: {tt}")
                self._print_msg(f"Speed: {size / tt} B/s")
        except KeyboardInterrupt as ex:
            self._print_msg(str(ex))
            if self._script:
                return 1
            else:
                raise ex
        except ValueError as ex:
            self._print_msg(str(ex))
            if self._script:
                return 1
            else:
                raise ex
        except IOError as ex:
            self._print_msg(str(ex))
            if self._script:
                return 1
            else:
                raise ex
        except Exception as ex:
            raise ex
        finally:
            if output_fw is not None:
                output_fw.close()
            if dump_wav is not None:
                dump_wav.close()
            if d is not None:
                self._lib.quiet_decoder_destroy(d)
            if stream is not None:
                stream.stop_stream()
                stream.close()
            if p is not None:
                p.terminate()
        return 0