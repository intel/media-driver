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
(W&f0.0) jmpi     L304        
L32:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw       0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3]<0;1,0>:uw
(f0.0)   if       (16|M0)                L288                     L288                      
L80:
         shr      (16|M0)                null.0<1>:uw             r[a0.7]<16;16,1>:uw       0x4:uw                    {AccWrEn}
         add      (16|M0)                null.0<1>:uw             -acc0.0<16;16,1>:w        r[a0.7]<16;16,1>:uw       {AccWrEn}
         add      (16|M0)                (sat)r17.0<1>:uw         acc0.0<16;16,1>:w         0x1000:uw                 {AccWrEn}
         add      (16|M0)                r16.0<1>:uw              -acc0.0<16;16,1>:w        0x0:uw                    
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
L288:
         endif    (16|M0)                L304                     
L304:
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,2]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L608        
L336:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw    0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,2]<0;1,0>:uw
(f0.0)   if       (16|M0)                L592                     L592                      
L384:
         shr      (16|M0)                null.0<1>:uw             r[a0.7,32]<16;16,1>:uw    0x4:uw                    {AccWrEn}
         add      (16|M0)                null.0<1>:uw             -acc0.0<16;16,1>:w        r[a0.7,32]<16;16,1>:uw    {AccWrEn}
         add      (16|M0)                (sat)r17.0<1>:uw         acc0.0<16;16,1>:w         0x1000:uw                 {AccWrEn}
         add      (16|M0)                r16.0<1>:uw              -acc0.0<16;16,1>:w        0x0:uw                    
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L592:
         endif    (16|M0)                L608                     
L608:
         add      (4|M0)                 a0.0<1>:ud               a0.0<4;4,1>:ud            r22.4<1;2,0>:ud           
         add      (1|M0)                 a0.3<1>:w                a0.3<0;1,0>:w             -r22.9<0;1,0>:w           
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,4]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L944        
L672:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7]<16;16,1>:uw       0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,4]<0;1,0>:uw
(f0.0)   if       (16|M0)                L928                     L928                      
L720:
         shr      (16|M0)                null.0<1>:uw             r[a0.7]<16;16,1>:uw       0x4:uw                    {AccWrEn}
         add      (16|M0)                null.0<1>:uw             -acc0.0<16;16,1>:w        r[a0.7]<16;16,1>:uw       {AccWrEn}
         add      (16|M0)                (sat)r17.0<1>:uw         acc0.0<16;16,1>:w         0x1000:uw                 {AccWrEn}
         add      (16|M0)                r16.0<1>:uw              -acc0.0<16;16,1>:w        0x0:uw                    
         mul      (16|M0)                acc0.0<1>:w              r[a0.4]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.5]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
         mul      (16|M0)                acc0.0<1>:w              r[a0.6]<16;16,1>:uw       r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2]<16;16,1>:uw       r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2]<1>:uw       acc0.0<16;16,1>:w         0x10:uw                   
L928:
         endif    (16|M0)                L944                     
L944:
         cmp      (1|M0)     (eq)f0.0    null.0<1>:w              r[a0.3,6]<0;1,0>:uw       0x0:uw                    
(W&f0.0) jmpi     L1248       
L976:
         cmp      (16|M0)    (ne)f0.0    null.0<1>:w              r[a0.7,32]<16;16,1>:uw    0x0:uw                    
         and      (1|M0)                 f0.0<1>:uw               f0.0<0;1,0>:uw            r[a0.3,6]<0;1,0>:uw
(f0.0)   if       (16|M0)                L1232                    L1232                     
L1024:
         shr      (16|M0)                null.0<1>:uw             r[a0.7,32]<16;16,1>:uw    0x4:uw                    {AccWrEn}
         add      (16|M0)                null.0<1>:uw             -acc0.0<16;16,1>:w        r[a0.7,32]<16;16,1>:uw    {AccWrEn}
         add      (16|M0)                (sat)r17.0<1>:uw         acc0.0<16;16,1>:w         0x1000:uw                 {AccWrEn}
         add      (16|M0)                r16.0<1>:uw              -acc0.0<16;16,1>:w        0x0:uw                    
         mul      (16|M0)                acc0.0<1>:w              r[a0.4,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.0,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.0,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.5,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.1,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.1,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
         mul      (16|M0)                acc0.0<1>:w              r[a0.6,32]<16;16,1>:uw    r17.0<16;16,1>:uw
         mac      (16|M0)                acc0.0<1>:w              r[a0.2,32]<16;16,1>:uw    r16.0<16;16,1>:uw
         shr      (16|M0)                (sat)r[a0.2,32]<1>:uw    acc0.0<16;16,1>:w         0x10:uw
L1232:
         endif    (16|M0)                L1248                    
L1248:
         nop      
