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
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
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
(W&~f0.0.any16h)jmpi     L752        
L464:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.0)          if       (16|M0)                L736                     L736                      
L560:
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
L736:
                endif    (16|M0)                L752                     
L752:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,2]<0;1,0>:uw       
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f0.1.any16h)jmpi     L1248       
L960:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,2]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.1)          if       (16|M0)                L1232                    L1232                     
L1056:
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
L1232:
                endif    (16|M0)                L1248                    
L1248:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud           
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.0.any16h)jmpi     L1744       
L1456:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,4]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.0)          if       (16|M0)                L1728                    L1728                     
L1552:
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
L1728:
                endif    (16|M0)                L1744                    
L1744:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.1.any16h)jmpi     L2224       
L1936:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,6]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.1)          if       (16|M0)                L2208                    L2208                     
L2032:
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
L2208:
                endif    (16|M0)                L2224                    
L2224:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x200:uw                  
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
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
(W&~f0.0.any16h)jmpi     L2752       
L2464:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,8]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.0)          if       (16|M0)                L2736                    L2736                     
L2560:
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
L2736:
                endif    (16|M0)                L2752                    
L2752:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,10]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f0.1.any16h)jmpi     L3248       
L2960:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,10]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.1)          if       (16|M0)                L3232                    L3232                     
L3056:
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
L3232:
                endif    (16|M0)                L3248                    
L3248:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud           
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.0.any16h)jmpi     L3744       
L3456:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,12]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.0)          if       (16|M0)                L3728                    L3728                     
L3552:
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
L3728:
                endif    (16|M0)                L3744                    
L3744:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.1.any16h)jmpi     L4224       
L3936:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,14]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.1)          if       (16|M0)                L4208                    L4208                     
L4032:
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
L4208:
                endif    (16|M0)                L4224                    
L4224:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x400:uw                  
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
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
(W&~f0.0.any16h)jmpi     L4752       
L4464:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,16]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.0)          if       (16|M0)                L4736                    L4736                     
L4560:
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
L4736:
                endif    (16|M0)                L4752                    
L4752:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,18]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f0.1.any16h)jmpi     L5248       
L4960:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,18]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.1)          if       (16|M0)                L5232                    L5232                     
L5056:
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
L5232:
                endif    (16|M0)                L5248                    
L5248:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud           
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.0.any16h)jmpi     L5744       
L5456:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,20]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.0)          if       (16|M0)                L5728                    L5728                     
L5552:
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
L5728:
                endif    (16|M0)                L5744                    
L5744:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.1.any16h)jmpi     L6224       
L5936:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,22]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.1)          if       (16|M0)                L6208                    L6208                     
L6032:
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
L6208:
                endif    (16|M0)                L6224                    
L6224:
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x600:uw                  
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
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
(W&~f0.0.any16h)jmpi     L6752       
L6464:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,24]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.1<0;1,0>:uw            
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.0)          if       (16|M0)                L6736                    L6736                     
L6560:
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
L6736:
                endif    (16|M0)                L6752                    
L6752:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,26]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f0.1.any16h)jmpi     L7248       
L6960:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,26]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f0.1)          if       (16|M0)                L7232                    L7232                     
L7056:
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
L7232:
                endif    (16|M0)                L7248                    
L7248:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud           
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.0.any16h)jmpi     L7744       
L7456:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,28]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.0)          if       (16|M0)                L7728                    L7728                     
L7552:
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
L7728:
                endif    (16|M0)                L7744                    
L7744:
                add      (16|M0)                r17.0<1>:uw              r2.23<0;1,0>:ub           0x1:uw                    
                mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.3,32]<16;16,1>:uw
                shr      (16|M0)                r11.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                    
                cmp      (16|M0)    (eq)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0x0:uw                    
(f0.0)          shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                    
(f0.0)          shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                    
(W&~f1.1.any16h)jmpi     L8224       
L7936:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                    
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,30]<0;1,0>:uw
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.1<32;16,2>:ub         0xFF:uw                   
                mov      (1|M0)                 r14.4<1>:uw              f0.0<0;1,0>:uw            
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r14.4<0;1,0>:uw           
(f1.1)          if       (16|M0)                L8208                    L8208                     
L8032:
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
L8208:
                endif    (16|M0)                L8224                    
L8224:
                nop      
