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
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3]<0;1,0>:uw         0x0:uw                    
(W&f0.0) jmpi     L320        
L32:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw       0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3]<0;1,0>:uw
(f0.0)   if       (16|M0)                L304                     L304                      
L80:
         add      (16|M0)                r17.0<1>:uw              r24.1<0;1,0>:ub           0x1:uw                    
         mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.7]<16;16,1>:uw
         shr      (16|M0)                r17.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
         add      (16|M0)                (sat)r16.0<1>:uw         -r17.0<16;16,1>:uw        0xFF00:uw                 
         add      (16|M0)                (sat)r17.0<1>:uw         r17.0<16;16,1>:uw         0x100:uw                  
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
L304:
         endif    (16|M0)                L320                     
L320:
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,2]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L640        
L352:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw    0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,2]<0;1,0>:uw
(f0.0)   if       (16|M0)                L624                     L624                      
L400:
         add      (16|M0)                r17.0<1>:uw              r24.1<0;1,0>:ub           0x1:uw                    
         mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.7,32]<16;16,1>:uw
         shr      (16|M0)                r17.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
         add      (16|M0)                (sat)r16.0<1>:uw         -r17.0<16;16,1>:uw        0xFF00:uw                 
         add      (16|M0)                (sat)r17.0<1>:uw         r17.0<16;16,1>:uw         0x100:uw                  
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L624:
         endif    (16|M0)                L640                     
L640:
         add      (4|M0)                 a0.0<1>:ud               a0.0<4;4,1>:ud            r22.4<1;2,0>:ud           
         add      (1|M0)                 a0.3<1>:w                a0.3<0;1,0>:w             -r22.9<0;1,0>:w           
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,4]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L992        
L704:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw       0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,4]<0;1,0>:uw
(f0.0)   if       (16|M0)                L976                     L976                      
L752:
         add      (16|M0)                r17.0<1>:uw              r24.1<0;1,0>:ub           0x1:uw                    
         mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.7]<16;16,1>:uw
         shr      (16|M0)                r17.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
         add      (16|M0)                (sat)r16.0<1>:uw         -r17.0<16;16,1>:uw        0xFF00:uw                 
         add      (16|M0)                (sat)r17.0<1>:uw         r17.0<16;16,1>:uw         0x100:uw                  
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
L976:
         endif    (16|M0)                L992                     
L992:
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,6]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L1312       
L1024:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw    0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,6]<0;1,0>:uw
(f0.0)   if       (16|M0)                L1296                    L1296                     
L1072:
         add      (16|M0)                r17.0<1>:uw              r24.1<0;1,0>:ub           0x1:uw                    
         mul      (16|M0)                acc0.0<1>:w              r17.0<16;16,1>:uw         r[a0.7,32]<16;16,1>:uw
         shr      (16|M0)                r17.0<1>:uw              acc0.0<16;16,1>:w         0x8:uw                    
         add      (16|M0)                (sat)r16.0<1>:uw         -r17.0<16;16,1>:uw        0xFF00:uw                 
         add      (16|M0)                (sat)r17.0<1>:uw         r17.0<16;16,1>:uw         0x100:uw                  
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L1296:
         endif    (16|M0)                L1312                    
L1312:
         nop      
