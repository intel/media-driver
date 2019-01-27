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
         mov    (2|M0)     f0.0<1>:uw          r[a0.3]<2;2,1>:uw         
         mov    (2|M0)     f1.0<1>:uw          r[a0.3,4]<2;2,1>:uw       
(f0.0)   mov    (16|M0)    r[a0.0]<1>:uw       r[a0.4]<16;16,1>:uw       
(f0.0)   mov    (16|M0)    r[a0.1]<1>:uw       r[a0.5]<16;16,1>:uw       
(f0.0)   mov    (16|M0)    r[a0.2]<1>:uw       r[a0.6]<16;16,1>:uw       
(f0.1)   mov    (16|M0)    r[a0.0,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    
(f0.1)   mov    (16|M0)    r[a0.1,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    
(f0.1)   mov    (16|M0)    r[a0.2,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    
         add    (4|M0)     a0.0<1>:ud          a0.0<4;4,1>:ud            r22.4<1;2,0>:ud    
(f1.0)   mov    (16|M0)    r[a0.0]<1>:uw       r[a0.4]<16;16,1>:uw       
(f1.0)   mov    (16|M0)    r[a0.1]<1>:uw       r[a0.5]<16;16,1>:uw       
(f1.0)   mov    (16|M0)    r[a0.2]<1>:uw       r[a0.6]<16;16,1>:uw       
(f1.1)   mov    (16|M0)    r[a0.0,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    
(f1.1)   mov    (16|M0)    r[a0.1,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    
(f1.1)   mov    (16|M0)    r[a0.2,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    
