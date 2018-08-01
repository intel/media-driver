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
         mov     (8|M0)                 r28.0<1>:ud         r27.0<8;8,1>:ud           
         mov     (8|M0)                 r37.0<1>:ud         r27.0<8;8,1>:ud           
         mov     (2|M0)                 r28.0<1>:d          r7.0<2;2,1>:w             
         mov     (1|M0)                 r37.0<1>:d          r7.0<0;1,0>:w             
         shr     (1|M0)                 r37.1<1>:d          r7.1<0;1,0>:w             1:w                      
         mov     (1|M0)                 r28.2<1>:ud         0xF000F:ud                
         mov     (1|M0)                 r37.2<1>:ud         0x7000F:ud                
         add     (4|M0)                 a0.4<1>:w           a0.0<4;4,1>:w             r22.8<0;2,1>:w           
         and     (1|M0)                 r17.0<1>:uw         r2.5<0;1,0>:uw            0x700:uw                 
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x0:uw                   
(W&f1.0) jmpi    L352        
L192:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x100:uw                 
(W&f1.0) jmpi    L1584       
L224:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x200:uw                 
(W&f1.0) jmpi    L2944       
L256:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x300:uw                 
(W&f1.0) jmpi    L4176       
L288:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x400:uw                 
(W&f1.0) jmpi    L5280       
L320:
         cmp     (1|M0)     (eq)f1.0    null.0<1>:uw        r17.0<0;1,0>:uw           0x500:uw                 
(W&f1.0) jmpi    L6512       
L352:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,2]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,2]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
(W)      jmpi    L7600       
L1584:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r38.0<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r38.16<2>:ub        r46.9<16;4,2>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r38.1<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r38.17<2>:ub        r46.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r39.0<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r39.16<2>:ub        r46.9<16;4,2>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r39.1<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r39.17<2>:ub        r46.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r40.0<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r40.16<2>:ub        r46.9<16;4,2>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r40.1<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r40.17<2>:ub        r46.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r41.0<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r41.16<2>:ub        r46.9<16;4,2>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         avg     (16|M0)                r46.0<1>:uw         r48.0<16;8,2>:uw          r48.1<16;8,2>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r46.0<16;16,1>:uw         0x80:uw                  
         mov     (8|M0)                 r41.1<2>:ub         r46.1<16;4,2>:ub          
         mov     (8|M0)                 r41.17<2>:ub        r46.9<16;4,2>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
(W)      jmpi    L7600       
L2944:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<2>:ub         r48.1<32;16,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.1<2>:ub         r48.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<2>:ub         r48.1<32;16,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.1<2>:ub         r48.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.0<2>:ub         r48.1<32;16,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.1<2>:ub         r48.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.2,16]<16;4,2>:uw     r[a0.2,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.6,16]<16;4,2>:uw     r[a0.6,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.0<2>:ub         r48.1<32;16,2>:ub         
         avg     (8|M0)                 r46.0<1>:uw         r[a0.0,16]<16;4,2>:uw     r[a0.0,18]<16;4,2>:uw
         avg     (8|M0)                 r47.0<1>:uw         r[a0.4,16]<16;4,2>:uw     r[a0.4,18]<16;4,2>:uw
         add     (8|M0)                 (sat)r48.0<1>:uw    r46.0<16;4,1>:uw          0x80:uw                  
         add     (8|M0)                 (sat)r48.8<1>:uw    r46.4<16;4,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.1<2>:ub         r48.1<32;16,2>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
(W)      jmpi    L7600       
L4176:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r38.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r38.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r38.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r38.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r39.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r39.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r39.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r39.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r40.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r40.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r40.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r40.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r41.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r41.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0]<16;8,1>:uw        0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4]<16;8,1>:uw        0x80:uw                  
         mov     (8|M0)                 r41.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r41.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
(W)      jmpi    L7600       
L5280:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r38.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r39.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r40.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         avg     (16|M0)                r48.0<1>:uw         r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.0<2>:ub         r46.1<32;8,4>:ub          
         avg     (16|M0)                r48.0<1>:uw         r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)                r49.0<1>:uw         r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)                (sat)r46.0<1>:uw    r48.0<16;8,1>:uw          0x80:uw                  
         add     (16|M0)                (sat)r47.0<1>:uw    r48.8<16;8,1>:uw          0x80:uw                  
         mov     (16|M0)                r41.1<2>:ub         r46.1<32;8,4>:ub          
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
(W)      jmpi    L7600       
L6512:
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r29.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r29.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r30.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r30.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r38.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r38.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r38.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r38.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r31.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r31.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r32.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r32.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r39.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r39.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r39.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r39.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r33.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r33.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r34.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r34.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r40.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r40.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r40.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r40.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r48.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.5]<16;16,1>:uw       0x80:uw                  
         add     (16|M0)                (sat)r49.0<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         mov     (16|M0)                r35.0<1>:ub         r46.1<32;8,2>:ub          
         mov     (16|M0)                r35.16<1>:ub        r46.17<32;8,2>:ub         
         mov     (16|M0)                r36.0<1>:ub         r48.1<32;8,2>:ub          
         mov     (16|M0)                r36.16<1>:ub        r48.17<32;8,2>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.2,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.6,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r41.0<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r41.16<2>:ub        r46.17<32;4,4>:ub         
         add     (16|M0)                (sat)r46.0<1>:uw    r[a0.0,16]<16;8,1>:uw     0x80:uw
         add     (16|M0)                (sat)r47.0<1>:uw    r[a0.4,16]<16;8,1>:uw     0x80:uw
         mov     (8|M0)                 r41.1<2>:ub         r46.1<32;4,4>:ub          
         mov     (8|M0)                 r41.17<2>:ub        r46.17<32;4,4>:ub         
         add     (8|M0)                 a0.0<1>:w           a0.0<4;4,1>:w             0x200:uw                 
L7600:
         add     (1|M0)                 a0.0<1>:ud          r2.3<0;1,0>:ub            0x120A8000:ud            
         send    (1|M0)                 null:d              r28:ub                    0xC                      a0.0    
         add     (1|M0)                 a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8001:ud             
         send    (1|M0)                 null:d              r37:ub                    0xC                      a0.0    
         nop     
