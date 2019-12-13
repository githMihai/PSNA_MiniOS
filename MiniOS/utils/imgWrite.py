import os, sys, time
from struct import pack

SECTOR_SIZE         = 512
NUMBER_OF_SECTORS   = 20160
IMAGE_SIZE          = 10321920

imagePath   = ".\\bin\\hdd_CHS_20_16_63.img"

out = open(imagePath, "wb")

# out.write(b'\1')
# out.write(b'\0')
# out.write(b'\0')
# out.write(b'\0')
# out.write(b'\2')
# out.write(b'\0')
# out.write(b'\0')
# out.write(b'\0')
# out.write(b'\3')
# out.write(b'\0')
# for i in range(0, 10321910):
#     out.write(b'\0')

for i in range(0, int(SECTOR_SIZE/2)):
    out.write(bytes([i, 0]))

for i in range(1, NUMBER_OF_SECTORS):
    for j in range(0, SECTOR_SIZE):
        out.write(b'\0')


out.close()