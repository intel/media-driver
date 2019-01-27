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
         add     (4|M0)     a0.0<1>:w                r22.0<4;4,1>:w            0x0:uw                   {AccWrEn}
         mov     (8|M0)     r27.0<1>:ud              r0.0<8;8,1>:ud            
         shl     (2|M0)     r27.0<1>:d               r7.0<2;2,1>:w             0x1:v                
         mov     (1|M0)     r27.2<1>:ud              0xF000F:ud                
         add     (4|M0)     a0.4<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w       
         mov     (8|M0)     r46.0<1>:uw              0x51405140:v              
         mov     (8|M0)     r46.8<1>:uw              0x37263726:v              
         mov     (8|M0)     r47.0<1>:uw              0x40514051:v              
         mov     (8|M0)     r47.8<1>:uw              0x26372637:v              
         mov     (16|M0)    r48.0<1>:uw              r46.0<16;16,1>:uw         
         mov     (16|M0)    r49.0<1>:uw              r47.0<16;16,1>:uw         
         shl     (16|M0)    r46.0<1>:uw              r46.0<16;16,1>:uw         0x8:uw               
         shl     (16|M0)    r47.0<1>:uw              r47.0<16;16,1>:uw         0x8:uw               
         shl     (16|M0)    r48.0<1>:uw              r48.0<16;16,1>:uw         0x8:uw               
         shl     (16|M0)    r49.0<1>:uw              r49.0<16;16,1>:uw         0x8:uw               
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         mov     (8|M0)     r50.0<1>:uw              0x63726372:v              
         mov     (8|M0)     r50.8<1>:uw              0x14051405:v              
         mov     (8|M0)     r51.0<1>:uw              0x72637263:v              
         mov     (8|M0)     r51.8<1>:uw              0x5140514:v               
         mov     (16|M0)    r52.0<1>:uw              r50.0<16;16,1>:uw         
         mov     (16|M0)    r53.0<1>:uw              r51.0<16;16,1>:uw         
         shl     (16|M0)    r50.0<1>:uw              r50.0<16;16,1>:uw         0x7:uw               
         shl     (16|M0)    r51.0<1>:uw              r51.0<16;16,1>:uw         0x7:uw               
         shl     (16|M0)    r52.0<1>:uw              r52.0<16;16,1>:uw         0x7:uw               
         shl     (16|M0)    r53.0<1>:uw              r53.0<16;16,1>:uw         0x7:uw               
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       r50.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    r51.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       r52.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    r53.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         and     (16|M0)    r29.0<1>:uw              r[a0.0]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r30.0<1>:uw              r[a0.0,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r29.0<2>:ub              r[a0.2,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r30.0<2>:ub              r[a0.2,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r29.0<1>:uw              r[a0.1]<16;16,1>:uw       r29.0<16;16,1>:uw
         or      (16|M0)    r30.0<1>:uw              r[a0.1,32]<16;16,1>:uw    r30.0<16;16,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw             
         and     (16|M0)    r38.0<1>:uw              r[a0.4]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r39.0<1>:uw              r[a0.4,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r38.0<2>:ub              r[a0.6,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r39.0<2>:ub              r[a0.6,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r38.0<1>:uw              r[a0.5]<16;16,1>:uw       r38.0<16;16,1>:uw
         or      (16|M0)    r39.0<1>:uw              r[a0.5,32]<16;16,1>:uw    r39.0<16;16,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw             
         mov     (8|M0)     r28.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r37.0<1>:ud              r27.0<8;8,1>:ud           
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       r50.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    r51.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       r52.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    r53.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         and     (16|M0)    r31.0<1>:uw              r[a0.0]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r32.0<1>:uw              r[a0.0,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r31.0<2>:ub              r[a0.2,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r32.0<2>:ub              r[a0.2,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r31.0<1>:uw              r[a0.1]<16;16,1>:uw       r31.0<16;16,1>:uw
         or      (16|M0)    r32.0<1>:uw              r[a0.1,32]<16;16,1>:uw    r32.0<16;16,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw             
         and     (16|M0)    r40.0<1>:uw              r[a0.4]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r41.0<1>:uw              r[a0.4,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r40.0<2>:ub              r[a0.6,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r41.0<2>:ub              r[a0.6,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r40.0<1>:uw              r[a0.5]<16;16,1>:uw       r40.0<16;16,1>:uw
         or      (16|M0)    r41.0<1>:uw              r[a0.5,32]<16;16,1>:uw    r41.0<16;16,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw             
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       r50.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    r51.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       r52.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    r53.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         and     (16|M0)    r33.0<1>:uw              r[a0.0]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r34.0<1>:uw              r[a0.0,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r33.0<2>:ub              r[a0.2,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r34.0<2>:ub              r[a0.2,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r33.0<1>:uw              r[a0.1]<16;16,1>:uw       r33.0<16;16,1>:uw
         or      (16|M0)    r34.0<1>:uw              r[a0.1,32]<16;16,1>:uw    r34.0<16;16,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw             
         and     (16|M0)    r42.0<1>:uw              r[a0.4]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r43.0<1>:uw              r[a0.4,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r42.0<2>:ub              r[a0.6,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r43.0<2>:ub              r[a0.6,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r42.0<1>:uw              r[a0.5]<16;16,1>:uw       r42.0<16;16,1>:uw
         or      (16|M0)    r43.0<1>:uw              r[a0.5,32]<16;16,1>:uw    r43.0<16;16,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw             
         add     (1|M0)     r37.0<1>:d               r27.0<0;1,0>:d            16:d                 
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       r50.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    r51.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       r52.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    r53.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       r46.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    r47.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       r48.0<16;16,1>:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    r49.0<16;16,1>:uw
         and     (16|M0)    r35.0<1>:uw              r[a0.0]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r36.0<1>:uw              r[a0.0,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r35.0<2>:ub              r[a0.2,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r36.0<2>:ub              r[a0.2,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r35.0<1>:uw              r[a0.1]<16;16,1>:uw       r35.0<16;16,1>:uw
         or      (16|M0)    r36.0<1>:uw              r[a0.1,32]<16;16,1>:uw    r36.0<16;16,1>:uw
         and     (16|M0)    r44.0<1>:uw              r[a0.4]<16;16,1>:uw       0xF800:uw            
         and     (16|M0)    r45.0<1>:uw              r[a0.4,32]<16;16,1>:uw    0xF800:uw            
         shr     (16|M0)    r44.0<2>:ub              r[a0.6,1]<32;16,2>:ub     0x3:uw               
         shr     (16|M0)    r45.0<2>:ub              r[a0.6,33]<32;16,2>:ub    0x3:uw               
         shr     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x5:uw               
         shr     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x5:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0x7E0:uw             
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0x7E0:uw
         or      (16|M0)    r44.0<1>:uw              r[a0.5]<16;16,1>:uw       r44.0<16;16,1>:uw
         or      (16|M0)    r45.0<1>:uw              r[a0.5,32]<16;16,1>:uw    r45.0<16;16,1>:uw
         send    (8|M0)     null:d                   r28:ub                    0xC                  0x120A8018    
         send    (8|M0)     null:d                   r37:ub                    0xC                  0x120A8018    
