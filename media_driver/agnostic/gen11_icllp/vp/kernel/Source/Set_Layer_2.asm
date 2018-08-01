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
         mov     (16|M0)                r20.0<1>:uw    0xFFFF:uw            
         cmp     (1|M0)     (ne)f0.0    null.0<1>:w    r7.6<0;1,0>:uw       0x0:uw             
(f0.0)   cmp     (1|M0)     (ne)f0.0    null.0<1>:w    r7.7<0;1,0>:uw       0x0:uw             
         mov     (1|M0)                 r24.0<1>:ub    f0.0<0;1,0>:uw       
(W&~f0.0)jmpi    L592        
L80:
         mov     (1|M0)                 r26.6<1>:f     r7.5<0;1,0>:f        
         mov     (1|M0)                 r26.4<1>:f     r3.2<0;1,0>:f        
         mov     (1|M0)                 r26.5<1>:f     r4.2<0;1,0>:f        
         mov     (1|M0)                 r26.2<1>:f     r6.2<0;1,0>:f        
         mov     (1|M0)                 r26.3<1>:f     r5.2<0;1,0>:f        
         mov     (8|M0)                 acc0.0<1>:f    r6.2<0;1,0>:f        
         mov     (1|M0)                 r9.6<1>:f      r6.2<0;1,0>:f        
         mac     (1|M0)                 acc0.0<1>:f    r3.2<0;1,0>:f        4.0:f              
         mac     (1|M0)                 r9.0<1>:f      r7.5<0;1,0>:f        6.0:f              
         mac     (1|M0)                 acc0.1<1>:f    r3.2<0;1,0>:f        8.0:f              
         mac     (1|M0)                 r9.1<1>:f      r7.5<0;1,0>:f        28.0:f             
         mac     (1|M0)                 acc0.2<1>:f    r3.2<0;1,0>:f        12.0:f             
         mac     (1|M0)                 r9.2<1>:f      r7.5<0;1,0>:f        66.0:f             
         mov     (8|M0)                 acc0.0<1>:f    r5.2<0;1,0>:f        
         mov     (1|M0)                 r9.7<1>:f      r5.2<0;1,0>:f        
         mac     (1|M0)                 r9.3<1>:f      r4.2<0;1,0>:f        4.0:f              
         mac     (1|M0)                 r9.4<1>:f      r4.2<0;1,0>:f        8.0:f              
         mac     (1|M0)                 r9.5<1>:f      r4.2<0;1,0>:f        12.0:f             
         mul     (1|M0)                 r24.1<1>:f     r4.2<0;1,0>:f        4.0:f              
         mov     (1|M0)                 r24.2<1>:ub    0x0:uw               
         mov     (1|M0)                 r24.5<1>:ud    0x6:ud               
         cmp     (1|M0)     (eq)f0.0    null.0<1>:d    r7.3<0;1,0>:ud       0xFFFFFFFF:ud      
(W&f0.0) jmpi    L576        
L448:
         mov     (1|M0)                 f0.0<1>:uw     r7.7<0;1,0>:uw       
         mov     (16|M0)                r16.0<1>:uw    0x0:uw               
         mov     (8|M0)                 r17.0<1>:ub    r7.12<1;4,0>:ub      
(f0.0)   mov     (16|M0)                r16.0<1>:uw    0xFFFF:uw            
         mov     (16|M0)                r16.0<1>:ub    r16.0<32;16,2>:ub    
         mov     (8|M0)                 r16.0<1>:ud    r16.0<1;2,0>:ud      
         mov     (8|M0)                 r17.0<1>:ud    r17.0<0;2,1>:ud      
         and     (8|M0)                 r20.0<1>:ud    r17.0<8;8,1>:ud      r16.0<8;8,1>:ud    
L576:
         mov     (1|M0)                 r24.1<1>:ub    r1.25<0;1,0>:ub      
L592:
         nop     
