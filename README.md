# quiet-transfer

---
![PyPI - Status](https://img.shields.io/pypi/status/quiet-transfer)
![PyPI - License](https://img.shields.io/pypi/l/quiet-transfer?color=blue)
![PyPI - Python Version](https://img.shields.io/pypi/pyversions/quiet-transfer)

## Tool to send/receive text/binary file over audio via many modulation schemes

### Installation

`
pip install quiet-transfer
`

#### Dependencies

`cffi>=1.12.0`, `pyaudio`, `soundfile`

## Documentation

`quiet-transfer` can be used as a command line command. It's purpose is to convert data to sound and send it 
to a sound card or to a WAV file, and all the way back.

### Usage
```bash
usage: quiet-transfer [-h] {send,receive} ...

Command line utility to send/receive files/strings via quiet library.

options:
  -h, --help      show this help message and exit

commands:
  {send,receive}  send or receive data.
    send          modulate data into audio signal.
    receive       demodulate data from audio signal.
```

### Send

```bash
usage: quiet-transfer send [-h] [-i <inputfile>] [-o <wavoutputfile>] [-p <protocol>] [-f]

Command line utility to send/receive files/strings via quiet library.

options:
  -h, --help            show this help message and exit
  -i <inputfile>, --input <inputfile>
                        input file (use '-' for stdin).
  -o <wavoutputfile>, --output-wav <wavoutputfile>
                        write audio to this wav file.
  -p <protocol>, --protocol <protocol>
                        protocol
  -f, --file-transfer   enable file transfer mode.
```

 - `<inputfile>` can be the name of a file to read data from or `-` (default) if you want to read data from `stdin`.
 - `<wavoutputfile>` is the optional name of a WAV file to write audio data to. If not present, the audio data will be 
written to the current default output audio device
 - `<protocol>` can be one of:
   - audible
   - audible-7k-channel-0
   - audible-7k-channel-1
   - cable-64k
   - ultrasonic
   - ultrasonic-3600
   - ultrasonic-whisper
 - the `--file-transfer` flag enables the following behaviour:
   1) If `<inputfile>` is missing or is `-`, the flag is ignored.
   2) Otherwise, the `<inputfile>` is read in memory all at once and its CRC32 is calculated.
   2) The file size and the CRC32 are put in a JSON header which is encoded in audio and sent to audio/written to the WAV file.
   3) Some information messages are written to `stderr`
  
 ### Receive

```bash
usage: quiet-transfer receive [-h] [-o <outputfile>] [-w] [-d <dumpfile>] [-p <protocol>] [-i <wavinputfile>] [-f]

Command line utility to send/receive files/strings via quiet library.

options:
  -h, --help            show this help message and exit
  -o <outputfile>, --output <outputfile>
                        output file (use '-' for stdout).
  -w, --overwrite       overwrite output file if it exists.
  -d <dumpfile>, --dump <dumpfile>
                        dump received audio to this wav file.
  -p <protocol>, --protocol <protocol>
                        protocol
  -i <wavinputfile>, --input-wav <wavinputfile>
                        WAV file to read from.
  -f, --file-transfer   enable file transfer mode.
```
- `<outputfile>` can be the name of a file to write data to or `-` (default) if you want to write data to `stdout`.
- `--overwrite` must be specified if `<outputfile>` already exists.
- `<protocol>` can be one of:
   - audible
   - audible-7k-channel-0
   - audible-7k-channel-1
   - cable-64k
   - ultrasonic
   - ultrasonic-3600
   - ultrasonic-whisper
- `<dumpfile>` is the optional name of a WAV file. All the audio data read from the soundcard 
or from a input WAV file is written to this file.
- `<wavinputfile>` is the optional name of a WAV file to analyze. Usually this file is generated using the `--output-wav` option.
- `--file-transfer` enables parsing the JSON header generated while sending, used to compare the remote file's size and CRC32 with the received file's ones.

### See also

fm-transfer: (https://github.com/matteotenca/fm-transfer)  
gg-transfer: (https://github.com/matteotenca/gg-transfer)
