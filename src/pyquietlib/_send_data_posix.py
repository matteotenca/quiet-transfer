import argparse
import binascii
import io
import json
import sys
from pathlib import Path
import time
from typing import Optional

import soundfile as sf
import pyaudio
from pyquietlib import lib, ffi, profile_file


def _send_file(args: argparse.Namespace):
    sample_rate = 44100
    e = None
    p = None
    stream = None
    fw = None
    fi = None
    first = False
    size = -1
    buf: Optional[io.BytesIO]

    try:
        opt = lib.quiet_encoder_profile_filename(profile_file.encode(), args.protocol.encode())
        e = lib.quiet_encoder_create(opt, sample_rate)
        done = False
        block_len = 4 * 1024
        samplebuf_len = 4 * 1024
        samplebuf = ffi.new(f"quiet_sample_t[{samplebuf_len}]")
        if args.dump is not None:
            fw = sf.SoundFile(args.dump, 'w', channels=1, samplerate=sample_rate, format='WAV', subtype="FLOAT")
        if args.input != "-":
            if Path(args.input).is_file():
                if args.file_transfer:
                    s = Path(args.input).stat()
                    size = s.st_size
                    buf = io.BytesIO()
                    with open(args.input, "rb", buffering=0) as fi:
                        buf.write(fi.read())
                    buf.seek(0)
                    crc32: int = binascii.crc32(buf.getbuffer())
                    fixed_length_hex: str = f'{crc32:08x}'
                    buf.seek(0)
                    print("Size:", size, flush=True, file=sys.stderr)
                    print("CRC32:", fixed_length_hex, flush=True, file=sys.stderr)
                    header = {"size": size, "crc32": fixed_length_hex}
                    first = True
                    input_data = buf
                else:
                    fi = open(args.input, "rb", buffering=0)
                    input_data = fi
            else:
                raise ValueError
        else:
            input_data = sys.stdin.buffer
        p = pyaudio.PyAudio()
        stream = p.open(format=pyaudio.paFloat32, channels=1, rate=sample_rate, output=True,
                        frames_per_buffer=4096)
        total = 0
        t = time.time()
        while not done:
            if first:
                nread = json.dumps(header).encode("utf-8")
                first = False
                total -= len(nread)
            else:
                nread = input_data.read(block_len)
                if nread is None:
                    break
                elif len(nread) < block_len:
                    done = True
            frame_len = lib.quiet_encoder_get_frame_len(e)
            for i in range(0, len(nread), frame_len):
                frame_len = len(nread) - i if frame_len > (len(nread) - i) else frame_len
                if lib.quiet_encoder_send(e, nread[i:i+frame_len], frame_len) < 0:
                    raise ValueError()
            if args.file_transfer:
                total += len(nread)
                print("Sent:", total, "    \r", end="", flush=True, file=sys.stderr)
            written = samplebuf_len
            while written == samplebuf_len:
                written = lib.quiet_encoder_emit(e, samplebuf, samplebuf_len)
                if written > 0:
                    stream.write(ffi.buffer(samplebuf))
                    if args.dump is not None and fw:
                        fw.buffer_write(ffi.buffer(samplebuf), 'float32')

        if size > 0:
            tt = time.time() - t
            print()
            print("Time taken to encode waveform:", tt, flush=True, file=sys.stderr)
            print("Speed:", size / tt, "B/s", flush=True, file=sys.stderr)
    except KeyboardInterrupt:
        done = True
    except Exception as ex:
        print(ex)
        raise ex
        # sys.exit(1)
    finally:
        if fw is not None:
            fw.close()
        if fi is not None:
            fi.close()
        if stream is not None:
            stream.stop_stream()
            stream.close()
        if p is not None:
            p.terminate()
        if e is not None:
            lib.quiet_encoder_destroy(e)
