import argparse
import binascii
import io
import json
import sys
import time
from pathlib import Path
from typing import BinaryIO, Optional
# import soundfile as sf
# import pyaudio
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
    err = None

    try:
        if args.output and args.output != "-":
            if (Path(args.output).is_file() and args.overwrite) or not Path(args.output).exists():
                fw = open(args.output, "wb", buffering=0)
                output = fw
            elif Path(args.output).exists():
                raise IOError
        opt = lib.quiet_decoder_profile_filename(profile_file.encode(), args.protocol.encode())
        # e = lib.quiet_decoder_create(opt, sample_rate)
        write_buffer_size = 16384
        # uint8_t *write_buffer = (uint8_t*)malloc(write_buffer_size*sizeof(uint8_t));
        write_buffer = ffi.new(f"uint8_t[{write_buffer_size + 1}]")
        # with sf.SoundFile('in.wav', 'w', channels=1, samplerate=sample_rate, format='WAV', subtype="FLOAT") as fw:
        # with open(filename, "wb", buffering=0) as fw:
        err = lib.Pa_Initialize()
        if err != lib.paNoError:
            raise IOError
        device = lib.Pa_GetDefaultInputDevice()
        device_info = lib.Pa_GetDeviceInfo(device)
        sample_rate = device_info.defaultSampleRate
        latency = device_info.defaultLowInputLatency
        d = lib.quiet_portaudio_decoder_create(opt, device, latency, sample_rate)
        # lib.quiet_portaudio_decoder_set_blocking(d, 0, 0)
        while not done:
            # lib.quiet_portaudio_decoder_consume(d)
            read_size = lib.quiet_portaudio_decoder_recv(d, write_buffer, write_buffer_size)
            if read_size <= 0:
                continue
            if first and args.file_transfer:
                # sys.stderr.buffer.write(ffi.buffer(write_buffer[0:read_size]))
                # sys.stderr.buffer.flush()
                first = False
                js = json.loads(ffi.buffer(write_buffer[0:read_size])[:])
                size = js["size"]
                crc32 = js["crc32"]
                print("Size:", size, file=sys.stderr, flush=True)
                print("CRC32:", crc32, file=sys.stderr, flush=True)
                t = time.time()
                buf = io.BytesIO()
            else:
                total += read_size
                print("Received:", total, "\r", end="", flush=True, file=sys.stderr)
                output.write(ffi.buffer(write_buffer[0:read_size]))
                output.flush()
                if buf is not None:
                    buf.write(ffi.buffer(write_buffer[0:read_size]))
                if total >= size:
                    tt = time.time() - t
                    crc32r: int = binascii.crc32(buf.getbuffer())
                    fixed_length_hex: str = f'{crc32r:08x}'
                    print()
                    if crc32 != fixed_length_hex:
                        print("ERROR: CRC32 mismatch!", flush=True, file=sys.stderr)
                    else:
                        print("CRC32 check passed.", flush=True, file=sys.stderr)
                    print("Time taken to encode waveform:", tt, flush=True, file=sys.stderr)
                    print("Speed:", size / tt, "B/s", flush=True, file=sys.stderr)
                    done = True
    except KeyboardInterrupt:
        done = True
    except Exception as ex:
        print(ex)
        sys.exit(1)
    finally:
        if fw is not None:
            fw.close()
        if d is not None:
            lib.quiet_portaudio_decoder_destroy(d)
        if err is not None:
            lib.Pa_Terminate()

