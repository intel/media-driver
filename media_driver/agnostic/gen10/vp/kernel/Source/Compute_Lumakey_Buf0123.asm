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
         mov    (4|M0)     r17.0<1>:uw     0x3210:v           
         mov    (4|M0)     acc0.0<1>:w     r22.1<0;1,0>:uw    
         mac    (4|M0)     a0.0<1>:uw      r17.0<4;4,1>:uw    0x200:uw                  
         cmp    (16|M0)    (le)f0.0        null.0<1>:w        r[a0.0,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f0.1        null.0<1>:w        r[a0.0,33]<32;16,2>:ub    r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.0        null.0<1>:w        r[a0.1,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.1        null.0<1>:w        r[a0.1,33]<32;16,2>:ub    r2.25<0;1,0>:ub
(f0.0)   cmp    (16|M0)    (ge)f0.0        null.0<1>:w        r[a0.0,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f0.1)   cmp    (16|M0)    (ge)f0.1        null.0<1>:w        r[a0.0,33]<32;16,2>:ub    r2.24<0;1,0>:ub
(f1.0)   cmp    (16|M0)    (ge)f1.0        null.0<1>:w        r[a0.1,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f1.1)   cmp    (16|M0)    (ge)f1.1        null.0<1>:w        r[a0.1,33]<32;16,2>:ub    r2.24<0;1,0>:ub
         not    (2|M0)     r21.0<1>:uw     f0.0<2;2,1>:uw     
         not    (2|M0)     r21.4<1>:uw     f1.0<2;2,1>:uw     
         add    (4|M0)     a0.4<1>:uw      a0.0<4;4,1>:uw     r22.8<1;2,0>:uw           
         cmp    (16|M0)    (le)f0.0        null.0<1>:w        r[a0.2,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f0.1        null.0<1>:w        r[a0.2,33]<32;16,2>:ub    r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.0        null.0<1>:w        r[a0.3,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.1        null.0<1>:w        r[a0.3,33]<32;16,2>:ub    r2.25<0;1,0>:ub
(f0.0)   cmp    (16|M0)    (ge)f0.0        null.0<1>:w        r[a0.2,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f0.1)   cmp    (16|M0)    (ge)f0.1        null.0<1>:w        r[a0.2,33]<32;16,2>:ub    r2.24<0;1,0>:ub
(f1.0)   cmp    (16|M0)    (ge)f1.0        null.0<1>:w        r[a0.3,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f1.1)   cmp    (16|M0)    (ge)f1.1        null.0<1>:w        r[a0.3,33]<32;16,2>:ub    r2.24<0;1,0>:ub
         not    (2|M0)     r21.8<1>:uw     f0.0<2;2,1>:uw     
         not    (2|M0)     r21.12<1>:uw    f1.0<2;2,1>:uw     
         cmp    (16|M0)    (le)f0.0        null.0<1>:w        r[a0.4,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f0.1        null.0<1>:w        r[a0.4,33]<32;16,2>:ub    r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.0        null.0<1>:w        r[a0.5,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.1        null.0<1>:w        r[a0.5,33]<32;16,2>:ub    r2.25<0;1,0>:ub
(f0.0)   cmp    (16|M0)    (ge)f0.0        null.0<1>:w        r[a0.4,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f0.1)   cmp    (16|M0)    (ge)f0.1        null.0<1>:w        r[a0.4,33]<32;16,2>:ub    r2.24<0;1,0>:ub
(f1.0)   cmp    (16|M0)    (ge)f1.0        null.0<1>:w        r[a0.5,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f1.1)   cmp    (16|M0)    (ge)f1.1        null.0<1>:w        r[a0.5,33]<32;16,2>:ub    r2.24<0;1,0>:ub
         not    (2|M0)     r21.2<1>:uw     f0.0<2;2,1>:uw     
         not    (2|M0)     r21.6<1>:uw     f1.0<2;2,1>:uw     
         cmp    (16|M0)    (le)f0.0        null.0<1>:w        r[a0.6,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f0.1        null.0<1>:w        r[a0.6,33]<32;16,2>:ub    r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.0        null.0<1>:w        r[a0.7,1]<32;16,2>:ub     r2.25<0;1,0>:ub
         cmp    (16|M0)    (le)f1.1        null.0<1>:w        r[a0.7,33]<32;16,2>:ub    r2.25<0;1,0>:ub
(f0.0)   cmp    (16|M0)    (ge)f0.0        null.0<1>:w        r[a0.6,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f0.1)   cmp    (16|M0)    (ge)f0.1        null.0<1>:w        r[a0.6,33]<32;16,2>:ub    r2.24<0;1,0>:ub
(f1.0)   cmp    (16|M0)    (ge)f1.0        null.0<1>:w        r[a0.7,1]<32;16,2>:ub     r2.24<0;1,0>:ub
(f1.1)   cmp    (16|M0)    (ge)f1.1        null.0<1>:w        r[a0.7,33]<32;16,2>:ub    r2.24<0;1,0>:ub
         not    (2|M0)     r21.10<1>:uw    f0.0<2;2,1>:uw     
         not    (2|M0)     r21.14<1>:uw    f1.0<2;2,1>:uw     
