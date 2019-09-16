import os,shutil
import sys

import struct
import os
from sys import platform

class GenSource2Bin(object):
    def __init__(self, component='codec'):
        self.fromdir    = './Binary'

    def iga_compile(self):
        for filename in os.listdir('./Source'):
            fp = os.path.join('./Source', filename)
            fileext = os.path.splitext(filename)[1]
            if os.path.isfile(fp) and 'cpp' in fileext:
                command  = 'cmc /c /Qxcm -Qxcm_jit_target=gen11lp '
                command += fp
                command += ' -mCM_emit_common_isa  -mCM_no_input_reorder -mCM_jit_option="-nocompaction" '
                os.system(command)

                print("Compiled %s" % fp);

    def deleteFile(self):
        for filename in os.listdir('./'):
            fp = os.path.join('./', filename)
            fileext = os.path.splitext(filename)[1]
            if os.path.isfile(fp) and 'asm' in fileext:
                os.remove(fp)

            if os.path.isfile(fp) and 'dat' in fileext:
                os.remove(fp)

            if os.path.isfile(fp) and 'visaasm' in fileext:
                os.remove(fp)

            if os.path.isfile(fp) and 'isa' in fileext:
                os.remove(fp)

    def join(self):
        isExists=os.path.exists("Binary")
        if not isExists:
            os.mkdir("Binary")

        shutil.copy('downscale_kernel_genx_0.dat', 'Binary/DS4x_Frame.krn')
        shutil.copy('downscale_kernel_genx_1.dat', 'Binary/DS2x_Frame.krn')
        shutil.copy('hme_kernel_genx_0.dat', 'Binary/HME_P.krn')
        shutil.copy('hme_kernel_genx_1.dat', 'Binary/HME_B.krn')
        shutil.copy('hme_kernel_genx_2.dat', 'Binary/HME_VDENC.krn')
        shutil.copy('downscale_convert_kernel_genx_0.dat', 'Binary/DS_Convert.krn')

        output = open('commonkernel.krn', 'wb')

        bytes  = struct.pack('i', 6)
        output.write(bytes)

        count  = 64
        bytes  = struct.pack('i', count)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'DS4x_Frame.krn')
        count   += os.path.getsize(filepath)
        count1   = ((count + 63) >> 6) << 6
        byteremain1 = count1 - count

        bytes  = struct.pack('i', count1)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'DS2x_Frame.krn')
        count1  += os.path.getsize(filepath)
        count2   = ((count1 + 63) >> 6) << 6
        byteremain2 = count2 - count1

        bytes  = struct.pack('i', count2)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'HME_P.krn')
        count2  += os.path.getsize(filepath)
        count3   = ((count2 + 63) >> 6) << 6
        byteremain3 = count3 - count2

        bytes  = struct.pack('i', count3)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'HME_B.krn')
        count3  += os.path.getsize(filepath)
        count4   = ((count3 + 63) >> 6) << 6
        byteremain4 = count4 - count3

        bytes  = struct.pack('i', count4)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'HME_VDENC.krn')
        count4  += os.path.getsize(filepath)
        count5   = ((count4 + 63) >> 6) << 6
        byteremain5 = count5 - count4

        bytes  = struct.pack('i', count5)
        output.write(bytes)

        filepath = os.path.join('./Binary', 'DS_Convert.krn')
        count5  += os.path.getsize(filepath)
        count6   = ((count5 + 63) >> 6) << 6
        byteremain6 = count6 - count5

        zerofill = chr(0)*36
        output.write(zerofill.encode(encoding = "utf-8"));

        filepath = os.path.join('./Binary', 'DS4x_Frame.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                byteremain = count1 - os.path.getsize(filepath)
                zerofill = chr(0)*byteremain1
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        filepath = os.path.join('./Binary', 'DS2x_Frame.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                zerofill = chr(0)*byteremain2
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        filepath = os.path.join('./Binary', 'HME_P.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                zerofill = chr(0)*byteremain3
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        filepath = os.path.join('./Binary', 'HME_B.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                zerofill = chr(0)*byteremain4
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        filepath = os.path.join('./Binary', 'HME_VDENC.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                zerofill = chr(0)*byteremain5
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        filepath = os.path.join('./Binary', 'DS_Convert.krn')
        fileobj  = open(filepath, 'rb')

        while 1:
            filebytes = fileobj.read()

            if not filebytes:
                zerofill = chr(0)*128
                output.write(zerofill.encode(encoding = "utf-8"))
                break

            output.write(filebytes)
        fileobj.close()

        output.close()
        print ("Completed kernel merge")

def main():

    k2s = GenSource2Bin()

    k2s.iga_compile()
    k2s.join()
    k2s.deleteFile()

if __name__=='__main__':
    main()
