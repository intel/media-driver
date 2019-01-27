# Media Driver Open Source Kernel

## Step1: Building IGA (Intel Graphics Assembler)

1. Download or clone IGC (Intel Graphics Compiler)

   https://github.com/intel/intel-graphics-compiler.git

2. Chdir into 'intel-graphics-compiler' (or any other workspace folder of choice)

   It should read the following folder strucutre:

   workspace
      |- visa
      |- IGC
      |- inc
      |- 3d
      |- skuwa

3. Chdir into IGA sub-component
```
   cd visa/iga
```
4. Create build directory
```
    mkdir build
```
5. Change into build directory
```
    cd build
```
6. Run cmake
```
   cmake ../
```
7. Run make to build IGA project
```
   make
```
8. Get the output executable "iga64" in IGAExe folder


## Step2: Building the other post processing tools

1. Download or clone Tool

   https://github.com/intel/media-driver

2. Chdir into "Tools/MediaDriverTools/GenKrnBin"

3. Create build directory
```
   mkdir build
```
5. Change into build directory
```
   cd build
```
6. Run cmake
```
   cmake ../
```
7. Run make to build GenKrnBin project
```
   make
```
8. Get the output executable "GenKrnBin" in build folder

9. Chdir into "Tools/MediaDriverTools/KernelBinToSource"

10. Create build directory
```
    mkdir build
```
11. Change into build directory

    cd build

12. Run cmake
```
    cmake ../
```
13. Run make to build KernelBinToSource project
```
    make
```
14. Get the output executable "KernelBinToSource" in build folder

15. Chdir into "Tools/MediaDriverTools/KrnToHex_IGA"

16. Create build directory
```
    mkdir build
```
17. Change into build directory
```
    cd build
```
18. Run cmake
```
    cmake ../
```
19. Run make to build KrnToHex_IGA project
```
    make
```
20. Get the output executable "KrnToHex_IGA" in build folder


## Step3: Building the kernel asm code to igvpkrn_g11_icllp.c and igvpkrn_g11_icllp.h

1. Download or clone Driver source code

   https://github.com/intel/media-driver

2. Chdir into "./agnostic/gen11_icllp/vp/kernel"

3. Create compile directory
```
   mkdir compile
```
4. Copy tool biniary (GenKrnBin, KernelBinToSource and KrnToHex_IGA) from step2 to compile folder

3. Create IGA directory
```
   mkdir IGA
```
5. Copy tool biniary (iga64) from step1 to IGA folder

6. Chdir into "./agnostic/gen11_icllp/vp/kernel"

7. run build.py script to generate igvpkrn_g11_icllp.c and igvpkrn_g11_icllp.h