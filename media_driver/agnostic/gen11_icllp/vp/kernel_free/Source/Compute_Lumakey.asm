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
         add    (1|M0)     a0.3<1>:w          a0.3<0;1,0>:w             -32:w                  
         shl    (16|M0)    r15.0<1>:uw        r2.25<0;1,0>:ub           0x8:uw                 
         shl    (16|M0)    r14.0<1>:uw        r2.24<0;1,0>:ub           0x8:uw                 
         and    (16|M0)    r13.0<1>:uw        r[a0.5]<16;16,1>:uw       0xFF00:uw              
         cmp    (16|M0)    (le)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r15.0<16;16,1>:uw
(f0.0)   cmp    (16|M0)    (ge)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r14.0<16;16,1>:uw
         not    (1|M0)     r17.0<1>:uw        f0.0<0;1,0>:uw            
         and    (1|M0)     r[a0.3]<1>:uw      r17.0<0;1,0>:uw           r[a0.3]<0;1,0>:uw
         and    (16|M0)    r13.0<1>:uw        r[a0.5,32]<16;16,1>:uw    0xFF00:uw              
         cmp    (16|M0)    (le)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r15.0<16;16,1>:uw
(f0.0)   cmp    (16|M0)    (ge)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r14.0<16;16,1>:uw
         not    (1|M0)     r17.1<1>:uw        f0.0<0;1,0>:uw            
         and    (1|M0)     r[a0.3,2]<1>:uw    r17.1<0;1,0>:uw           r[a0.3,2]<0;1,0>:uw
         add    (2|M0)     a0.2<1>:ud         a0.2<2;2,1>:ud            r22.5<0;1,0>:ud        
         and    (16|M0)    r13.0<1>:uw        r[a0.5]<16;16,1>:uw       0xFF00:uw              
         cmp    (16|M0)    (le)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r15.0<16;16,1>:uw
(f0.0)   cmp    (16|M0)    (ge)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r14.0<16;16,1>:uw
         not    (1|M0)     r17.2<1>:uw        f0.0<0;1,0>:uw            
         and    (1|M0)     r[a0.3,4]<1>:uw    r17.2<0;1,0>:uw           r[a0.3,4]<0;1,0>:uw
         and    (16|M0)    r13.0<1>:uw        r[a0.5,32]<16;16,1>:uw    0xFF00:uw              
         cmp    (16|M0)    (le)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r15.0<16;16,1>:uw
(f0.0)   cmp    (16|M0)    (ge)f0.0           null.0<1>:w               r13.0<16;16,1>:uw      r14.0<16;16,1>:uw
         not    (1|M0)     r17.3<1>:uw        f0.0<0;1,0>:uw            
         and    (1|M0)     r[a0.3,6]<1>:uw    r17.3<0;1,0>:uw           r[a0.3,6]<0;1,0>:uw
         add    (2|M0)     a0.2<1>:ud         a0.2<2;2,1>:ud            -r22.5<0;1,0>:ud
