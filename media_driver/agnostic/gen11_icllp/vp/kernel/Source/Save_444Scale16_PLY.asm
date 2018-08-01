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
         add     (4|M0)     a0.0<1>:w                r22.0<4;4,1>:w            0x0:uw            
         mov     (8|M0)     r28.0<1>:ud              r0.0<8;8,1>:ud            
         mov     (2|M0)     r28.0<1>:d               r7.0<2;2,1>:w             
         mov     (1|M0)     r28.2<1>:ud              0xF000F:ud                
         add     (4|M0)     a0.4<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x80:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x80:uw
         add     (4|M0)     a0.0<1>:w                r22.0<4;4,1>:w            0x0:uw            
         add     (4|M0)     a0.4<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         mov     (8|M0)     r29.0<1>:ub              r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)     r29.16<1>:ub             r[a0.1,17]<16;8,2>:ub     
         mov     (8|M0)     r30.0<1>:ub              r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)     r30.16<1>:ub             r[a0.1,49]<16;8,2>:ub     
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r29.8<1>:ub              r[a0.5,1]<16;8,2>:ub      
         mov     (8|M0)     r29.24<1>:ub             r[a0.5,17]<16;8,2>:ub     
         mov     (8|M0)     r30.8<1>:ub              r[a0.5,33]<16;8,2>:ub     
         mov     (8|M0)     r30.24<1>:ub             r[a0.5,49]<16;8,2>:ub     
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r31.0<1>:ub              r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)     r31.16<1>:ub             r[a0.1,17]<16;8,2>:ub     
         mov     (8|M0)     r32.0<1>:ub              r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)     r32.16<1>:ub             r[a0.1,49]<16;8,2>:ub     
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r31.8<1>:ub              r[a0.5,1]<16;8,2>:ub      
         mov     (8|M0)     r31.24<1>:ub             r[a0.5,17]<16;8,2>:ub     
         mov     (8|M0)     r32.8<1>:ub              r[a0.5,33]<16;8,2>:ub     
         mov     (8|M0)     r32.24<1>:ub             r[a0.5,49]<16;8,2>:ub     
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r33.0<1>:ub              r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)     r33.16<1>:ub             r[a0.1,17]<16;8,2>:ub     
         mov     (8|M0)     r34.0<1>:ub              r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)     r34.16<1>:ub             r[a0.1,49]<16;8,2>:ub     
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r33.8<1>:ub              r[a0.5,1]<16;8,2>:ub      
         mov     (8|M0)     r33.24<1>:ub             r[a0.5,17]<16;8,2>:ub     
         mov     (8|M0)     r34.8<1>:ub              r[a0.5,33]<16;8,2>:ub     
         mov     (8|M0)     r34.24<1>:ub             r[a0.5,49]<16;8,2>:ub     
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw          
         mov     (8|M0)     r35.0<1>:ub              r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)     r35.16<1>:ub             r[a0.1,17]<16;8,2>:ub     
         mov     (8|M0)     r36.0<1>:ub              r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)     r36.16<1>:ub             r[a0.1,49]<16;8,2>:ub     
         mov     (8|M0)     r35.8<1>:ub              r[a0.5,1]<16;8,2>:ub      
         mov     (8|M0)     r35.24<1>:ub             r[a0.5,17]<16;8,2>:ub     
         mov     (8|M0)     r36.8<1>:ub              r[a0.5,33]<16;8,2>:ub     
         mov     (8|M0)     r36.24<1>:ub             r[a0.5,49]<16;8,2>:ub     
         send    (1|M0)     null:d                   r28:ub                    0xC               0x120A8018    
