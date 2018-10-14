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
(W&~f0.1)jmpi    L1792      
L16:
         mov     (8|M0)     acc0.0<1>:f          r25.3<0;1,0>:f       
         mac     (1|M0)     r23.3<1>:f           r25.5<0;1,0>:f       3.0:f             
         mov     (4|M0)     r10.0<1>:f           0x5054585C:vf        
         mov     (4|M0)     r10.4<1>:f           0x304048:vf          
         mov     (8|M0)     r13.0<1>:ud          r0.0<8;8,1>:ud       
         mov     (2|M0)     r13.0<1>:ud          0x0:ud               
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (8|M0)     acc0.0<1>:f          r23.3<0;1,0>:f       
         mac     (8|M0)     r16.0<1>:f           r25.5<0;1,0>:f       r10.7<0;1,0>:f    
         mac     (8|M0)     r17.0<1>:f           r25.5<0;1,0>:f       -1.0:f            
         mov     (8|M0)     acc0.0<1>:f          r25.2<0;1,0>:f       
         mac     (8|M0)     r14.0<1>:f           r25.4<0;1,0>:f       r10.0<8;8,1>:f    
         mac     (8|M0)     r15.0<1>:f           r25.4<0;1,0>:f       r10.0<8;8,1>:f    
         mul     (8|M0)     acc0.0<1>:f          r25.4<0;1,0>:f       8.0:f             
         add     (8|M0)     r14.0<1>:f           acc0.0<8;8,1>:f      r14.0<8;8,1>:f    
         add     (8|M0)     r15.0<1>:f           acc0.0<8;8,1>:f      r15.0<8;8,1>:f    
         send    (16|M0)    r120:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0201:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r124:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0302:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r126:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f          r25.5<0;1,0>:f       -2.0:f            
         add     (8|M0)     r16.0<1>:f           acc0.0<8;8,1>:f      r16.0<8;8,1>:f    
         add     (8|M0)     r17.0<1>:f           acc0.0<8;8,1>:f      r17.0<8;8,1>:f    
         mul     (16|M0)    acc0.0<1>:f          r120.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r112.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r124.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r116.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r126.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r118.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r112.0<1>:uw         r112.0<16;8,2>:uw    
         mov     (16|M0)    r116.0<1>:uw         r116.0<16;8,2>:uw    
         mov     (16|M0)    r118.0<1>:uw         r118.0<16;8,2>:uw    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r120:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0201:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r124:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0302:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r126:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f          r25.5<0;1,0>:f       2.0:f             
         add     (8|M0)     r16.0<1>:f           acc0.0<8;8,1>:f      r16.0<8;8,1>:f    
         add     (8|M0)     r17.0<1>:f           acc0.0<8;8,1>:f      r17.0<8;8,1>:f    
         mov     (8|M0)     acc0.0<1>:f          r14.0<8;8,1>:f       
         mac     (8|M0)     r14.0<1>:f           r25.4<0;1,0>:f       -8.0:f            
         mov     (8|M0)     acc0.0<1>:f          r15.0<8;8,1>:f       
         mac     (8|M0)     r15.0<1>:f           r25.4<0;1,0>:f       -8.0:f            
         mul     (16|M0)    acc0.0<1>:f          r120.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r120.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r124.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r122.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r126.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r124.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r113.0<1>:uw         r120.0<16;8,2>:uw    
         mov     (16|M0)    r117.0<1>:uw         r122.0<16;8,2>:uw    
         mov     (16|M0)    r119.0<1>:uw         r124.0<16;8,2>:uw    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r120:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0201:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r124:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0302:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r126:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f          r25.5<0;1,0>:f       -2.0:f            
         add     (8|M0)     r16.0<1>:f           acc0.0<8;8,1>:f      r16.0<8;8,1>:f    
         add     (8|M0)     r17.0<1>:f           acc0.0<8;8,1>:f      r17.0<8;8,1>:f    
         mul     (16|M0)    acc0.0<1>:f          r120.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r120.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r124.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r124.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r126.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r126.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r10.0<1>:uw          r120.0<16;8,2>:uw    
         mov     (16|M0)    r11.0<1>:uw          r124.0<16;8,2>:uw    
         mov     (16|M0)    r12.0<1>:uw          r126.0<16;8,2>:uw    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r120:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0201:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r124:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud           r23.5<0;1,0>:ud      0xA2C0302:ud      
         mov     (1|M0)     r13.2<1>:ud          0xE000:ud            
         send    (16|M0)    r126:uw              r13:ub               0x2               a0.0    
         mul     (16|M0)    acc0.0<1>:f          r120.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r120.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r124.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r124.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f          r126.0<8;8,1>:f      65280.0:f         
         mov     (16|M0)    (sat)r126.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r121.0<1>:uw         r120.0<16;8,2>:uw    
         mov     (16|M0)    r125.0<1>:uw         r124.0<16;8,2>:uw    
         mov     (16|M0)    r127.0<1>:uw         r126.0<16;8,2>:uw    
         mov     (16|M0)    r120.0<1>:uw         r10.0<16;16,1>:uw    
         mov     (16|M0)    r124.0<1>:uw         r11.0<16;16,1>:uw    
         mov     (16|M0)    r126.0<1>:uw         r12.0<16;16,1>:uw    
         mov     (16|M0)    r114.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r115.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r122.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r123.0<1>:uw         0xFFFF:uw            
         mov     (1|M0)     a0.8<1>:uw           0xE00:uw             
         mov     (1|M0)     a0.9<1>:uw           0xE80:uw             
         mov     (1|M0)     a0.10<1>:uw          0xEC0:uw             
         add     (4|M0)     a0.12<1>:uw          a0.8<4;4,1>:uw       0x100:uw          
L1792:
         nop     
