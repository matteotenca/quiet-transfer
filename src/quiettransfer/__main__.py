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

import quiettransfer
from quiettransfer.Receive import ReceiveFile
from quiettransfer.Send import SendFile


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
        "-p", "--protocol", help="protocol", metavar="<protocol>", choices=quiettransfer.protocols,
        default="audible")
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
        "-p", "--protocol", help="protocol", metavar="<protocol>", choices=quiettransfer.protocols,
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
    else:
        return 1


if __name__ == '__main__':
    _main()
