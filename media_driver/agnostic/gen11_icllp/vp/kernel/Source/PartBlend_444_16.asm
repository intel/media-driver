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
         mov      (1|M0)                 r14.0<1>:uw              a0.3<0;1,0>:uw             
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3]<0;1,0>:uw          0x0:uw                 
(W&f0.0) jmpi     L400        
L48:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw        0x0:uw                 
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw             r[a0.3]<0;1,0>:uw
         mov      (8|M0)                 a0.0<1>:uw               r17.0<8;8,1>:uw            
(f0.0)   if       (16|M0)                L384                     L384                       
L112:
         add      (16|M0)                (sat)r16.0<1>:uw         -r[a0.7]<16;16,1>:uw       0xFF00:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.7]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw        0x1:uw                 
L384:
         endif    (16|M0)                L400                     
L400:
         mov      (1|M0)                 a0.3<1>:uw               r14.0<0;1,0>:uw            
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,2]<0;1,0>:uw        0x0:uw                 
(W&f0.0) jmpi     L800        
L448:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw     0x0:uw                 
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw             r[a0.3,2]<0;1,0>:uw
         mov      (8|M0)                 a0.0<1>:uw               r17.0<8;8,1>:uw            
(f0.0)   if       (16|M0)                L784                     L784                       
L512:
         add      (16|M0)                (sat)r16.0<1>:uw         -r[a0.7,32]<16;16,1>:uw    0xFF00:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.7,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw     0x1:uw
L784:
         endif    (16|M0)                L800                     
L800:
         add      (4|M0)                 r17.0<1>:ud              r17.0<4;4,1>:ud            r22.4<1;2,0>:ud        
         mov      (8|M0)                 a0.0<1>:uw               r17.0<8;8,1>:uw            
         mov      (1|M0)                 a0.3<1>:uw               r14.0<0;1,0>:uw            
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,4]<0;1,0>:uw        0x0:uw                 
(W&f0.0) jmpi     L1232       
L880:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw        0x0:uw                 
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw             r[a0.3,4]<0;1,0>:uw
         mov      (8|M0)                 a0.0<1>:uw               r17.0<8;8,1>:uw            
(f0.0)   if       (16|M0)                L1216                    L1216                      
L944:
         add      (16|M0)                (sat)r16.0<1>:uw         -r[a0.7]<16;16,1>:uw       0xFF00:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.0]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.1]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.2]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw        0x1:uw                 
         mul      (16|M0)                acc0.0<1>:w              r[a0.7]<16;16,1>:uw        0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.3]<16;16,1>:uw        r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.3]<1>:uw            acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw        0x1:uw                 
L1216:
         endif    (16|M0)                L1232                    
L1232:
         mov      (1|M0)                 a0.3<1>:uw               r14.0<0;1,0>:uw            
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,6]<0;1,0>:uw        0x0:uw                 
(W&f0.0) jmpi     L1632       
L1280:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw     0x0:uw                 
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw             r[a0.3,6]<0;1,0>:uw
         mov      (8|M0)                 a0.0<1>:uw               r17.0<8;8,1>:uw            
(f0.0)   if       (16|M0)                L1616                    L1616                      
L1344:
         add      (16|M0)                (sat)r16.0<1>:uw         -r[a0.7,32]<16;16,1>:uw    0xFF00:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.0,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.1,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.2,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw     0x1:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.7,32]<16;16,1>:uw     0xFFFF:uw              
         mac      (16|M0)                acc0.0<1>:w              r[a0.3,32]<16;16,1>:uw     r16.0<16;16,1>:uw
         shr      (16|M0)                r[a0.3,32]<1>:uw         acc0.0<16;16,1>:w          0x11:uw                
         shl      (16|M0)                (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw     0x1:uw
L1616:
         endif    (16|M0)                L1632                    
L1632:
         mov      (1|M0)                 a0.3<1>:uw               r14.0<0;1,0>:uw            
         nop      
