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
         mov     (1|M0)                 r22.5<1>:ud    0x1200120:ud       
         mov     (1|M0)                 f0.0<1>:uw     r24.0<0;1,0>:ub    
(W&~f0.0)jmpi    L752        
L48:
         mov     (8|M0)                 r17.0<1>:ud    r26.0<8;8,1>:ud    
         mov     (1|M0)                 r16.2<1>:ud    0xE000:ud          
         cmp     (1|M0)     (eq)f1.0    null.0<1>:w    r24.2<0;1,0>:ub    0x1:uw          
         add     (1|M0)                 a0.0<1>:ud     r24.5<0;1,0>:ud    0x43EA100:ud    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
         send    (1|M0)                 r46:uw         r16:ub             0x2             a0.0    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
         send    (1|M0)                 r55:uw         r16:ub             0x2             a0.0    
         add     (1|M0)                 a0.0<1>:ud     r24.5<0;1,0>:ud    0x43EA201:ud    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
         send    (1|M0)                 r50:uw         r16:ub             0x2             a0.0    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
         send    (1|M0)                 r59:uw         r16:ub             0x2             a0.0    
         add     (1|M0)                 a0.0<1>:ud     r24.5<0;1,0>:ud    0x43EA302:ud    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.4<0;1,0>:f      
         send    (1|M0)                 r52:uw         r16:ub             0x2             a0.0    
(~f1.0)  mov     (1|M0)                 r17.2<1>:f     r9.0<0;1,0>:f      
(~f1.0)  mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.2<1>:f     r9.2<0;1,0>:f      
(f1.0)   mov     (1|M0)                 r17.3<1>:f     r9.7<0;1,0>:f      
         send    (1|M0)                 r61:uw         r16:ub             0x2             a0.0    
         mov     (16|M0)                r48.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)                r49.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)                r57.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)                r58.0<1>:uw    0xFFFF:uw          
         mov     (1|M0)                 a0.8<1>:uw     0x5C0:uw           
         mov     (1|M0)                 a0.9<1>:uw     0x640:uw           
         mov     (1|M0)                 a0.10<1>:uw    0x680:uw           
         add     (4|M0)                 a0.12<1>:uw    a0.8<4;4,1>:uw     0x120:uw        
L752:
         nop     
