import argparse

from pyquietlib.Receive import ReceiveFile
from pyquietlib.Send import SendFile

def _main() -> int:
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
        send_obj = SendFile(args=args)
        return send_obj.send_file()
    elif args.command == "receive":
        receive_obj = ReceiveFile(args=args)
        return receive_obj.receive_file()


if __name__ == '__main__':
    _main()
