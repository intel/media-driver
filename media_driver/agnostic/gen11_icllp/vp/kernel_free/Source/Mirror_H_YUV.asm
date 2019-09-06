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
         mov     (1|M0)    f0.0<1>:uw           r24.0<0;1,0>:ub         
(W&~f0.0)jmpi    L960      
L32:
         mov     (1|M0)    f1.0<1>:uw           0x5555:uw               
         mov     (8|M0)    r12.0<1>:uw          a0.0<8;8,1>:uw          
         mov     (8|M0)    r13.0<1>:uw          a0.8<8;8,1>:uw          
         add     (2|M0)    r13.0<1>:uw          r13.1<2;2,1>:uw         -r13.0<2;2,1>:uw
         add     (1|M0)    r13.2<1>:uw          r13.7<0;1,0>:uw         -r13.3<0;1,0>:uw
         mov     (1|M0)    a0.3<1>:uw           a0.8<0;1,0>:uw          
         add     (1|M0)    a0.2<1>:uw           a0.3<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.1<1>:uw           a0.2<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.0<1>:uw           a0.1<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.7<1>:uw           a0.0<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.6<1>:uw           a0.7<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.5<1>:uw           a0.6<0;1,0>:uw          0x4:uw              
         add     (1|M0)    a0.4<1>:uw           a0.5<0;1,0>:uw          0x4:uw              
         movi    (8|M0)    r17.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          r13.2<0;1,0>:uw     
         movi    (8|M0)    r11.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.8,16]<1>:uw     r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.8]<1>:uw        r11.1<2;2,0>:uw         r11.0<2;2,0>:uw     
(f1.0)   sel     (8|M0)    r[a0.12,16]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.12]<1>:uw       r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         movi    (8|M0)    r11.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          -r13.2<0;1,0>:uw
         movi    (8|M0)    r17.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.8,48]<1>:uw     r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.8,32]<1>:uw     r11.1<2;2,0>:uw         r11.0<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.12,48]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.12,32]<1>:uw    r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          r13.0<0;1,0>:uw     
         movi    (8|M0)    r17.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          r13.2<0;1,0>:uw     
         movi    (8|M0)    r11.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.9,16]<1>:uw     r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.9]<1>:uw        r11.1<2;2,0>:uw         r11.0<2;2,0>:uw     
(f1.0)   sel     (8|M0)    r[a0.13,16]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.13]<1>:uw       r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         movi    (8|M0)    r11.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          -r13.2<0;1,0>:uw
         movi    (8|M0)    r17.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.9,48]<1>:uw     r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.9,32]<1>:uw     r11.1<2;2,0>:uw         r11.0<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.13,48]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.13,32]<1>:uw    r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          r13.1<0;1,0>:uw     
         movi    (8|M0)    r17.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          r13.2<0;1,0>:uw     
         movi    (8|M0)    r11.0<1>:ud          r[a0.0]<4;4,1>:ud       null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.10,16]<1>:uw    r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.10]<1>:uw       r11.1<2;2,0>:uw         r11.0<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.14,16]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.14]<1>:uw       r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         movi    (8|M0)    r11.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
         add     (8|M0)    a0.0<1>:uw           a0.0<8;8,1>:uw          -r13.2<0;1,0>:uw
         movi    (8|M0)    r17.0<1>:ud          r[a0.0,32]<4;4,1>:ud    null.0<8;8,1>:ub
(f1.0)   sel     (8|M0)    r[a0.10,48]<1>:uw    r11.9<2;2,0>:uw         r11.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.10,32]<1>:uw    r11.1<2;2,0>:uw         r11.0<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.14,48]<1>:uw    r17.9<2;2,0>:uw         r17.8<2;2,0>:uw
(f1.0)   sel     (8|M0)    r[a0.14,32]<1>:uw    r17.1<2;2,0>:uw         r17.0<2;2,0>:uw
         mov     (8|M0)    a0.0<1>:uw           r12.0<8;8,1>:uw         
L960:
         nop     
