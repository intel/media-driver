## DS and HME Kernel compile and convert it to .c

### Step1: Building CM Compiler

1. Download or clone cm compiler

   https://github.com/intel/cm-compiler

2. Build cmc following the steps in readme @https://github.com/intel/cm-compiler

3. Download or clone intel graphics comiler

   https://github.com/intel/intel-graphics-compiler

4. Build igc following the steps in readme @ https://github.com/intel/intel-graphics-compiler

### Step2: Building Kernels

1. Set envrionment variale (Set compiler path)

   export PATH=$PATH:<cm_compiler_path>/include:<cm_compiler_path>/bin

2. run build.py to generate kernel binary

### Step3: covert Kernels to .c Array file

1. run command to generate source files.

   "KernelBinToSource -i commonkernel.krn -o media_driver/agnostic/gen11/codec/kernel/ -v igcodeckrn_g11 -index 14 -t 17"

   please refer media_driver/agnostic/common/codec/kernel/codeckrnheader.h about the index and total number.
