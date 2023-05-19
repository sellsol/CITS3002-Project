import struct
import zlib

# Inspired by PyPNG - https://gitlab.com/drj11/pypng/-/blob/main/code/png.py

# The PNG signature
signature = struct.pack('8B', 137, 80, 78, 71, 13, 10, 26, 10)

class Writer:
    def __init__(self, width=None, height=None):
        self.width = width
        self.height = height

    def write(self, outfile, rows):
        outfile.write(signature) #Write signature

        #Write preamble
        write_chunk(outfile, b'IHDR',
                    struct.pack("!2I5B", self.width, self.height, 8, 2, 0, 0, 0))

        compressor = zlib.compressobj()
        data = bytearray()

        for row in rows: #Write every row as a chunk
                data.append(0)
                data.extend(row)
                if len(data) > 2 ** 20:
                    compressed = compressor.compress(data)
                    write_chunk(outfile, b'IDAT', compressed)
                    data = bytearray()

        #Write any leftover data
        compressed = compressor.compress(bytes(data))
        flushed = compressor.flush()

        if len(compressed) or len(flushed):
            write_chunk(outfile, b'IDAT', compressed + flushed)

        write_chunk(outfile, b'IEND')

#Write chunk of png code with tag
def write_chunk(outfile, tag, data=b''):
    outfile.write(struct.pack("!I", len(data)))
    outfile.write(tag)
    outfile.write(data)
    checksum = zlib.crc32(tag)
    checksum = zlib.crc32(data, checksum)
    checksum &= 2 ** 32 - 1
    outfile.write(struct.pack("!I", checksum))

width = 255
height = 255
img = []
for y in range(height):
    row = ()
    for x in range(width):
        row = row + (0, 255, 0)
    img.append(row)
with open('image.png', 'wb') as f:
    w = Writer(width, height)
    w.write(f, img)