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
               cmp     (16|M0)    (eq)f0.0    null.0<1>:w         r21.0<16;16,1>:uw       0xFFFF:uw          
(f0.0.all16h)  cmp     (1|M0)     (eq)f0.0    null.0<1>:d         r7.1<0;1,0>:ud          0xFFFFFFFF:ud      
(W&f0.0.all16h)jmpi    L1696       
L48:
               cmp     (1|M0)     (eq)f0.0    null.0<1>:d         r7.1<0;1,0>:ud          0xFFFFFFFF:ud      
               and     (16|M0)                r21.0<1>:uw         r21.0<16;16,1>:uw       0xFFFF:uw          
(W&f0.0)       jmpi    L240        
L96:
               mov     (1|M0)                 f0.0<1>:uw          r7.3<0;1,0>:uw          
               mov     (16|M0)                r16.0<1>:uw         0x0:uw                  
               mov     (8|M0)                 r17.0<1>:ub         r7.4<1;4,0>:ub          
(f0.0)         mov     (16|M0)                r16.0<1>:uw         0xFFFF:uw               
               mov     (16|M0)                r16.0<1>:ub         r16.0<32;16,2>:ub       
               mov     (8|M0)                 r16.0<1>:ud         r16.0<1;2,0>:ud         
               mov     (8|M0)                 r17.0<1>:ud         r17.0<0;2,1>:ud         
               and     (8|M0)                 acc0.0<1>:d         r17.0<8;8,1>:ud         r16.0<8;8,1>:ud    
               and     (8|M0)                 r21.0<1>:ud         acc0.0<8;8,1>:d         r21.0<8;8,1>:ud    
L240:
               mov     (4|M0)                 a0.0<1>:uw          r22.0<4;4,1>:w          
               add     (2|M0)                 a0.2<1>:ud          r22.0<2;2,1>:ud         r22.4<0;1,0>:ud    
               mov     (8|M0)                 r17.0<1>:uw         a0.0<8;8,1>:uw          
               mov     (1|M0)                 a0.3<1>:uw          0x2A0:uw                
               mov     (2|M0)                 f0.0<1>:uw          r[a0.3]<2;2,1>:uw       
               mov     (2|M0)                 f1.0<1>:uw          r[a0.3,4]<2;2,1>:uw     
               mov     (8|M0)                 a0.0<1>:uw          r17.0<8;8,1>:uw         
(~f0.0)        shl     (16|M0)                r[a0.0]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.1]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.2]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.3]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.0,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.1,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.2,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.3,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.4]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.5]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.6]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.7]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.4,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.5,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.6,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.7,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
               add     (8|M0)                 a0.0<1>:w           a0.0<8;8,1>:w           0x200:uw           
               mov     (8|M0)                 r17.0<1>:uw         a0.0<8;8,1>:uw          
               mov     (1|M0)                 a0.3<1>:uw          0x2A0:uw                
               mov     (2|M0)                 f0.0<1>:uw          r[a0.3,8]<2;2,1>:uw     
               mov     (2|M0)                 f1.0<1>:uw          r[a0.3,12]<2;2,1>:uw    
               mov     (8|M0)                 a0.0<1>:uw          r17.0<8;8,1>:uw         
(~f0.0)        shl     (16|M0)                r[a0.0]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.1]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.2]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.3]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.0,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.1,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.2,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.3,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.4]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.5]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.6]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.7]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.4,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.5,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.6,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.7,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
               add     (8|M0)                 a0.0<1>:w           a0.0<8;8,1>:w           0x200:uw           
               mov     (8|M0)                 r17.0<1>:uw         a0.0<8;8,1>:uw          
               mov     (1|M0)                 a0.3<1>:uw          0x2A0:uw                
               mov     (2|M0)                 f0.0<1>:uw          r[a0.3,16]<2;2,1>:uw    
               mov     (2|M0)                 f1.0<1>:uw          r[a0.3,20]<2;2,1>:uw    
               mov     (8|M0)                 a0.0<1>:uw          r17.0<8;8,1>:uw         
(~f0.0)        shl     (16|M0)                r[a0.0]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.1]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.2]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.3]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.0,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.1,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.2,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.3,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.4]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.5]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.6]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.7]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.4,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.5,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.6,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.7,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
               add     (8|M0)                 a0.0<1>:w           a0.0<8;8,1>:w           0x200:uw           
               mov     (8|M0)                 r17.0<1>:uw         a0.0<8;8,1>:uw          
               mov     (1|M0)                 a0.3<1>:uw          0x2A0:uw                
               mov     (2|M0)                 f0.0<1>:uw          r[a0.3,24]<2;2,1>:uw    
               mov     (2|M0)                 f1.0<1>:uw          r[a0.3,28]<2;2,1>:uw    
               mov     (8|M0)                 a0.0<1>:uw          r17.0<8;8,1>:uw         
(~f0.0)        shl     (16|M0)                r[a0.0]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.1]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.2]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f0.0)        shl     (16|M0)                r[a0.3]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.0,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.1,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.2,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f0.1)        shl     (16|M0)                r[a0.3,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.4]<1>:uw       r2.20<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.5]<1>:uw       r2.21<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.6]<1>:uw       r2.22<0;1,0>:ub         0x8:uw             
(~f1.0)        shl     (16|M0)                r[a0.7]<1>:uw       r2.23<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.4,32]<1>:uw    r2.20<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.5,32]<1>:uw    r2.21<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.6,32]<1>:uw    r2.22<0;1,0>:ub         0x8:uw             
(~f1.1)        shl     (16|M0)                r[a0.7,32]<1>:uw    r2.23<0;1,0>:ub         0x8:uw             
               add     (8|M0)                 a0.0<1>:w           a0.0<8;8,1>:w           0x200:uw           
               mov     (8|M0)                 r17.0<1>:uw         a0.0<8;8,1>:uw          
L1696:
               nop     
