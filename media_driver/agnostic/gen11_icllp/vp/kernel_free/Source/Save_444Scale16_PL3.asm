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
         add     (4|M0)                 a0.0<1>:w           r22.0<4;4,1>:w            0x0:uw                   
         mov     (8|M0)                 r28.0<1>:ud         r0.0<8;8,1>:ud            
         mov     (8|M0)                 r37.0<1>:ud         r0.0<8;8,1>:ud            
         mov     (8|M0)                 r46.0<1>:ud         r0.0<8;8,1>:ud            
         mov     (2|M0)                 r28.0<1>:d          r7.0<2;2,1>:w             
         shr     (2|M0)                 r37.0<1>:d          r7.0<2;2,1>:w             1:w                      
         shr     (2|M0)                 r46.0<1>:d          r7.0<2;2,1>:w             1:w                      
         mov     (1|M0)                 r28.2<1>:ud         0xF000F:ud                
         mov     (1|M0)                 r37.2<1>:ud         0x70007:ud                
         mov     (1|M0)                 r46.2<1>:ud         0x70007:ud                
         add     (4|M0)                 a0.4<1>:w           a0.0<4;4,1>:w             r22.8<0;2,1>:w           
         and     (1|M0)                 r17.0<1>:uw         r2.5<0;1,0>:uw            0x700:uw                 
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x0:uw                   
(W&f1.0) jmpi    L1696       
L224:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x200:uw                 
(W&f1.0) jmpi    L2912       
L256:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x300:uw                 
(W&f1.0) jmpi    L4128       
L288:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x400:uw                 
(W&f1.0) jmpi    L5216       
L320:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x500:uw                 
(W&f1.0) jmpi    L6432       
L352:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r38.0<1>:ub         r50.1<16;4,2>:ub          
         mov     (8|M0)                 r38.8<1>:ub         r50.9<16;4,2>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r47.0<1>:ub         r50.1<16;4,2>:ub          
         mov     (8|M0)                 r47.8<1>:ub         r50.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r38.16<1>:ub        r50.1<16;4,2>:ub          
         mov     (8|M0)                 r38.24<1>:ub        r50.9<16;4,2>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r47.16<1>:ub        r50.1<16;4,2>:ub          
         mov     (8|M0)                 r47.24<1>:ub        r50.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r39.0<1>:ub         r50.1<16;4,2>:ub          
         mov     (8|M0)                 r39.8<1>:ub         r50.9<16;4,2>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r48.0<1>:ub         r50.1<16;4,2>:ub          
         mov     (8|M0)                 r48.8<1>:ub         r50.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r39.16<1>:ub        r50.1<16;4,2>:ub          
         mov     (8|M0)                 r39.24<1>:ub        r50.9<16;4,2>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r50.0<1>:uw         r52.0<16;8,2>:uw          r52.1<16;8,2>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r50.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r48.16<1>:ub        r50.1<16;4,2>:ub          
         mov     (8|M0)                 r48.24<1>:ub        r50.9<16;4,2>:ub          
(W)      jmpi    L7504       
L1696:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<1>:ub         r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.0<1>:ub         r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.16<1>:ub        r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.16<1>:ub        r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<1>:ub         r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.0<1>:ub         r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.16<1>:ub        r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.16<1>:ub        r50.1<32;8,4>:ub          
(W)      jmpi    L7504       
L2912:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<1>:ub         r50.1<32;16,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.0<1>:ub         r50.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.16<1>:ub        r50.1<32;16,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.16<1>:ub        r50.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<1>:ub         r50.1<32;16,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.0<1>:ub         r50.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.16<1>:ub        r50.1<32;16,2>:ub         
         avg     (8|M0)                 r52.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r53.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r50.0<1>:uw    r52.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r50.8<1>:uw    r52.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.16<1>:ub        r50.1<32;16,2>:ub         
(W)      jmpi    L7504       
L4128:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r38.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r38.8<1>:ub         r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r47.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r47.8<1>:ub         r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r38.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r38.24<1>:ub        r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r47.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r47.24<1>:ub        r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r39.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r39.8<1>:ub         r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r48.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r48.8<1>:ub         r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r39.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r39.24<1>:ub        r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r48.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r48.24<1>:ub        r50.17<32;4,4>:ub         
(W)      jmpi    L7504       
L5216:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<1>:ub         r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.0<1>:ub         r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.16<1>:ub        r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r47.16<1>:ub        r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<1>:ub         r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.0<1>:ub         r50.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         avg     (16|M0)                r52.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.16<1>:ub        r50.1<32;8,4>:ub          
         avg     (16|M0)                r52.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r53.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r50.0<1>:uw    r52.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r51.0<1>:uw    r52.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r48.16<1>:ub        r50.1<32;8,4>:ub          
(W)      jmpi    L7504       
L6432:
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r38.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r38.8<1>:ub         r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r47.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r47.8<1>:ub         r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r38.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r38.24<1>:ub        r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r47.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r47.24<1>:ub        r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r39.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r39.8<1>:ub         r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r48.0<1>:ub         r50.1<32;4,4>:ub          
         mov     (8|M0)                 r48.8<1>:ub         r50.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r52.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r53.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r50.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r50.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r52.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r52.17<32;8,2>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r39.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r39.24<1>:ub        r50.17<32;4,4>:ub         
         add     (16|M0)                (sat)r50.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r51.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r48.16<1>:ub        r50.1<32;4,4>:ub          
         mov     (8|M0)                 r48.24<1>:ub        r50.17<32;4,4>:ub         
L7504:
         send    (1|M0)                 null:d              r28:ub                    0xC                      0x120A8018    
         send    (1|M0)                 null:d              r37:ub                    0xC                      0x060A8019    
         send    (1|M0)                 null:d              r46:ub                    0xC                      0x060A801A    
