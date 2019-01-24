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
         add     (4|M0)    a0.0<1>:w      r22.0<4;4,1>:w           0x0:uw                {AccWrEn}
         mov     (8|M0)    r27.0<1>:ud    r0.0<8;8,1>:ud           
         shl     (2|M0)    r27.0<1>:d     r7.0<2;2,1>:w            0x2:v             
         mov     (1|M0)    r27.2<1>:ud    0xF000F:ud               
         add     (4|M0)    a0.4<1>:w      a0.0<4;4,1>:w            r22.8<0;2,1>:w    
         mov     (8|M0)    r29.0<4>:ub    r[a0.0,1]<16;4,2>:ub     
         mov     (8|M0)    r29.1<4>:ub    r[a0.2,1]<16;4,2>:ub     
         mov     (8|M0)    r29.2<4>:ub    r[a0.1,1]<16;4,2>:ub     
         mov     (8|M0)    r29.3<4>:ub    r[a0.3,1]<16;4,2>:ub     
         mov     (8|M0)    r30.0<4>:ub    r[a0.0,33]<16;4,2>:ub    
         mov     (8|M0)    r30.1<4>:ub    r[a0.2,33]<16;4,2>:ub    
         mov     (8|M0)    r30.2<4>:ub    r[a0.1,33]<16;4,2>:ub    
         mov     (8|M0)    r30.3<4>:ub    r[a0.3,33]<16;4,2>:ub    
         mov     (8|M0)    r38.0<4>:ub    r[a0.0,9]<16;4,2>:ub     
         mov     (8|M0)    r38.1<4>:ub    r[a0.2,9]<16;4,2>:ub     
         mov     (8|M0)    r38.2<4>:ub    r[a0.1,9]<16;4,2>:ub     
         mov     (8|M0)    r38.3<4>:ub    r[a0.3,9]<16;4,2>:ub     
         mov     (8|M0)    r39.0<4>:ub    r[a0.0,41]<16;4,2>:ub    
         mov     (8|M0)    r39.1<4>:ub    r[a0.2,41]<16;4,2>:ub    
         mov     (8|M0)    r39.2<4>:ub    r[a0.1,41]<16;4,2>:ub    
         mov     (8|M0)    r39.3<4>:ub    r[a0.3,41]<16;4,2>:ub    
         add     (4|M0)    a0.0<1>:w      a0.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)    r47.0<4>:ub    r[a0.4,1]<16;4,2>:ub     
         mov     (8|M0)    r47.1<4>:ub    r[a0.6,1]<16;4,2>:ub     
         mov     (8|M0)    r47.2<4>:ub    r[a0.5,1]<16;4,2>:ub     
         mov     (8|M0)    r47.3<4>:ub    r[a0.7,1]<16;4,2>:ub     
         mov     (8|M0)    r48.0<4>:ub    r[a0.4,33]<16;4,2>:ub    
         mov     (8|M0)    r48.1<4>:ub    r[a0.6,33]<16;4,2>:ub    
         mov     (8|M0)    r48.2<4>:ub    r[a0.5,33]<16;4,2>:ub    
         mov     (8|M0)    r48.3<4>:ub    r[a0.7,33]<16;4,2>:ub    
         mov     (8|M0)    r56.0<4>:ub    r[a0.4,9]<16;4,2>:ub     
         mov     (8|M0)    r56.1<4>:ub    r[a0.6,9]<16;4,2>:ub     
         mov     (8|M0)    r56.2<4>:ub    r[a0.5,9]<16;4,2>:ub     
         mov     (8|M0)    r56.3<4>:ub    r[a0.7,9]<16;4,2>:ub     
         mov     (8|M0)    r57.0<4>:ub    r[a0.4,41]<16;4,2>:ub    
         mov     (8|M0)    r57.1<4>:ub    r[a0.6,41]<16;4,2>:ub    
         mov     (8|M0)    r57.2<4>:ub    r[a0.5,41]<16;4,2>:ub    
         mov     (8|M0)    r57.3<4>:ub    r[a0.7,41]<16;4,2>:ub    
         add     (4|M0)    a0.4<1>:w      a0.4<4;4,1>:w            0x200:uw          
         mov     (8|M0)    r28.0<1>:ud    r27.0<8;8,1>:ud          
         mov     (8|M0)    r37.0<1>:ud    r27.0<8;8,1>:ud          
         mov     (8|M0)    r46.0<1>:ud    r27.0<8;8,1>:ud          
         mov     (8|M0)    r55.0<1>:ud    r27.0<8;8,1>:ud          
         mov     (8|M0)    r31.0<4>:ub    r[a0.0,1]<16;4,2>:ub     
         mov     (8|M0)    r31.1<4>:ub    r[a0.2,1]<16;4,2>:ub     
         mov     (8|M0)    r31.2<4>:ub    r[a0.1,1]<16;4,2>:ub     
         mov     (8|M0)    r31.3<4>:ub    r[a0.3,1]<16;4,2>:ub     
         mov     (8|M0)    r32.0<4>:ub    r[a0.0,33]<16;4,2>:ub    
         mov     (8|M0)    r32.1<4>:ub    r[a0.2,33]<16;4,2>:ub    
         mov     (8|M0)    r32.2<4>:ub    r[a0.1,33]<16;4,2>:ub    
         mov     (8|M0)    r32.3<4>:ub    r[a0.3,33]<16;4,2>:ub    
         mov     (8|M0)    r40.0<4>:ub    r[a0.0,9]<16;4,2>:ub     
         mov     (8|M0)    r40.1<4>:ub    r[a0.2,9]<16;4,2>:ub     
         mov     (8|M0)    r40.2<4>:ub    r[a0.1,9]<16;4,2>:ub     
         mov     (8|M0)    r40.3<4>:ub    r[a0.3,9]<16;4,2>:ub     
         mov     (8|M0)    r41.0<4>:ub    r[a0.0,41]<16;4,2>:ub    
         mov     (8|M0)    r41.1<4>:ub    r[a0.2,41]<16;4,2>:ub    
         mov     (8|M0)    r41.2<4>:ub    r[a0.1,41]<16;4,2>:ub    
         mov     (8|M0)    r41.3<4>:ub    r[a0.3,41]<16;4,2>:ub    
         add     (4|M0)    a0.0<1>:w      a0.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)    r49.0<4>:ub    r[a0.4,1]<16;4,2>:ub     
         mov     (8|M0)    r49.1<4>:ub    r[a0.6,1]<16;4,2>:ub     
         mov     (8|M0)    r49.2<4>:ub    r[a0.5,1]<16;4,2>:ub     
         mov     (8|M0)    r49.3<4>:ub    r[a0.7,1]<16;4,2>:ub     
         mov     (8|M0)    r50.0<4>:ub    r[a0.4,33]<16;4,2>:ub    
         mov     (8|M0)    r50.1<4>:ub    r[a0.6,33]<16;4,2>:ub    
         mov     (8|M0)    r50.2<4>:ub    r[a0.5,33]<16;4,2>:ub    
         mov     (8|M0)    r50.3<4>:ub    r[a0.7,33]<16;4,2>:ub    
         mov     (8|M0)    r58.0<4>:ub    r[a0.4,9]<16;4,2>:ub     
         mov     (8|M0)    r58.1<4>:ub    r[a0.6,9]<16;4,2>:ub     
         mov     (8|M0)    r58.2<4>:ub    r[a0.5,9]<16;4,2>:ub     
         mov     (8|M0)    r58.3<4>:ub    r[a0.7,9]<16;4,2>:ub     
         mov     (8|M0)    r59.0<4>:ub    r[a0.4,41]<16;4,2>:ub    
         mov     (8|M0)    r59.1<4>:ub    r[a0.6,41]<16;4,2>:ub    
         mov     (8|M0)    r59.2<4>:ub    r[a0.5,41]<16;4,2>:ub    
         mov     (8|M0)    r59.3<4>:ub    r[a0.7,41]<16;4,2>:ub    
         add     (4|M0)    a0.4<1>:w      a0.4<4;4,1>:w            0x200:uw          
         mov     (8|M0)    r33.0<4>:ub    r[a0.0,1]<16;4,2>:ub     
         mov     (8|M0)    r33.1<4>:ub    r[a0.2,1]<16;4,2>:ub     
         mov     (8|M0)    r33.2<4>:ub    r[a0.1,1]<16;4,2>:ub     
         mov     (8|M0)    r33.3<4>:ub    r[a0.3,1]<16;4,2>:ub     
         mov     (8|M0)    r34.0<4>:ub    r[a0.0,33]<16;4,2>:ub    
         mov     (8|M0)    r34.1<4>:ub    r[a0.2,33]<16;4,2>:ub    
         mov     (8|M0)    r34.2<4>:ub    r[a0.1,33]<16;4,2>:ub    
         mov     (8|M0)    r34.3<4>:ub    r[a0.3,33]<16;4,2>:ub    
         mov     (8|M0)    r42.0<4>:ub    r[a0.0,9]<16;4,2>:ub     
         mov     (8|M0)    r42.1<4>:ub    r[a0.2,9]<16;4,2>:ub     
         mov     (8|M0)    r42.2<4>:ub    r[a0.1,9]<16;4,2>:ub     
         mov     (8|M0)    r42.3<4>:ub    r[a0.3,9]<16;4,2>:ub     
         mov     (8|M0)    r43.0<4>:ub    r[a0.0,41]<16;4,2>:ub    
         mov     (8|M0)    r43.1<4>:ub    r[a0.2,41]<16;4,2>:ub    
         mov     (8|M0)    r43.2<4>:ub    r[a0.1,41]<16;4,2>:ub    
         mov     (8|M0)    r43.3<4>:ub    r[a0.3,41]<16;4,2>:ub    
         add     (4|M0)    a0.0<1>:w      a0.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)    r51.0<4>:ub    r[a0.4,1]<16;4,2>:ub     
         mov     (8|M0)    r51.1<4>:ub    r[a0.6,1]<16;4,2>:ub     
         mov     (8|M0)    r51.2<4>:ub    r[a0.5,1]<16;4,2>:ub     
         mov     (8|M0)    r51.3<4>:ub    r[a0.7,1]<16;4,2>:ub     
         mov     (8|M0)    r52.0<4>:ub    r[a0.4,33]<16;4,2>:ub    
         mov     (8|M0)    r52.1<4>:ub    r[a0.6,33]<16;4,2>:ub    
         mov     (8|M0)    r52.2<4>:ub    r[a0.5,33]<16;4,2>:ub    
         mov     (8|M0)    r52.3<4>:ub    r[a0.7,33]<16;4,2>:ub    
         mov     (8|M0)    r60.0<4>:ub    r[a0.4,9]<16;4,2>:ub     
         mov     (8|M0)    r60.1<4>:ub    r[a0.6,9]<16;4,2>:ub     
         mov     (8|M0)    r60.2<4>:ub    r[a0.5,9]<16;4,2>:ub     
         mov     (8|M0)    r60.3<4>:ub    r[a0.7,9]<16;4,2>:ub     
         mov     (8|M0)    r61.0<4>:ub    r[a0.4,41]<16;4,2>:ub    
         mov     (8|M0)    r61.1<4>:ub    r[a0.6,41]<16;4,2>:ub    
         mov     (8|M0)    r61.2<4>:ub    r[a0.5,41]<16;4,2>:ub    
         mov     (8|M0)    r61.3<4>:ub    r[a0.7,41]<16;4,2>:ub    
         add     (4|M0)    a0.4<1>:w      a0.4<4;4,1>:w            0x200:uw          
         add     (1|M0)    r37.0<1>:d     r27.0<0;1,0>:d           16:d              
         add     (1|M0)    r46.0<1>:d     r27.0<0;1,0>:d           32:d              
         add     (1|M0)    r55.0<1>:d     r27.0<0;1,0>:d           48:d              
         mov     (8|M0)    r35.0<4>:ub    r[a0.0,1]<16;4,2>:ub     
         mov     (8|M0)    r35.1<4>:ub    r[a0.2,1]<16;4,2>:ub     
         mov     (8|M0)    r35.2<4>:ub    r[a0.1,1]<16;4,2>:ub     
         mov     (8|M0)    r35.3<4>:ub    r[a0.3,1]<16;4,2>:ub     
         mov     (8|M0)    r36.0<4>:ub    r[a0.0,33]<16;4,2>:ub    
         mov     (8|M0)    r36.1<4>:ub    r[a0.2,33]<16;4,2>:ub    
         mov     (8|M0)    r36.2<4>:ub    r[a0.1,33]<16;4,2>:ub    
         mov     (8|M0)    r36.3<4>:ub    r[a0.3,33]<16;4,2>:ub    
         mov     (8|M0)    r44.0<4>:ub    r[a0.0,9]<16;4,2>:ub     
         mov     (8|M0)    r44.1<4>:ub    r[a0.2,9]<16;4,2>:ub     
         mov     (8|M0)    r44.2<4>:ub    r[a0.1,9]<16;4,2>:ub     
         mov     (8|M0)    r44.3<4>:ub    r[a0.3,9]<16;4,2>:ub     
         mov     (8|M0)    r45.0<4>:ub    r[a0.0,41]<16;4,2>:ub    
         mov     (8|M0)    r45.1<4>:ub    r[a0.2,41]<16;4,2>:ub    
         mov     (8|M0)    r45.2<4>:ub    r[a0.1,41]<16;4,2>:ub    
         mov     (8|M0)    r45.3<4>:ub    r[a0.3,41]<16;4,2>:ub    
         mov     (8|M0)    r53.0<4>:ub    r[a0.4,1]<16;4,2>:ub     
         mov     (8|M0)    r53.1<4>:ub    r[a0.6,1]<16;4,2>:ub     
         mov     (8|M0)    r53.2<4>:ub    r[a0.5,1]<16;4,2>:ub     
         mov     (8|M0)    r53.3<4>:ub    r[a0.7,1]<16;4,2>:ub     
         mov     (8|M0)    r54.0<4>:ub    r[a0.4,33]<16;4,2>:ub    
         mov     (8|M0)    r54.1<4>:ub    r[a0.6,33]<16;4,2>:ub    
         mov     (8|M0)    r54.2<4>:ub    r[a0.5,33]<16;4,2>:ub    
         mov     (8|M0)    r54.3<4>:ub    r[a0.7,33]<16;4,2>:ub    
         mov     (8|M0)    r62.0<4>:ub    r[a0.4,9]<16;4,2>:ub     
         mov     (8|M0)    r62.1<4>:ub    r[a0.6,9]<16;4,2>:ub     
         mov     (8|M0)    r62.2<4>:ub    r[a0.5,9]<16;4,2>:ub     
         mov     (8|M0)    r62.3<4>:ub    r[a0.7,9]<16;4,2>:ub     
         mov     (8|M0)    r63.0<4>:ub    r[a0.4,41]<16;4,2>:ub    
         mov     (8|M0)    r63.1<4>:ub    r[a0.6,41]<16;4,2>:ub    
         mov     (8|M0)    r63.2<4>:ub    r[a0.5,41]<16;4,2>:ub    
         mov     (8|M0)    r63.3<4>:ub    r[a0.7,41]<16;4,2>:ub    
         send    (8|M0)    null:d         r28:ub                   0xC               0x120A8018    
         send    (8|M0)    null:d         r37:ub                   0xC               0x120A8018    
         send    (8|M0)    null:d         r46:ub                   0xC               0x120A8018    
         send    (8|M0)    null:d         r55:ub                   0xC               0x120A8018    
