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
                cmp      (1|M0)     (eq)f0.0    null.0<1>:d              r7.1<0;1,0>:ud            0xFFFFFFFF:ud           
                and      (16|M0)                r21.0<1>:uw              r21.0<16;16,1>:uw         0xFFFF:uw               
(W&f0.0)        jmpi     L192        
L48:
                mov      (1|M0)                 f0.0<1>:uw               r7.3<0;1,0>:uw            
                mov      (16|M0)                r16.0<1>:uw              0x0:uw                    
                mov      (8|M0)                 r17.0<1>:ub              r7.4<1;4,0>:ub            
(f0.0)          mov      (16|M0)                r16.0<1>:uw              0xFFFF:uw                 
                mov      (16|M0)                r16.0<1>:ub              r16.0<32;16,2>:ub         
                mov      (8|M0)                 r16.0<1>:ud              r16.0<1;2,0>:ud           
                mov      (8|M0)                 r17.0<1>:ud              r17.0<0;2,1>:ud           
                and      (8|M0)                 acc0.0<1>:d              r17.0<8;8,1>:ud           r16.0<8;8,1>:ud         
                and      (8|M0)                 r21.0<1>:ud              acc0.0<8;8,1>:d           r21.0<8;8,1>:ud         
L192:
                shl      (4|M0)                 r14.0<1>:uw              r2.20<4;4,1>:ub           0x8:uw                  
                mov      (1|M0)                 a0.7<1>:uw               0x2A0:uw                  
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7]<0;1,0>:uw         
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,4]<0;1,0>:uw       
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw                  
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L832        
L448:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L816                     L816                      
L544:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L816:
                endif    (16|M0)                L832                     
L832:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,2]<0;1,0>:uw       
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L1408       
L1024:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,2]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L1392                    L1392                     
L1120:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L1392:
                endif    (16|M0)                L1408                    
L1408:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L1984       
L1600:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,4]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L1968                    L1968                     
L1696:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L1968:
                endif    (16|M0)                L1984                    
L1984:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L2544       
L2160:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,6]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L2528                    L2528                     
L2256:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L2528:
                endif    (16|M0)                L2544                    
L2544:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,8]<0;1,0>:uw       
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,12]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x200:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L3152       
L2768:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,8]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L3136                    L3136                     
L2864:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L3136:
                endif    (16|M0)                L3152                    
L3152:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,10]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L3728       
L3344:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,10]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L3712                    L3712                     
L3440:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L3712:
                endif    (16|M0)                L3728                    
L3728:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L4304       
L3920:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,12]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L4288                    L4288                     
L4016:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L4288:
                endif    (16|M0)                L4304                    
L4304:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L4864       
L4480:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,14]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L4848                    L4848                     
L4576:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L4848:
                endif    (16|M0)                L4864                    
L4864:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,16]<0;1,0>:uw      
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,20]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x400:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L5472       
L5088:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,16]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L5456                    L5456                     
L5184:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L5456:
                endif    (16|M0)                L5472                    
L5472:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,18]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L6048       
L5664:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,18]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L6032                    L6032                     
L5760:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L6032:
                endif    (16|M0)                L6048                    
L6048:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L6624       
L6240:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,20]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L6608                    L6608                     
L6336:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L6608:
                endif    (16|M0)                L6624                    
L6624:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L7184       
L6800:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,22]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L7168                    L7168                     
L6896:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L7168:
                endif    (16|M0)                L7184                    
L7184:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,24]<0;1,0>:uw      
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,28]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x600:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L7792       
L7408:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,24]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L7776                    L7776                     
L7504:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L7776:
                endif    (16|M0)                L7792                    
L7792:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,26]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L8368       
L7984:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,26]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L8352                    L8352                     
L8080:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L8352:
                endif    (16|M0)                L8368                    
L8368:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L8944       
L8560:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,28]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L8928                    L8928                     
L8656:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x1:uw                  
                mul      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw       0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x1:uw                  
L8928:
                endif    (16|M0)                L8944                    
L8944:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L9504       
L9120:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,30]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L9488                    L9488                     
L9216:
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x1:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw    0xFFFF:uw               
                mac      (16|M0)                acc0.0<1>:w              r14.3<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w         0x11:uw                 
                shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x1:uw
L9488:
                endif    (16|M0)                L9504                    
L9504:
                nop      
