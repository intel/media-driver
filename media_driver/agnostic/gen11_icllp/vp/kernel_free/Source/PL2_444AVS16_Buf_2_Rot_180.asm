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
(W&~f0.1)jmpi    L384       
L16:
         add     (1|M0)     r25.1<1>:ud     r7.12<0;1,0>:uw       0x1:ud          
         mov     (8|M0)     r17.0<1>:ud     r25.0<8;8,1>:ud       
         add     (1|M0)     a0.0<1>:ud      r23.5<0;1,0>:ud       0x44EB100:ud    
         mov     (1|M0)     r16.2<1>:ud     0xD000:ud             
         send    (1|M0)     r100:uw         r16:ub                0x2             a0.0    
         add     (1|M0)     a0.0<1>:ud      r23.5<0;1,0>:ud       0x48EB301:ud    
         or      (1|M0)     r17.7<1>:ud     r17.7<0;1,0>:ud       0x8000000:ud    
         mov     (1|M0)     r16.2<1>:ud     0xA000:ud             
         send    (1|M0)     r104:uw         r16:ub                0x2             a0.0    
         mov     (1|M0)     a0.8<1>:uw      0xC00:uw              
         mov     (1|M0)     a0.9<1>:uw      0xC80:uw              
         mov     (1|M0)     a0.10<1>:uw     0xD00:uw              
         add     (4|M0)     a0.12<1>:uw     a0.8<4;4,1>:uw        0x40:uw         
         mov     (16|M0)    r96.0<1>:uw     r106.0<16;16,1>:uw    
         mov     (16|M0)    r97.0<1>:uw     r107.0<16;16,1>:uw    
         mov     (16|M0)    r106.0<1>:uw    r108.0<16;16,1>:uw    
         mov     (16|M0)    r107.0<1>:uw    r109.0<16;16,1>:uw    
         mov     (16|M0)    r98.0<1>:uw     r110.0<16;16,1>:uw    
         mov     (16|M0)    r99.0<1>:uw     r111.0<16;16,1>:uw    
         mov     (16|M0)    r108.0<1>:uw    0xFFFF:uw             
         mov     (16|M0)    r109.0<1>:uw    0xFFFF:uw             
         mov     (16|M0)    r110.0<1>:uw    0xFFFF:uw             
         mov     (16|M0)    r111.0<1>:uw    0xFFFF:uw             
L384:
         nop     
