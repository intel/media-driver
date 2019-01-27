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
         mov     (16|M0)                r21.0<1>:uw     0xFFFF:uw         
         cmp     (1|M0)     (ne)f0.1    null.0<1>:w     r7.2<0;1,0>:uw    0x0:uw    
(f0.1)   cmp     (1|M0)     (ne)f0.1    null.0<1>:w     r7.3<0;1,0>:uw    0x0:uw    
         mov     (1|M0)                 r24.24<1>:ub    0x1:uw            
         mov     (1|M0)                 r24.0<1>:ub     0x1:uw            
(W&~f0.1)jmpi    L432        
L96:
         mov     (1|M0)                 r25.6<1>:f      r7.5<0;1,0>:f     
         mov     (1|M0)                 r25.4<1>:f      r3.0<0;1,0>:f     
         mov     (1|M0)                 r25.5<1>:f      r4.0<0;1,0>:f     
         mov     (1|M0)                 r25.2<1>:f      r6.0<0;1,0>:f     
         mov     (1|M0)                 r25.3<1>:f      r5.0<0;1,0>:f     
         mov     (8|M0)                 acc0.0<1>:f     r6.0<0;1,0>:f     
         mov     (1|M0)                 r10.6<1>:f      r6.0<0;1,0>:f     
         mac     (1|M0)                 acc0.0<1>:f     r3.0<0;1,0>:f     4.0:f     
         mac     (1|M0)                 r10.0<1>:f      r7.5<0;1,0>:f     6.0:f     
         mac     (1|M0)                 acc0.1<1>:f     r3.0<0;1,0>:f     8.0:f     
         mac     (1|M0)                 r10.1<1>:f      r7.5<0;1,0>:f     28.0:f    
         mac     (1|M0)                 acc0.2<1>:f     r3.0<0;1,0>:f     12.0:f    
         mac     (1|M0)                 r10.2<1>:f      r7.5<0;1,0>:f     66.0:f    
         mov     (8|M0)                 acc0.0<1>:f     r5.0<0;1,0>:f     
         mov     (1|M0)                 r10.7<1>:f      r5.0<0;1,0>:f     
         mac     (1|M0)                 r10.3<1>:f      r4.0<0;1,0>:f     4.0:f     
         mac     (1|M0)                 r10.4<1>:f      r4.0<0;1,0>:f     8.0:f     
         mac     (1|M0)                 r10.5<1>:f      r4.0<0;1,0>:f     12.0:f    
         mul     (1|M0)                 r23.1<1>:f      r4.0<0;1,0>:f     4.0:f     
         mov     (1|M0)                 r24.2<1>:ub     0x0:uw            
         mov     (1|M0)                 r23.5<1>:ud     0x0:ud            
L432:
         nop     
