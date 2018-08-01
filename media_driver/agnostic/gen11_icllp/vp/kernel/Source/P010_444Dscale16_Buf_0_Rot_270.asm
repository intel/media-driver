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
         mov     (1|M0)     r22.4<1>:ud         0x1000100:ud         
         mov     (4|M0)     acc0.0<1>:w         0x2406:v             
         add     (4|M0)     acc0.0<1>:w         acc0.0<4;4,1>:w      0x40:uw           
         shl     (4|M0)     r22.0<1>:w          acc0.0<4;4,1>:w      0x5:uw            
(W&~f0.1)jmpi    L1616      
L80:
         mov     (8|M0)     acc0.0<1>:f         r25.2<0;1,0>:f       
         mac     (1|M0)     r23.3<1>:f          r25.4<0;1,0>:f       15.0:f            
         mov     (4|M0)     r10.0<1>:f          0x48403000:vf        
         mov     (4|M0)     r10.4<1>:f          0x5C585450:vf        
         mov     (8|M0)     r13.0<1>:ud         r0.0<8;8,1>:ud       
         mov     (2|M0)     r13.0<1>:ud         0x0:ud               
         mov     (1|M0)     r13.2<1>:ud         0xE000:ud            
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (8|M0)     acc0.0<1>:f         r25.3<0;1,0>:f       
         mac     (8|M0)     r16.0<1>:f          r25.5<0;1,0>:f       r10.0<8;8,1>:f    
         mac     (8|M0)     r17.0<1>:f          r25.5<0;1,0>:f       r10.0<8;8,1>:f    
         mov     (8|M0)     acc0.0<1>:f         r23.3<0;1,0>:f       
         mac     (8|M0)     r14.0<1>:f          r25.4<0;1,0>:f       r10.0<0;1,0>:f    
         mul     (8|M0)     acc0.0<1>:f         r25.4<0;1,0>:f       -1.0:f            
         add     (8|M0)     r15.0<1>:f          acc0.0<8;8,1>:f      r14.0<8;8,1>:f    
         send    (16|M0)    r72:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA4C0201:ud      
         mov     (1|M0)     r13.2<1>:ud         0xC000:ud            
         send    (16|M0)    r76:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f         r25.4<0;1,0>:f       -2.0:f            
         add     (8|M0)     r14.0<1>:f          acc0.0<8;8,1>:f      r14.0<8;8,1>:f    
         add     (8|M0)     r15.0<1>:f          acc0.0<8;8,1>:f      r15.0<8;8,1>:f    
         mul     (16|M0)    acc0.0<1>:f         r72.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r64.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r76.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r68.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r78.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r70.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r64.0<1>:uw         r64.0<16;8,2>:uw     
         mov     (16|M0)    r68.0<1>:uw         r68.0<16;8,2>:uw     
         mov     (16|M0)    r70.0<1>:uw         r70.0<16;8,2>:uw     
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud         0xE000:ud            
         send    (16|M0)    r72:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA4C0201:ud      
         mov     (1|M0)     r13.2<1>:ud         0xC000:ud            
         send    (16|M0)    r74:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f         r25.5<0;1,0>:f       8.0:f             
         add     (8|M0)     r16.0<1>:f          acc0.0<8;8,1>:f      r16.0<8;8,1>:f    
         add     (8|M0)     r17.0<1>:f          acc0.0<8;8,1>:f      r17.0<8;8,1>:f    
         mul     (8|M0)     acc0.0<1>:f         r25.4<0;1,0>:f       2.0:f             
         add     (8|M0)     r14.0<1>:f          acc0.0<8;8,1>:f      r14.0<8;8,1>:f    
         add     (8|M0)     r15.0<1>:f          acc0.0<8;8,1>:f      r15.0<8;8,1>:f    
         mul     (16|M0)    acc0.0<1>:f         r72.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r72.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r74.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r74.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r76.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r76.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r65.0<1>:uw         r72.0<16;8,2>:uw     
         mov     (16|M0)    r69.0<1>:uw         r74.0<16;8,2>:uw     
         mov     (16|M0)    r71.0<1>:uw         r76.0<16;8,2>:uw     
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud         0xE000:ud            
         send    (16|M0)    r72:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA4C0201:ud      
         mov     (1|M0)     r13.2<1>:ud         0xC000:ud            
         send    (16|M0)    r76:uw              r13:ub               0x2               a0.0    
         mul     (8|M0)     acc0.0<1>:f         r25.4<0;1,0>:f       -2.0:f            
         add     (8|M0)     r14.0<1>:f          acc0.0<8;8,1>:f      r14.0<8;8,1>:f    
         add     (8|M0)     r15.0<1>:f          acc0.0<8;8,1>:f      r15.0<8;8,1>:f    
         mul     (16|M0)    acc0.0<1>:f         r72.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r72.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r76.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r76.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r78.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r78.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r10.0<1>:uw         r72.0<16;8,2>:uw     
         mov     (16|M0)    r11.0<1>:uw         r76.0<16;8,2>:uw     
         mov     (16|M0)    r12.0<1>:uw         r78.0<16;8,2>:uw     
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA2C0100:ud      
         mov     (1|M0)     r13.2<1>:ud         0xE000:ud            
         send    (16|M0)    r72:uw              r13:ub               0x2               a0.0    
         add     (1|M0)     a0.0<1>:ud          r23.5<0;1,0>:ud      0xA4C0201:ud      
         mov     (1|M0)     r13.2<1>:ud         0xC000:ud            
         send    (16|M0)    r76:uw              r13:ub               0x2               a0.0    
         mul     (16|M0)    acc0.0<1>:f         r72.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r72.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r76.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r76.0<1>:ud    acc0.0<8;8,1>:f      
         mul     (16|M0)    acc0.0<1>:f         r78.0<8;8,1>:f       65472.0:f         
         mov     (16|M0)    (sat)r78.0<1>:ud    acc0.0<8;8,1>:f      
         mov     (16|M0)    r73.0<1>:uw         r72.0<16;8,2>:uw     
         mov     (16|M0)    r77.0<1>:uw         r76.0<16;8,2>:uw     
         mov     (16|M0)    r79.0<1>:uw         r78.0<16;8,2>:uw     
         mov     (16|M0)    r72.0<1>:uw         r10.0<16;16,1>:uw    
         mov     (16|M0)    r76.0<1>:uw         r11.0<16;16,1>:uw    
         mov     (16|M0)    r78.0<1>:uw         r12.0<16;16,1>:uw    
         mov     (16|M0)    r66.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r67.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r74.0<1>:uw         0xFFFF:uw            
         mov     (16|M0)    r75.0<1>:uw         0xFFFF:uw            
         mov     (1|M0)     a0.8<1>:uw          0x800:uw             
         mov     (1|M0)     a0.9<1>:uw          0x880:uw             
         mov     (1|M0)     a0.10<1>:uw         0x8C0:uw             
         add     (4|M0)     a0.12<1>:uw         a0.8<4;4,1>:uw       0x100:uw          
L1616:
         nop     
