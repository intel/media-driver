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
         and     (16|M0)    (ne)f0.0    null.0<1>:w    r2.3<0;1,0>:uw            1:w                
         add     (4|M0)                 a0.0<1>:w      r22.0<4;4,1>:w            0x0:uw             {AccWrEn}
(f0.0)   mov     (1|M0)                 r17.0<1>:uw    a0.0<0;1,0>:uw            
(f0.0)   mov     (1|M0)                 a0.0<1>:uw     a0.2<0;1,0>:uw            
(f0.0)   mov     (1|M0)                 a0.2<1>:uw     r17.0<0;1,0>:uw           
         add     (4|M0)                 a0.4<1>:w      a0.0<4;4,1>:w             r22.8<0;2,1>:w     
         shl     (1|M0)                 r27.0<1>:d     r7.0<0;1,0>:w             2:w                
         mov     (1|M0)                 r27.1<1>:d     r7.1<0;1,0>:w             
         mov     (1|M0)                 r27.2<1>:ud    0x7001F:ud                
         mov     (8|M0)                 r28.0<1>:ud    r27.0<8;8,1>:ud           
         mov     (8|M0)                 r37.0<1>:ud    r27.0<8;8,1>:ud           
         mov     (8|M0)                 r46.0<1>:ud    r27.0<8;8,1>:ud           
         mov     (8|M0)                 r55.0<1>:ud    r27.0<8;8,1>:ud           
         add     (1|M0)                 r37.0<1>:d     r27.0<0;1,0>:d            32:d               
         add     (1|M0)                 r46.1<1>:d     r27.1<0;1,0>:d            8:d                
         add     (1|M0)                 r55.0<1>:d     r27.0<0;1,0>:d            32:d               
         add     (1|M0)                 r55.1<1>:d     r27.1<0;1,0>:d            8:d                
         mov     (1|M0)                 f0.0<1>:uw     0x5555:uw                 
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r29.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r30.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r31.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r32.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r38.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r39.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r40.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r41.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         add     (8|M0)                 a0.0<1>:uw     a0.0<8;8,1>:uw            0x200:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r33.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r34.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r35.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r36.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r42.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r43.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r44.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r45.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         add     (8|M0)                 a0.0<1>:uw     a0.0<8;8,1>:uw            0x200:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r47.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r48.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r49.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r50.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r56.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r57.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r58.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r59.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         add     (8|M0)                 a0.0<1>:uw     a0.0<8;8,1>:uw            0x200:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r51.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r52.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.2,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.1,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.0,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.3,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r53.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r54.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,1]<32;16,2>:ub     
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,1]<32;16,2>:ub     0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,1]<32;16,2>:ub     
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,1]<32;16,2>:ub     0x100:uw           
(f0.0)   sel     (16|M0)                r60.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r61.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         mov     (16|M0)                acc0.0<1>:w    r[a0.6,33]<32;16,2>:ub    
         mac     (16|M0)                r16.0<1>:uw    r[a0.5,33]<32;16,2>:ub    0x100:uw           
         mov     (16|M0)                acc0.0<1>:w    r[a0.4,33]<32;16,2>:ub    
         mac     (16|M0)                r17.0<1>:uw    r[a0.7,33]<32;16,2>:ub    0x100:uw           
(f0.0)   sel     (16|M0)                r62.0<1>:uw    r16.0<1;2,0>:uw           r17.0<1;2,0>:uw    
(f0.0)   sel     (16|M0)                r63.0<1>:uw    r16.8<1;2,0>:uw           r17.8<1;2,0>:uw    
         send    (8|M0)                 null:d         r28:ub                    0xC                0x120A8018    
         send    (8|M0)                 null:d         r37:ub                    0xC                0x120A8018    
         send    (8|M0)                 null:d         r46:ub                    0xC                0x120A8018    
         send    (8|M0)                 null:d         r55:ub                    0xC                0x120A8018    
