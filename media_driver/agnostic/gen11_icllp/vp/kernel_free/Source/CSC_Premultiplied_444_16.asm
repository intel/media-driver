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
(W&~f0.0)jmpi    L1152      
L16:
         shr     (1|M0)     r12.0<1>:w               r2.4<0;1,0>:ub            5:w                
         add     (1|M0)     r12.0<1>:w               r12.0<0;1,0>:w            7:w                
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r18.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r18.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r18.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r18.15<0;1,0>:w
         shr     (16|M0)    (sat)r14.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r19.8<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r19.9<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r19.10<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r19.11<0;1,0>:w
         shr     (16|M0)    (sat)r15.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r19.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r19.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r19.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r19.15<0;1,0>:w
         shr     (16|M0)    (sat)r[a0.2]<1>:uw       acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r18.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r18.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r18.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r18.15<0;1,0>:w
         shr     (16|M0)    (sat)r16.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r19.8<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r19.9<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r19.10<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r19.11<0;1,0>:w
         shr     (16|M0)    (sat)r17.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r19.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r19.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r19.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r19.15<0;1,0>:w
         shr     (16|M0)    (sat)r[a0.2,32]<1>:uw    acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mov     (16|M0)    r[a0.0]<1>:uw            r14.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.1]<1>:uw            r15.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.0,32]<1>:uw         r16.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.1,32]<1>:uw         r17.0<16;16,1>:uw         
         add     (2|M0)     a0.0<1>:ud               a0.0<2;2,1>:ud            r22.6<0;1,0>:ud    
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r18.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r18.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r18.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r18.15<0;1,0>:w
         shr     (16|M0)    (sat)r14.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r19.8<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r19.9<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r19.10<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r19.11<0;1,0>:w
         shr     (16|M0)    (sat)r15.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r19.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r19.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r19.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3]<16;16,1>:uw       r19.15<0;1,0>:w
         shr     (16|M0)    (sat)r[a0.2]<1>:uw       acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r18.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r18.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r18.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r18.15<0;1,0>:w
         shr     (16|M0)    (sat)r16.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r19.8<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r19.9<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r19.10<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r19.11<0;1,0>:w
         shr     (16|M0)    (sat)r17.0<1>:uw         acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mul     (16|M0)    acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r19.12<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r19.13<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r19.14<0;1,0>:w
         mac     (16|M0)    acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    r19.15<0;1,0>:w
         shr     (16|M0)    (sat)r[a0.2,32]<1>:uw    acc0.0<8;8,1>:w           r12.0<0;1,0>:w
         mov     (16|M0)    r[a0.0]<1>:uw            r14.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.1]<1>:uw            r15.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.0,32]<1>:uw         r16.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.1,32]<1>:uw         r17.0<16;16,1>:uw         
L1152:
         nop     
