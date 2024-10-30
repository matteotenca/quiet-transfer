import os
import unittest
import zlib
from unittest import skip

from quiettransfer import CompressFile
from quiettransfer import ReceiveFile


class MyTestCase(unittest.TestCase):
    @skip("Skip compress")
    def test_something(self):
        header_size = 0
        crc = ""
        size = 0
        with open(r"r:\out.bin", "wb") as f:
            c = CompressFile(r'd:\download\ContrattoPrestazione-GIGROUP-2007844507.pdf',
                             compress=True, is_script=True)

            # data = c.read(8)
            # f.write(data)
            while dt := c.read(1000):
                # data = c.read(1000)
                f.write(dt)
            # data += c.read(1000)
            # f.write(data)
            f.write(c.read())
            f.write(c.gflush())
            header_size = c.header_size
            # print(f"Header Size: {header_size}")
            size = c.size
            crc = c.crc
            c.close()

        with open(r"r:\out.bin", "rb") as f:
            s = os.stat(r"r:\out.bin")
            ss = s.st_size
            print(f"Compressed Size: {ss}")
            z = zlib.decompressobj()
            c = z.decompress(f.read())
            print(f"Header Size: {header_size}, Data Size: {len(c[header_size:])}")
            print(f"{zlib.crc32(c[header_size:]):08x}")
        self.assertEqual(crc, f"{zlib.crc32(c[header_size:]):08x}")  # add assertion here

    def test_receive(self):
        r = ReceiveFile(output=r"r:\out.bin", overwrite=True, file_transfer=True, zlb=True)
        r.receive_file()


if __name__ == '__main__':
    unittest.main()
