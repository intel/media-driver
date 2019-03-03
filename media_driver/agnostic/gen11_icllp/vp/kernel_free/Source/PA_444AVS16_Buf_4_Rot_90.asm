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
         mov     (8|M0)                acc0.0<1>:w     0x64200000:v       
         add     (8|M0)                acc0.0<1>:w     acc0.0<8;8,1>:w    0x1C:uw            
         shl     (4|M0)                r22.4<1>:w      acc0.4<4;4,1>:w    0x5:uw             
         mov     (1|M0)                r22.14<1>:uw    0x100:uw           
         mov     (1|M0)                f0.0<1>:uw      r24.0<0;1,0>:ub    
(W&~f0.0)jmpi    L384        
L96:
         add     (1|M0)                a0.0<1>:ud      r24.5<0;1,0>:ud    0x50EB100:ud       
         mov     (8|M0)                r16.0<1>:ud     r0.0<8;8,1>:ud     
         mov     (1|M0)                r26.0<1>:ud     0x0:ud             
         and     (1|M0)                r12.0<1>:ud     r2.1<0;1,0>:ud     0x8000000:ud       
         or      (1|M0)                r7.7<1>:ud      r7.7<0;1,0>:ud     r12.0<0;1,0>:ud    
         mov     (1|M0)                r26.7<1>:ud     r7.7<0;1,0>:ud     
         mov     (1|M0)                r26.1<1>:ud     r7.12<0;1,0>:uw    
         mov     (1|M0)                r16.2<1>:ud     0x0:ud             
         cmp     (1|M0)    (eq)f1.0    null.0<1>:w     r24.2<0;1,0>:ub    0x1:uw             
(~f1.0)  mov     (1|M0)                r26.2<1>:f      r9.6<0;1,0>:f      
(f1.0)   mov     (1|M0)                r26.2<1>:f      r9.1<0;1,0>:f      
         mov     (8|M0)                r17.0<1>:ud     r26.0<8;8,1>:ud    
         send    (1|M0)                r28:uw          r16:ub             0x2                a0.0    
         mov     (1|M0)                a0.8<1>:uw      0x380:uw           
         mov     (1|M0)                a0.9<1>:uw      0x3C0:uw           
         mov     (1|M0)                a0.10<1>:uw     0x400:uw           
         mov     (1|M0)                a0.11<1>:uw     0x440:uw           
         add     (4|M0)                a0.12<1>:uw     a0.8<4;4,1>:uw     0x100:uw           
L384:
         nop     
