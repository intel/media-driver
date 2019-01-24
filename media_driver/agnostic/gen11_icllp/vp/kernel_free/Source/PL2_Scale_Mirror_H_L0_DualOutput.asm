/* 
* Copyright (c) 2017, Intel Corporation
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
* OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*/
 
L0:
         mov     (1|M0)    a0.8<1>:uw     0x800:uw          
         mov     (1|M0)    a0.9<1>:uw     0x880:uw          
         mov     (1|M0)    a0.10<1>:uw    0x8C0:uw          
         add     (4|M0)    a0.12<1>:uw    a0.8<4;4,1>:uw    0x100:uw          
         add     (1|M0)    r8.7<1>:d      ip.0<0;1,0>:ud    32:d              
(W)      jmpi    L0        
L96:
         mov     (1|M0)    a0.8<1>:uw     0xA00:uw          
         mov     (1|M0)    a0.9<1>:uw     0xA80:uw          
         mov     (1|M0)    a0.10<1>:uw    0xAC0:uw          
         add     (4|M0)    a0.12<1>:uw    a0.8<4;4,1>:uw    0x100:uw          
         add     (1|M0)    r8.7<1>:d      ip.0<0;1,0>:ud    32:d              
(W)      jmpi    L0        
L192:
         mov     (1|M0)    a0.8<1>:uw     0xC00:uw          
         mov     (1|M0)    a0.9<1>:uw     0xC80:uw          
         mov     (1|M0)    a0.10<1>:uw    0xCC0:uw          
         add     (4|M0)    a0.12<1>:uw    a0.8<4;4,1>:uw    0x100:uw          
         add     (1|M0)    r8.7<1>:d      ip.0<0;1,0>:ud    32:d              
(W)      jmpi    L0        
L288:
         mov     (1|M0)    a0.8<1>:uw     0xE00:uw          
         mov     (1|M0)    a0.9<1>:uw     0xE80:uw          
         mov     (1|M0)    a0.10<1>:uw    0xEC0:uw          
         add     (4|M0)    a0.12<1>:uw    a0.8<4;4,1>:uw    0x100:uw          
         add     (1|M0)    r8.7<1>:d      ip.0<0;1,0>:ud    32:d              
(W)      jmpi    L0        
L384:
         add     (1|M0)    r7.0<1>:w      r2.0<0;1,0>:uw    -r7.0<0;1,0>:w    
         add     (1|M0)    r7.0<1>:w      r7.0<0;1,0>:w     -16:w             
