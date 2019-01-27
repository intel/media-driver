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
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw                  
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
                mov      (2|M0)                 f0.0<1>:uw               r[a0.7]<2;2,1>:uw         
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,4]<2;2,1>:uw       
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L720        
L432:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L704                     L704                      
L528:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L704:
                endif    (16|M0)                L720                     
L720:
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
(W&~f0.1.any16h)jmpi     L1184       
L896:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,2]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L1168                    L1168                     
L992:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L1168:
                endif    (16|M0)                L1184                    
L1184:
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
(W&~f1.0.any16h)jmpi     L1648       
L1360:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,4]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L1632                    L1632                     
L1456:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L1632:
                endif    (16|M0)                L1648                    
L1648:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L2096       
L1808:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,6]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L2080                    L2080                     
L1904:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L2080:
                endif    (16|M0)                L2096                    
L2096:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x200:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
                mov      (2|M0)                 f0.0<1>:uw               r[a0.7,8]<2;2,1>:uw       
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,12]<2;2,1>:uw      
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L2592       
L2304:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,8]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L2576                    L2576                     
L2400:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L2576:
                endif    (16|M0)                L2592                    
L2592:
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
(W&~f0.1.any16h)jmpi     L3056       
L2768:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,10]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L3040                    L3040                     
L2864:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L3040:
                endif    (16|M0)                L3056                    
L3056:
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
(W&~f1.0.any16h)jmpi     L3520       
L3232:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,12]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L3504                    L3504                     
L3328:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L3504:
                endif    (16|M0)                L3520                    
L3520:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L3968       
L3680:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,14]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L3952                    L3952                     
L3776:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L3952:
                endif    (16|M0)                L3968                    
L3968:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x400:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
                mov      (2|M0)                 f0.0<1>:uw               r[a0.7,16]<2;2,1>:uw      
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,20]<2;2,1>:uw      
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L4464       
L4176:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,16]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L4448                    L4448                     
L4272:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L4448:
                endif    (16|M0)                L4464                    
L4464:
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
(W&~f0.1.any16h)jmpi     L4928       
L4640:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,18]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L4912                    L4912                     
L4736:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L4912:
                endif    (16|M0)                L4928                    
L4928:
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
(W&~f1.0.any16h)jmpi     L5392       
L5104:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,20]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L5376                    L5376                     
L5200:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L5376:
                endif    (16|M0)                L5392                    
L5392:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L5840       
L5552:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,22]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L5824                    L5824                     
L5648:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L5824:
                endif    (16|M0)                L5840                    
L5840:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x600:uw                
                mov      (16|M0)                r11.0<1>:uw              r[a0.3]<16;16,1>:uw       
                mov      (2|M0)                 f0.0<1>:uw               r[a0.7,24]<2;2,1>:uw      
                mov      (2|M0)                 f1.0<1>:uw               r[a0.7,28]<2;2,1>:uw      
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.1)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(f0.1)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L6336       
L6048:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,24]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.0)          if       (16|M0)                L6320                    L6320                     
L6144:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L6320:
                endif    (16|M0)                L6336                    
L6336:
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
(W&~f0.1.any16h)jmpi     L6800       
L6512:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,26]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f0.1)          if       (16|M0)                L6784                    L6784                     
L6608:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L6784:
                endif    (16|M0)                L6800                    
L6800:
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
(W&~f1.0.any16h)jmpi     L7264       
L6976:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,28]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.0)          if       (16|M0)                L7248                    L7248                     
L7072:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
                mul      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                 
L7248:
                endif    (16|M0)                L7264                    
L7264:
                mov      (16|M0)                r11.0<1>:uw              r[a0.3,32]<16;16,1>:uw    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                  
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L7712       
L7424:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,30]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                 
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw         
(f1.1)          if       (16|M0)                L7696                    L7696                     
L7520:
                add      (16|M0)                (sat)r17.0<1>:uw         r11.0<16;16,1>:uw         0x100:uw                
                add      (16|M0)                (sat)r16.0<1>:uw         -r11.0<16;16,1>:uw        0xFF00:uw               
                mul      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.0<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.1<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
                mul      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r17.0<16;16,1>:uw
                mac      (16|M0)                acc0.0<1>:w              r14.2<0;1,0>:uw           r16.0<16;16,1>:uw
                shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L7696:
                endif    (16|M0)                L7712                    
L7712:
                nop      
