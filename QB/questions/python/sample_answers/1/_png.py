import struct
import zlib

# The PNG signature
signature = struct.pack('8B', 137, 80, 78, 71, 13, 10, 26, 10)

class Writer:
    def __init__(self, width=None, height=None):
        self.width = width
        self.height = height

    def write(self, outfile, rows):
        outfile.write(signature)

        write_chunk(outfile, b'IHDR',
                    struct.pack("!2I5B", self.width, self.height, 8, 2, 0, 0, 0))

        compressor = zlib.compressobj()
        data = bytearray()

        for row in rows:
            data.append(0)
            data.extend(row)
            compressed = compressor.compress(data)
            write_chunk(outfile, b'IDAT', compressed)
            data = bytearray()

        compressed = compressor.compress(bytes(data))
        flushed = compressor.flush()

        if len(compressed) or len(flushed):
            write_chunk(outfile, b'IDAT', compressed + flushed)

        write_chunk(outfile, b'IEND')

def write_chunk(outfile, tag, data=b''):
    outfile.write(struct.pack("!I", len(data)))
    outfile.write(tag)
    outfile.write(data)
    checksum = zlib.crc32(tag)
    checksum = zlib.crc32(data, checksum)
    checksum &= 2 ** 32 - 1
    outfile.write(struct.pack("!I", checksum))

