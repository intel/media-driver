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
         add    (1|M0)    a0.4<1>:uw       a0.4<0;1,0>:uw           r22.14<0;1,0>:uw
         mov    (4|M0)    acc0.8<1>:w      r[a0.4]<1;2,2>:ub        
         mov    (4|M0)    acc0.12<1>:w     r[a0.4,288]<1;2,2>:ub    
         and    (8|M0)    r17.8<1>:uw      acc0.8<4;4,1>:w          0xC:uw               
         and    (8|M0)    r16.8<1>:uw      acc0.8<4;4,1>:w          0x30:uw              
         and    (8|M0)    acc0.8<1>:w      acc0.8<8;8,1>:w          0xC3:uw              
         shl    (8|M0)    r17.8<1>:uw      r17.8<8;8,1>:uw          0x2:uw               
         shr    (8|M0)    r16.8<1>:uw      r16.8<8;8,1>:uw          0x2:uw               
         or     (8|M0)    acc0.8<1>:w      acc0.8<8;8,1>:w          r17.8<8;8,1>:uw      
         or     (8|M0)    r16.8<1>:uw      acc0.8<8;8,1>:w          r16.8<8;8,1>:uw          {AccWrEn}
         and    (8|M0)    r17.8<1>:uw      acc0.8<8;8,1>:w          0xF:uw               
         shl    (4|M0)    r17.9<2>:uw      r17.9<8;4,2>:uw          0x4:uw               
         or     (4|M0)    r16.8<2>:uw      r17.8<8;4,2>:uw          r17.9<8;4,2>:uw      
         and    (8|M0)    r17.8<1>:uw      acc0.8<8;8,1>:w          0xF0:uw              
         shr    (4|M0)    r17.8<2>:uw      r17.8<8;4,2>:uw          0x4:uw               
         or     (4|M0)    r16.9<2>:uw      r17.8<8;4,2>:uw          r17.9<8;4,2>:uw      
         mov    (8|M0)    r17.0<1>:ub      r16.16<16;8,2>:ub        
         and    (4|M0)    r[a0.3]<1>:uw    r[a0.3]<4;4,1>:uw        r17.0<4;4,1>:uw
         add    (1|M0)    a0.4<1>:uw       a0.4<0;1,0>:uw           -r22.14<0;1,0>:uw
