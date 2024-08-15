import argparse
import sys

try:
    if sys.platform == 'win32':
        from pyquietlib._send_data_win32 import _send_file
        from pyquietlib._receive_data_win32 import _receive_file
    else:
        from pyquietlib._send_data_posix import _send_file
        from pyquietlib._receive_data_posix import _receive_file
except ImportError as im:
    print(f'Error importing from pyquielib binary: '
          f'path {im.path}, name {im.name}, imported from {im.name_from}',
          file=sys.stderr, flush=True)
    sys.exit(1)


def _main():
    parser = argparse.ArgumentParser(prog="quiet-transfer",
                                     description="Command line utility to send/receive "
                                                 "files/strings via quiet library.")
    subparsers = parser.add_subparsers(required=True, title="commands",
                                       help="send or receive data.")

    # noinspection PyTypeChecker
    sender = subparsers.add_parser(
        "send", help="modulate data into audio signal.",
        description="Command line utility to send/receive files/strings via quiet library.")
    sender.add_argument(
        "-i", "--input", help="input file (use '-' for stdin).",
        metavar="<inputfile>", default="-")
    sender.add_argument(
        "-o", "--output-wav", help="write audio to this wav file.",
        metavar="<wavoutputfile>", default=None)
    sender.add_argument(
        "-p", "--protocol", help="protocol", metavar="<protocol>",
        default="audible",)
    sender.add_argument(
        "-f", "--file-transfer", help="enable file transfer mode.",
        action="store_true", default=False)
    sender.set_defaults(command="send")

    # noinspection PyTypeChecker
    receiver = subparsers.add_parser(
        "receive", help="demodulate data from audio signal.",
        description="Command line utility to send/receive files/strings via quiet library.")
    receiver.add_argument(
        "-o", "--output", help="output file (use '-' for stdout).",
        metavar="<outputfile>", default="-")
    receiver.add_argument(
        "-w", "--overwrite", help="overwrite output file if it exists.",
        action="store_true", default=False)
    receiver.add_argument(
        "-d", "--dump", help="dump received audio to this wav file.",
        metavar="<dumpfile>", default=None)
    receiver.add_argument(
        "-p", "--protocol", help="protocol", metavar="<protocol>",
        default="audible",)
    receiver.add_argument(
        "-i", "--input-wav", help="WAV file to read from.",
        metavar="<wavinputfile>", default=None)
    receiver.add_argument(
        "-f", "--file-transfer", help="enable file transfer mode.",
        action="store_true", default=False)
    receiver.set_defaults(command="receive")

    args: argparse.Namespace = parser.parse_args()

    if args.command == "send":
        _send_file(args)
    elif args.command == "receive":
        _receive_file(args)


if __name__ == '__main__':
    _main()
