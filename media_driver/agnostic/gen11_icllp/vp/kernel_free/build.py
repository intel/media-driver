import os,shutil
import sys

import os
from sys import platform

class WinKernel2Source(object):
    def __init__(self, component='vp'):
        self.input_dir = '.\\'
        self.bin_dir ='.\\compile'
        self.component = 'vp'
        self.bintool = 'GenKrnBin.exe'
        self.sourcetool = 'KernelBinToSource.exe'

    def iga_compile(self):
        for filename in os.listdir('.\\Source'):
            fp = os.path.join('.\\Source', filename)
            fileext = os.path.splitext(filename)[1]
            if os.path.isfile(fp) and 'asm' in fileext:
                newfilename = os.path.splitext(filename)[0]
                newfilename += '.krn'
                newfp = os.path.join('.\\Source', newfilename)
                command = '.\\compile\\IGA\\iga64.exe -p 11 -a '
                command += fp
                command += ' -o '
                command += newfp;
                os.system(command)

                command_Hex = '.\\compile\\KrnToHex_IGA.exe '
                command_Hex += newfp;
                os.system(command_Hex)

                hexfilename  = os.path.splitext(filename)[0]
                hexfilename += '.hex'
                hexfp = os.path.join(".\\Source", hexfilename)

                isExists=os.path.exists("component_release")

                if not isExists:
                    os.mkdir("component_release")

                shutil.copy(hexfp, "component_release")

                os.remove(newfp)
                os.remove(hexfp)

                print("Compiled %s" % fp);

    def kernel2bin(self):
        cmd = '.\\compile\\GenKrnBin.exe component_release vp'
        os.system(cmd)

    def bin2source(self):
        cmd = '.\\compile\\KernelBinToSource.exe -i .\\component_release\\igvpkrn_g11_icllp.bin -o .\\'
        os.system(cmd)
        os.remove(r'..\\..\\..\\common\\vp\\kernel\\vpkrnheader.h')
        os.rename('.\\component_release\\igvpkrn_g11_icllp.h', '..\\..\\..\\common\\vp\\kernel\\vpkrnheader.h')
        os.remove(r'.\\component_release\\igvpkrn_g11_icllp.bin')

class LinuxKernel2Source(object):
    def __init__(self, component='vp'):
        self.input_dir = './'
        self.bin_dir ='./compile'
        self.component = 'vp'
        self.bintool = 'GenKrnBin'
        self.sourcetool = 'KernelBinToSource'

    def iga_compile(self):
        for filename in os.listdir('./Source'):
            fp = os.path.join('./Source', filename)
            fileext = os.path.splitext(filename)[1]
            if os.path.isfile(fp) and 'asm' in fileext:
                newfilename = os.path.splitext(filename)[0]
                newfilename += '.krn'
                newfp = os.path.join('./Source', newfilename)
                command = './compile/IGA/iga64.exe -p 11 -a '
                command += fp
                command += ' -o '
                command += newfp;
                os.system(command)

                command_Hex = './compile/KrnToHex_IGA '
                command_Hex += newfp;
                os.system(command_Hex)

        print ('test')

        hexfilename  = os.path.splitext(filename)[0]
        hexfilename += '.hex'
        hexfp = os.path.join("./Source", hexfilename)

        isExists=os.path.exists("component_release")

        if not isExists:
            os.mkdir("component_release")

        shutil.copy(hexfp, "component_release")

        os.remove(newfp)
        os.remove(hexfp)

        print("Compiled %s" % fp);

    def kernel2bin(self):
        cmd = './compile/GenKrnBin component_release vp'
        os.system(cmd)

    def bin2source(self):
        cmd = './compile/KernelBinToSource -i ./component_release/igvpkrn_g11_icllp.bin -o ./'
        os.system(cmd)
        os.remove(r'../../../common/vp/kernel/vpkrnheader.h')
        os.rename('./component_release/igvpkrn_g11_icllp.h', '../../../common/vp/kernel/vpkrnheader.h')
        os.remove(r'./component_release/igvpkrn_g11_icllp.bin')

def main():
    if platform == "linux" or platform == "linux2":
        k2s = LinuxKernel2Source()
    else:
        k2s = WinKernel2Source()

    k2s.iga_compile()
    k2s.kernel2bin()
    k2s.bin2source()

if __name__=='__main__':
    main()
