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
         mov    (8|M0)                r27.0<1>:ud    r0.0<8;8,1>:ud     
         mov    (8|M0)                r25.0<1>:ud    r0.0<8;8,1>:ud     
         mov    (8|M0)                r26.0<1>:ud    r0.0<8;8,1>:ud     
         mul    (8|M0)                r3.0<1>:f      r3.0<8;8,1>:f      r7.4<0;1,0>:f     
         mov    (2|M0)                r8.5<1>:f      r7.0<2;2,1>:w      
         cmp    (8|M0)    (eq)f0.0    null.0<1>:w    r2.26<0;1,0>:ub    0x1:uw            
         shr    (1|M0)                r13.0<1>:uw    r2.2<0;1,0>:ud     0x0:ud            
         shr    (1|M0)                r13.1<1>:uw    r2.2<0;1,0>:ud     0x3:ud            
         shr    (1|M0)                r13.2<1>:uw    r2.2<0;1,0>:ud     0x6:ud            
         shr    (1|M0)                r13.3<1>:uw    r2.2<0;1,0>:ud     0x9:ud            
         shr    (1|M0)                r13.4<1>:uw    r2.2<0;1,0>:ud     0xC:ud            
         shr    (1|M0)                r13.5<1>:uw    r2.2<0;1,0>:ud     0xF:ud            
         shr    (1|M0)                r13.6<1>:uw    r2.2<0;1,0>:ud     0x12:ud           
         shr    (1|M0)                r13.7<1>:uw    r2.2<0;1,0>:ud     0x15:ud           
         and    (8|M0)                r13.0<1>:uw    r13.0<8;8,1>:uw    0x7:uw            
         mov    (8|M0)                r17.0<1>:f     r8.5<0;1,0>:f      
         mov    (8|M0)                r15.0<1>:f     r8.6<0;1,0>:f      
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x11111111:uv     
(f0.1)   mov    (8|M0)                r17.0<1>:f     r8.6<0;1,0>:f      
(f0.1)   mov    (8|M0)                r14.0<1>:f     r2.0<0;1,0>:uw     
(f0.1)   add    (8|M0)                r15.0<1>:f     -r8.5<0;1,0>:f     r14.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r15.0<1>:f     r15.0<8;8,1>:f     -16.0:f           
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x22222222:uv     
(f0.1)   mov    (8|M0)                r16.0<1>:f     r2.0<0;1,0>:uw     
(f0.1)   add    (8|M0)                r17.0<1>:f     -r8.5<0;1,0>:f     r16.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r17.0<1>:f     r17.0<8;8,1>:f     -16.0:f           
(f0.1)   mov    (8|M0)                r14.0<1>:f     r2.1<0;1,0>:uw     
(f0.1)   add    (8|M0)                r15.0<1>:f     -r8.6<0;1,0>:f     r14.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r15.0<1>:f     r15.0<8;8,1>:f     -16.0:f           
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x33333333:uv     
(f0.1)   mov    (8|M0)                r16.0<1>:f     r2.1<0;1,0>:uw     
(f0.1)   add    (8|M0)                r17.0<1>:f     -r8.6<0;1,0>:f     r16.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r17.0<1>:f     r17.0<8;8,1>:f     -16.0:f           
(f0.1)   mov    (8|M0)                r15.0<1>:f     r8.5<0;1,0>:f      
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x44444444:uv     
(f0.1)   mov    (8|M0)                r16.0<1>:f     r2.0<0;1,0>:uw     
(f0.1)   add    (8|M0)                r17.0<1>:f     -r8.5<0;1,0>:f     r16.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r17.0<1>:f     r17.0<8;8,1>:f     -16.0:f           
(f0.1)   mov    (8|M0)                r15.0<1>:f     r8.6<0;1,0>:f      
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x77777777:uv     
(f0.1)   mov    (8|M0)                r17.0<1>:f     r8.6<0;1,0>:f      
(f0.1)   mov    (8|M0)                r15.0<1>:f     r8.5<0;1,0>:f      
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x55555555:uv     
(f0.1)   mov    (8|M0)                r17.0<1>:f     r8.5<0;1,0>:f      
(f0.1)   mov    (8|M0)                r14.0<1>:f     r2.1<0;1,0>:uw     
(f0.1)   add    (8|M0)                r15.0<1>:f     -r8.6<0;1,0>:f     r14.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r15.0<1>:f     r15.0<8;8,1>:f     -16.0:f           
         cmp    (8|M0)    (eq)f0.1    null.0<1>:w    r13.0<8;8,1>:uw    0x66666666:uv     
(f0.1)   mov    (8|M0)                r16.0<1>:f     r2.1<0;1,0>:uw     
(f0.1)   add    (8|M0)                r17.0<1>:f     -r8.6<0;1,0>:f     r16.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r17.0<1>:f     r17.0<8;8,1>:f     -16.0:f           
(f0.1)   mov    (8|M0)                r14.0<1>:f     r2.0<0;1,0>:uw     
(f0.1)   add    (8|M0)                r15.0<1>:f     -r8.5<0;1,0>:f     r14.0<8;8,1>:f    
(f0.1)   add    (8|M0)                r15.0<1>:f     r15.0<8;8,1>:f     -16.0:f           
(~f0.0)  mov    (8|M0)                acc0.0<1>:f    r6.0<8;8,1>:f      
(~f0.0)  mac    (8|M0)                r6.0<1>:f      r3.0<8;8,1>:f      r17.0<8;8,1>:f    
         mov    (8|M0)                acc0.0<1>:f    r5.0<8;8,1>:f      
         mac    (8|M0)                r5.0<1>:f      r4.0<8;8,1>:f      r15.0<8;8,1>:f    
