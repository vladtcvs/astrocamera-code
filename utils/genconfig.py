import sys

FourCC = "Y16 "
width = 640
height = 480

FourCC = FourCC.encode('ASCII')
block = [255]*256
block[0] = FourCC[0]
block[1] = FourCC[1]
block[2] = FourCC[2]
block[3] = FourCC[3]
block[4] = int(width / 256) % 256
block[5] = width % 256
block[6] = int(height / 256) % 256
block[7] = height % 256

block = bytes(block)

with open(sys.argv[1], "wb") as f:
    f.write(block)
