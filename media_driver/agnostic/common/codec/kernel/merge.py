import os,shutil
import sys
import argparse

import struct
import os
from sys import platform

# Usage:
#   merge.py -o out_file in_file [in_file...]
#
# Description:
#   This script merges N kernels given in arguments (in_file...) to the common kernel (out_file) according
# to the following output format:
#
# | DW0: kernel_count | DW1: offset_of_kernel_1 | DW1: offset_of_kernel_2 | ... | DW15: offset_of_kernel_15 |
# |                                     kernel_1 (size aligned to 64 bytes)                                 |
# |                                     kernel_2 (size aligned to 64 bytes)                                 |
# |                                                 ...                                                     |
# |                                     kernel_15 (size aligned to 64 bytes)                                |
# |                                            padding 128 bytes                                            |
#
# Limitations:
#   - No more than 15 kernels are being merged

parser = argparse.ArgumentParser()
parser.add_argument("-o", "--output", dest="out_file", required=True,
    help="merged output kernel file name")
parser.add_argument('dat_files', metavar='IN_FILE', type=str, nargs='+',
    help='list of input kernels to merge into common kernel (.dat files, order is important)')
args = parser.parse_args()

for filename in args.dat_files:
    if not os.path.isfile(filename) or not os.access(filename, os.R_OK):
        raise Exception("Can't read input: %s" % filename)

n_dats = len(args.dat_files)
if n_dats > 15:
    raise Exception("Can't merge more than 15 files")

offset  = [0 for i in range(64)]
padding = [0 for i in range(64)]

# offset of the first kernel (first 64 bytes are header for combined kernel)
offset[0] = 64
# program offset for each kernel in the header of combined kernel
for i in range(n_dats):
    count = offset[i] + os.path.getsize(args.dat_files[i])
    next_offset = ((count + 63) >> 6) << 6 # align next kernel at 64-bytes
    padding[i] = next_offset - count
    if (i != n_dats-1):
        offset[i+1] = next_offset

output = open(args.out_file, 'wb')

# total number of kernels we are going to merge
bytes = struct.pack('i', n_dats)
output.write(bytes)

# write offsets
for i in range(n_dats):
    bytes = struct.pack('i', offset[i])
    output.write(bytes)

# zero fill for the rest of the header, totally:
#  64 bytes header - 4*number_of_kernels - 4 bytes for number of kernels
zerofill = chr(0)*(64 - 4 - (n_dats << 2))
output.write(zerofill.encode(encoding = "utf-8"))

# write combined kernel, each child kernel is 64 bytes aligned
for i in range(n_dats):
    fileobj  = open(args.dat_files[i], 'rb')
    while 1:
        filebytes = fileobj.read()
        if not filebytes:
            zerofill = chr(0)*padding[i]
            output.write(zerofill.encode(encoding = "utf-8"))
            break
        output.write(filebytes)
    fileobj.close()

# padding 128 bytes at the end of combined kernel which is required by HW
zerofill = chr(0)*128
output.write(zerofill.encode(encoding = "utf-8"))

output.close()
print ("Completed kernel merge")
