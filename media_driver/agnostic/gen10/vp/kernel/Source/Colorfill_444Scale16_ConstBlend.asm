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
                mov      (16|M0)                r11.0<1>:uw              0x0:uw                    
                mov      (16|M0)                r11.1<2>:ub              r2.23<0;1,0>:ub           
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7]<0;1,0>:uw         
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,2]<0;1,0>:uw       
                mov      (1|M0)                 f1.0<1>:uw               r[a0.7,4]<0;1,0>:uw       
                mov      (1|M0)                 f1.1<1>:uw               r[a0.7,6]<0;1,0>:uw       
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw                  
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L656        
L416:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7]<0;1,0>:uw
(f0.0)          if       (16|M0)                L640                     L640                      
L464:
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
L640:
                endif    (16|M0)                L656                     
L656:
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,2]<0;1,0>:uw       
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L992        
L752:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,2]<0;1,0>:uw
(f0.1)          if       (16|M0)                L976                     L976                      
L800:
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
L976:
                endif    (16|M0)                L992                     
L992:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L1328       
L1088:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,4]<0;1,0>:uw
(f1.0)          if       (16|M0)                L1312                    L1312                     
L1136:
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
L1312:
                endif    (16|M0)                L1328                    
L1328:
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L1648       
L1408:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,6]<0;1,0>:uw
(f1.1)          if       (16|M0)                L1632                    L1632                     
L1456:
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
L1632:
                endif    (16|M0)                L1648                    
L1648:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,8]<0;1,0>:uw       
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,10]<0;1,0>:uw      
                mov      (1|M0)                 f1.0<1>:uw               r[a0.7,12]<0;1,0>:uw      
                mov      (1|M0)                 f1.1<1>:uw               r[a0.7,14]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x200:uw                
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L2048       
L1808:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,8]<0;1,0>:uw
(f0.0)          if       (16|M0)                L2032                    L2032                     
L1856:
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
L2032:
                endif    (16|M0)                L2048                    
L2048:
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,10]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L2384       
L2144:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,10]<0;1,0>:uw
(f0.1)          if       (16|M0)                L2368                    L2368                     
L2192:
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
L2368:
                endif    (16|M0)                L2384                    
L2384:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L2720       
L2480:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,12]<0;1,0>:uw
(f1.0)          if       (16|M0)                L2704                    L2704                     
L2528:
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
L2704:
                endif    (16|M0)                L2720                    
L2720:
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L3040       
L2800:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,14]<0;1,0>:uw
(f1.1)          if       (16|M0)                L3024                    L3024                     
L2848:
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
L3024:
                endif    (16|M0)                L3040                    
L3040:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,16]<0;1,0>:uw      
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,18]<0;1,0>:uw      
                mov      (1|M0)                 f1.0<1>:uw               r[a0.7,20]<0;1,0>:uw      
                mov      (1|M0)                 f1.1<1>:uw               r[a0.7,22]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x400:uw                
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L3440       
L3200:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,16]<0;1,0>:uw
(f0.0)          if       (16|M0)                L3424                    L3424                     
L3248:
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
L3424:
                endif    (16|M0)                L3440                    
L3440:
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,18]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L3776       
L3536:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,18]<0;1,0>:uw
(f0.1)          if       (16|M0)                L3760                    L3760                     
L3584:
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
L3760:
                endif    (16|M0)                L3776                    
L3776:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L4112       
L3872:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,20]<0;1,0>:uw
(f1.0)          if       (16|M0)                L4096                    L4096                     
L3920:
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
L4096:
                endif    (16|M0)                L4112                    
L4112:
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L4432       
L4192:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,22]<0;1,0>:uw
(f1.1)          if       (16|M0)                L4416                    L4416                     
L4240:
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
L4416:
                endif    (16|M0)                L4432                    
L4432:
                mov      (1|M0)                 f0.0<1>:uw               r[a0.7,24]<0;1,0>:uw      
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,26]<0;1,0>:uw      
                mov      (1|M0)                 f1.0<1>:uw               r[a0.7,28]<0;1,0>:uw      
                mov      (1|M0)                 f1.1<1>:uw               r[a0.7,30]<0;1,0>:uw      
                add      (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x600:uw                
(~f0.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f0.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.0.any16h)jmpi     L4832       
L4592:
                cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.7,24]<0;1,0>:uw
(f0.0)          if       (16|M0)                L4816                    L4816                     
L4640:
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
L4816:
                endif    (16|M0)                L4832                    
L4832:
                mov      (1|M0)                 f0.1<1>:uw               r[a0.7,26]<0;1,0>:uw      
(~f0.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f0.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f0.1.any16h)jmpi     L5168       
L4928:
                cmp      (16|M0)    (ne)f0.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f0.1<1>:uw               f0.1<0;1,0>:uw            r[a0.7,26]<0;1,0>:uw
(f0.1)          if       (16|M0)                L5152                    L5152                     
L4976:
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
L5152:
                endif    (16|M0)                L5168                    
L5168:
                add      (2|M0)                 a0.0<1>:ud               a0.0<2;2,1>:ud            r22.4<0;1,0>:ud         
(~f1.0)         shl      (16|M0)                r[a0.0]<1>:uw            r2.20<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.1]<1>:uw            r2.21<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.2]<1>:uw            r2.22<0;1,0>:ub           0x8:uw                  
(~f1.0)         shl      (16|M0)                r[a0.3]<1>:uw            r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.0.any16h)jmpi     L5504       
L5264:
                cmp      (16|M0)    (ne)f1.0    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.0<1>:uw               f1.0<0;1,0>:uw            r[a0.7,28]<0;1,0>:uw
(f1.0)          if       (16|M0)                L5488                    L5488                     
L5312:
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
L5488:
                endif    (16|M0)                L5504                    
L5504:
(~f1.1)         shl      (16|M0)                r[a0.0,32]<1>:uw         r2.20<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.1,32]<1>:uw         r2.21<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.2,32]<1>:uw         r2.22<0;1,0>:ub           0x8:uw                  
(~f1.1)         shl      (16|M0)                r[a0.3,32]<1>:uw         r2.23<0;1,0>:ub           0x8:uw                  
(W&~f1.1.any16h)jmpi     L5824       
L5584:
                cmp      (16|M0)    (ne)f1.1    null.0<1>:w              r11.0<16;16,1>:uw         0x0:uw                  
                and      (1|M0)                 f1.1<1>:uw               f1.1<0;1,0>:uw            r[a0.7,30]<0;1,0>:uw
(f1.1)          if       (16|M0)                L5808                    L5808                     
L5632:
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
L5808:
                endif    (16|M0)                L5824                    
L5824:
                nop      
