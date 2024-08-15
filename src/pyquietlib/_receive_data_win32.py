import argparse
import binascii
import io
import json
import sys
import time
from pathlib import Path
from typing import BinaryIO, Optional

import pyaudio
import soundfile as sf
from pyquietlib import lib, ffi, profile_file


def _receive_file(args: argparse.Namespace):
    d = None
    done = False
    output: BinaryIO = sys.stdout.buffer
    fw: Optional[BinaryIO] = None
    buf: Optional[io.BytesIO] = None
    total = 0
    first = True
    size = -1
    t = 0
    crc32: str = ""
    sample_rate = 44100
    p: Optional[pyaudio.PyAudio] = None
    stream = None
    fwav: Optional[sf.SoundFile] = None
    inwav: Optional[sf.SoundFile] = None

    try:
        if args.output and args.output != "-":
            if (Path(args.output).is_file() and args.overwrite) or not Path(args.output).exists():
                fw = open(args.output, "wb", buffering=0)
                output = fw
            elif Path(args.output).exists():
                raise IOError
        if args.dump:
            fwav = sf.SoundFile(args.dump, "wb", samplerate=sample_rate, channels=1, format='WAV', subtype="FLOAT")
        if args.input_wav:
            if Path(args.input_wav).is_file():
                inwav = sf.SoundFile(args.input_wav, "rb")
            else:
                raise IOError(f"Input wav file {args.input_wav} not found.")
        else:
            p = pyaudio.PyAudio()
            stream = p.open(format=pyaudio.paFloat32, channels=1, rate=sample_rate, input=True,
                            frames_per_buffer=4096)
        write_buffer_size = 16 * 1024
        write_buffer = ffi.new(f"uint8_t[{write_buffer_size}]")
        # with sf.SoundFile('in.wav', 'w', channels=1, samplerate=sample_rate, format='WAV', subtype="FLOAT") as fw:
        opt = lib.quiet_decoder_profile_filename(profile_file.encode(), args.protocol.encode())
        d = lib.quiet_decoder_create(opt, sample_rate)
        while not done:
            if inwav is not None:
                sound_data = inwav.buffer_read(16 * 1024, 'float32')
            else:
                sound_data = stream.read(16 * 1024)
            if fwav is not None:
                fwav.buffer_write(sound_data, 'float32')
                fwav.flush()
            read_size = int(len(sound_data) / ffi.sizeof("quiet_sample_t"))
            sound_data_ctype = ffi.from_buffer("quiet_sample_t *", sound_data)
            lib.quiet_decoder_consume(d, sound_data_ctype, read_size)
            decoded_size = lib.quiet_decoder_recv(d, write_buffer, write_buffer_size)
            if decoded_size < 0:
                continue
            elif decoded_size == 0:
                # continue
                print("decoded size is zero", file=sys.stderr, flush=True)
                done = True
            else:
                if lib.quiet_decoder_checksum_fails(d):
                    raise ValueError(f"ERROR: Checksum failed at block {total}")
                if first and args.file_transfer:
                    first = False
                    json_string = ffi.buffer(write_buffer)[0:decoded_size][:]
                    js = json.loads(json_string)
                    size = js["size"]
                    crc32 = js["crc32"]
                    print("Size:", size, file=sys.stderr, flush=True)
                    print("CRC32:", crc32, file=sys.stderr, flush=True)
                    t = time.time()
                    buf = io.BytesIO()
                else:
                    output.write(ffi.buffer(write_buffer)[0:decoded_size])
                    output.flush()
                    if args.file_transfer:
                        total += decoded_size
                        print("Received:", total, "\r", end="", flush=True, file=sys.stderr)
                        buf.write(ffi.buffer(write_buffer)[0:decoded_size])
                        buf.flush()
                        if total == size:
                            done = True
                        elif total > size:
                            raise ValueError("ERROR: too big")
        lib.quiet_decoder_flush(d)
        while True:
            decoded_size = lib.quiet_decoder_recv(d, write_buffer, write_buffer_size)
            if lib.quiet_decoder_checksum_fails(d):
                raise ValueError(f"ERROR: Checksum failed at block {total}")
            if decoded_size < 0:
                break
            output.write(ffi.buffer(write_buffer)[0:decoded_size])
            output.flush()
            if buf is not None:
                buf.write(ffi.buffer(write_buffer)[0:decoded_size])
                buf.flush()
        if args.file_transfer:
            tt = time.time() - t
            crc32r: int = binascii.crc32(buf.getbuffer())
            fixed_length_hex: str = f'{crc32r:08x}'
            print(flush=True, file=sys.stderr)
            if crc32 != fixed_length_hex:
                print("ERROR: CRC32 mismatch!", flush=True, file=sys.stderr)
            else:
                print("CRC32 check passed.", flush=True, file=sys.stderr)
            print("Time taken to decode waveform:", tt, flush=True, file=sys.stderr)
            print("Speed:", size / tt, "B/s", flush=True, file=sys.stderr)
    except KeyboardInterrupt:
        done = True
    except ValueError as ex:
        print(ex)
        sys.exit(1)
    except IOError as ex:
        print(ex)
        sys.exit(1)
    except Exception as ex:
        print(ex)
        raise ex
    finally:
        if fw is not None:
            fw.close()
        if fwav is not None:
            fwav.close()
        if d is not None:
            lib.quiet_decoder_destroy(d)
        if stream is not None:
            stream.stop_stream()
            stream.close()
        if p is not None:
            p.terminate()
