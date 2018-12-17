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
         add     (4|M0)     a0.0<1>:w                r22.0<4;4,1>:w            0x0:uw                      {AccWrEn}
         mov     (1|M0)     r17.0<1>:uw              a0.0<0;1,0>:uw            
         mov     (1|M0)     a0.0<1>:uw               a0.2<0;1,0>:uw            
         mov     (1|M0)     a0.2<1>:uw               r17.0<0;1,0>:uw           
         mov     (8|M0)     r27.0<1>:ud              r0.0<8;8,1>:ud            
         shl     (2|M0)     r27.0<1>:d               r7.0<2;2,1>:w             0x2:v                   
         mov     (1|M0)     r27.2<1>:ud              0xF000F:ud                
         add     (4|M0)     a0.4<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w          
         mov     (8|M0)     r28.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r37.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r46.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r55.0<1>:ud              r27.0<8;8,1>:ud           
         add     (1|M0)     r37.0<1>:d               r27.0<0;1,0>:d            16:d                    
         add     (1|M0)     r46.0<1>:d               r27.0<0;1,0>:d            32:d                    
         add     (1|M0)     r55.0<1>:d               r27.0<0;1,0>:d            48:d                    
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x2000:uw
         add     (16|M0)    (sat)r[a0.7]<1>:uw       r[a0.7]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.7,32]<1>:uw    r[a0.7,32]<16;16,1>:uw    0x2000:uw
         shr     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.0,32]<1>:uw         r[a0.0,32]<16;16,1>:uw    0x6:uw
         shr     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.4,32]<1>:uw         r[a0.4,32]<16;16,1>:uw    0x6:uw
         and     (16|M0)    r[a0.3]<1>:uw            r[a0.3]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.3,32]<1>:uw         r[a0.3,32]<16;16,1>:uw    0xC000:uw
         and     (16|M0)    r[a0.7]<1>:uw            r[a0.7]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.7,32]<1>:uw         r[a0.7,32]<16;16,1>:uw    0xC000:uw
         and     (8|M0)     r29.0<2>:uw              r[a0.1]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r29.1<2>:uw              r[a0.2]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r29.0<1>:ud              r29.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r29.0<2>:uw              r29.0<16;8,2>:uw          r[a0.0]<8;4,1>:uw
         add     (8|M0)     r29.1<2>:uw              r29.1<16;8,2>:uw          r[a0.3]<8;4,1>:uw
         and     (8|M0)     r30.0<2>:uw              r[a0.1,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r30.1<2>:uw              r[a0.2,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r30.0<1>:ud              r30.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r30.0<2>:uw              r30.0<16;8,2>:uw          r[a0.0,32]<8;4,1>:uw
         add     (8|M0)     r30.1<2>:uw              r30.1<16;8,2>:uw          r[a0.3,32]<8;4,1>:uw
         and     (8|M0)     r38.0<2>:uw              r[a0.1,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r38.1<2>:uw              r[a0.2,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r38.0<1>:ud              r38.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r38.0<2>:uw              r38.0<16;8,2>:uw          r[a0.0,8]<8;4,1>:uw
         add     (8|M0)     r38.1<2>:uw              r38.1<16;8,2>:uw          r[a0.3,8]<8;4,1>:uw
         and     (8|M0)     r39.0<2>:uw              r[a0.1,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r39.1<2>:uw              r[a0.2,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r39.0<1>:ud              r39.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r39.0<2>:uw              r39.0<16;8,2>:uw          r[a0.0,40]<8;4,1>:uw
         add     (8|M0)     r39.1<2>:uw              r39.1<16;8,2>:uw          r[a0.3,40]<8;4,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                
         and     (8|M0)     r47.0<2>:uw              r[a0.5]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r47.1<2>:uw              r[a0.6]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r47.0<1>:ud              r47.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r47.0<2>:uw              r47.0<16;8,2>:uw          r[a0.4]<8;4,1>:uw
         add     (8|M0)     r47.1<2>:uw              r47.1<16;8,2>:uw          r[a0.7]<8;4,1>:uw
         and     (8|M0)     r48.0<2>:uw              r[a0.5,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r48.1<2>:uw              r[a0.6,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r48.0<1>:ud              r48.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r48.0<2>:uw              r48.0<16;8,2>:uw          r[a0.4,32]<8;4,1>:uw
         add     (8|M0)     r48.1<2>:uw              r48.1<16;8,2>:uw          r[a0.7,32]<8;4,1>:uw
         and     (8|M0)     r56.0<2>:uw              r[a0.5,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r56.1<2>:uw              r[a0.6,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r56.0<1>:ud              r56.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r56.0<2>:uw              r56.0<16;8,2>:uw          r[a0.4,8]<8;4,1>:uw
         add     (8|M0)     r56.1<2>:uw              r56.1<16;8,2>:uw          r[a0.7,8]<8;4,1>:uw
         and     (8|M0)     r57.0<2>:uw              r[a0.5,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r57.1<2>:uw              r[a0.6,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r57.0<1>:ud              r57.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r57.0<2>:uw              r57.0<16;8,2>:uw          r[a0.4,40]<8;4,1>:uw
         add     (8|M0)     r57.1<2>:uw              r57.1<16;8,2>:uw          r[a0.7,40]<8;4,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw                
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x2000:uw
         add     (16|M0)    (sat)r[a0.7]<1>:uw       r[a0.7]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.7,32]<1>:uw    r[a0.7,32]<16;16,1>:uw    0x2000:uw
         shr     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.0,32]<1>:uw         r[a0.0,32]<16;16,1>:uw    0x6:uw
         shr     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.4,32]<1>:uw         r[a0.4,32]<16;16,1>:uw    0x6:uw
         and     (16|M0)    r[a0.3]<1>:uw            r[a0.3]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.3,32]<1>:uw         r[a0.3,32]<16;16,1>:uw    0xC000:uw
         and     (16|M0)    r[a0.7]<1>:uw            r[a0.7]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.7,32]<1>:uw         r[a0.7,32]<16;16,1>:uw    0xC000:uw
         and     (8|M0)     r31.0<2>:uw              r[a0.1]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r31.1<2>:uw              r[a0.2]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r31.0<1>:ud              r31.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r31.0<2>:uw              r31.0<16;8,2>:uw          r[a0.0]<8;4,1>:uw
         add     (8|M0)     r31.1<2>:uw              r31.1<16;8,2>:uw          r[a0.3]<8;4,1>:uw
         and     (8|M0)     r32.0<2>:uw              r[a0.1,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r32.1<2>:uw              r[a0.2,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r32.0<1>:ud              r32.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r32.0<2>:uw              r32.0<16;8,2>:uw          r[a0.0,32]<8;4,1>:uw
         add     (8|M0)     r32.1<2>:uw              r32.1<16;8,2>:uw          r[a0.3,32]<8;4,1>:uw
         and     (8|M0)     r40.0<2>:uw              r[a0.1,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r40.1<2>:uw              r[a0.2,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r40.0<1>:ud              r40.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r40.0<2>:uw              r40.0<16;8,2>:uw          r[a0.0,8]<8;4,1>:uw
         add     (8|M0)     r40.1<2>:uw              r40.1<16;8,2>:uw          r[a0.3,8]<8;4,1>:uw
         and     (8|M0)     r41.0<2>:uw              r[a0.1,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r41.1<2>:uw              r[a0.2,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r41.0<1>:ud              r41.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r41.0<2>:uw              r41.0<16;8,2>:uw          r[a0.0,40]<8;4,1>:uw
         add     (8|M0)     r41.1<2>:uw              r41.1<16;8,2>:uw          r[a0.3,40]<8;4,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                
         and     (8|M0)     r49.0<2>:uw              r[a0.5]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r49.1<2>:uw              r[a0.6]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r49.0<1>:ud              r49.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r49.0<2>:uw              r49.0<16;8,2>:uw          r[a0.4]<8;4,1>:uw
         add     (8|M0)     r49.1<2>:uw              r49.1<16;8,2>:uw          r[a0.7]<8;4,1>:uw
         and     (8|M0)     r50.0<2>:uw              r[a0.5,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r50.1<2>:uw              r[a0.6,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r50.0<1>:ud              r50.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r50.0<2>:uw              r50.0<16;8,2>:uw          r[a0.4,32]<8;4,1>:uw
         add     (8|M0)     r50.1<2>:uw              r50.1<16;8,2>:uw          r[a0.7,32]<8;4,1>:uw
         and     (8|M0)     r58.0<2>:uw              r[a0.5,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r58.1<2>:uw              r[a0.6,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r58.0<1>:ud              r58.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r58.0<2>:uw              r58.0<16;8,2>:uw          r[a0.4,8]<8;4,1>:uw
         add     (8|M0)     r58.1<2>:uw              r58.1<16;8,2>:uw          r[a0.7,8]<8;4,1>:uw
         and     (8|M0)     r59.0<2>:uw              r[a0.5,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r59.1<2>:uw              r[a0.6,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r59.0<1>:ud              r59.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r59.0<2>:uw              r59.0<16;8,2>:uw          r[a0.4,40]<8;4,1>:uw
         add     (8|M0)     r59.1<2>:uw              r59.1<16;8,2>:uw          r[a0.7,40]<8;4,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw                
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x2000:uw
         add     (16|M0)    (sat)r[a0.7]<1>:uw       r[a0.7]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.7,32]<1>:uw    r[a0.7,32]<16;16,1>:uw    0x2000:uw
         shr     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.0,32]<1>:uw         r[a0.0,32]<16;16,1>:uw    0x6:uw
         shr     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.4,32]<1>:uw         r[a0.4,32]<16;16,1>:uw    0x6:uw
         and     (16|M0)    r[a0.3]<1>:uw            r[a0.3]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.3,32]<1>:uw         r[a0.3,32]<16;16,1>:uw    0xC000:uw
         and     (16|M0)    r[a0.7]<1>:uw            r[a0.7]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.7,32]<1>:uw         r[a0.7,32]<16;16,1>:uw    0xC000:uw
         and     (8|M0)     r33.0<2>:uw              r[a0.1]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r33.1<2>:uw              r[a0.2]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r33.0<1>:ud              r33.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r33.0<2>:uw              r33.0<16;8,2>:uw          r[a0.0]<8;4,1>:uw
         add     (8|M0)     r33.1<2>:uw              r33.1<16;8,2>:uw          r[a0.3]<8;4,1>:uw
         and     (8|M0)     r34.0<2>:uw              r[a0.1,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r34.1<2>:uw              r[a0.2,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r34.0<1>:ud              r34.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r34.0<2>:uw              r34.0<16;8,2>:uw          r[a0.0,32]<8;4,1>:uw
         add     (8|M0)     r34.1<2>:uw              r34.1<16;8,2>:uw          r[a0.3,32]<8;4,1>:uw
         and     (8|M0)     r42.0<2>:uw              r[a0.1,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r42.1<2>:uw              r[a0.2,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r42.0<1>:ud              r42.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r42.0<2>:uw              r42.0<16;8,2>:uw          r[a0.0,8]<8;4,1>:uw
         add     (8|M0)     r42.1<2>:uw              r42.1<16;8,2>:uw          r[a0.3,8]<8;4,1>:uw
         and     (8|M0)     r43.0<2>:uw              r[a0.1,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r43.1<2>:uw              r[a0.2,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r43.0<1>:ud              r43.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r43.0<2>:uw              r43.0<16;8,2>:uw          r[a0.0,40]<8;4,1>:uw
         add     (8|M0)     r43.1<2>:uw              r43.1<16;8,2>:uw          r[a0.3,40]<8;4,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                
         and     (8|M0)     r51.0<2>:uw              r[a0.5]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r51.1<2>:uw              r[a0.6]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r51.0<1>:ud              r51.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r51.0<2>:uw              r51.0<16;8,2>:uw          r[a0.4]<8;4,1>:uw
         add     (8|M0)     r51.1<2>:uw              r51.1<16;8,2>:uw          r[a0.7]<8;4,1>:uw
         and     (8|M0)     r52.0<2>:uw              r[a0.5,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r52.1<2>:uw              r[a0.6,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r52.0<1>:ud              r52.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r52.0<2>:uw              r52.0<16;8,2>:uw          r[a0.4,32]<8;4,1>:uw
         add     (8|M0)     r52.1<2>:uw              r52.1<16;8,2>:uw          r[a0.7,32]<8;4,1>:uw
         and     (8|M0)     r60.0<2>:uw              r[a0.5,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r60.1<2>:uw              r[a0.6,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r60.0<1>:ud              r60.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r60.0<2>:uw              r60.0<16;8,2>:uw          r[a0.4,8]<8;4,1>:uw
         add     (8|M0)     r60.1<2>:uw              r60.1<16;8,2>:uw          r[a0.7,8]<8;4,1>:uw
         and     (8|M0)     r61.0<2>:uw              r[a0.5,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r61.1<2>:uw              r[a0.6,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r61.0<1>:ud              r61.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r61.0<2>:uw              r61.0<16;8,2>:uw          r[a0.4,40]<8;4,1>:uw
         add     (8|M0)     r61.1<2>:uw              r61.1<16;8,2>:uw          r[a0.7,40]<8;4,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw                
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.0,32]<1>:uw    r[a0.0,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.4,32]<1>:uw    r[a0.4,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.2,32]<1>:uw    r[a0.2,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         add     (16|M0)    (sat)r[a0.6,32]<1>:uw    r[a0.6,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r[a0.3]<1>:uw       r[a0.3]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.3,32]<1>:uw    r[a0.3,32]<16;16,1>:uw    0x2000:uw
         add     (16|M0)    (sat)r[a0.7]<1>:uw       r[a0.7]<16;16,1>:uw       0x2000:uw
         add     (16|M0)    (sat)r[a0.7,32]<1>:uw    r[a0.7,32]<16;16,1>:uw    0x2000:uw
         shr     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.0,32]<1>:uw         r[a0.0,32]<16;16,1>:uw    0x6:uw
         shr     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0x6:uw                  
         shr     (16|M0)    r[a0.4,32]<1>:uw         r[a0.4,32]<16;16,1>:uw    0x6:uw
         and     (16|M0)    r[a0.3]<1>:uw            r[a0.3]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.3,32]<1>:uw         r[a0.3,32]<16;16,1>:uw    0xC000:uw
         and     (16|M0)    r[a0.7]<1>:uw            r[a0.7]<16;16,1>:uw       0xC000:uw               
         and     (16|M0)    r[a0.7,32]<1>:uw         r[a0.7,32]<16;16,1>:uw    0xC000:uw
         and     (8|M0)     r35.0<2>:uw              r[a0.1]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r35.1<2>:uw              r[a0.2]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r35.0<1>:ud              r35.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r35.0<2>:uw              r35.0<16;8,2>:uw          r[a0.0]<8;4,1>:uw
         add     (8|M0)     r35.1<2>:uw              r35.1<16;8,2>:uw          r[a0.3]<8;4,1>:uw
         and     (8|M0)     r36.0<2>:uw              r[a0.1,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r36.1<2>:uw              r[a0.2,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r36.0<1>:ud              r36.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r36.0<2>:uw              r36.0<16;8,2>:uw          r[a0.0,32]<8;4,1>:uw
         add     (8|M0)     r36.1<2>:uw              r36.1<16;8,2>:uw          r[a0.3,32]<8;4,1>:uw
         and     (8|M0)     r44.0<2>:uw              r[a0.1,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r44.1<2>:uw              r[a0.2,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r44.0<1>:ud              r44.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r44.0<2>:uw              r44.0<16;8,2>:uw          r[a0.0,8]<8;4,1>:uw
         add     (8|M0)     r44.1<2>:uw              r44.1<16;8,2>:uw          r[a0.3,8]<8;4,1>:uw
         and     (8|M0)     r45.0<2>:uw              r[a0.1,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r45.1<2>:uw              r[a0.2,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r45.0<1>:ud              r45.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r45.0<2>:uw              r45.0<16;8,2>:uw          r[a0.0,40]<8;4,1>:uw
         add     (8|M0)     r45.1<2>:uw              r45.1<16;8,2>:uw          r[a0.3,40]<8;4,1>:uw
         add     (4|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                
         and     (8|M0)     r53.0<2>:uw              r[a0.5]<8;4,1>:uw         0xFFC0:uw               
         shr     (8|M0)     r53.1<2>:uw              r[a0.6]<8;4,1>:uw         0x6:uw                  
         shl     (8|M0)     r53.0<1>:ud              r53.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r53.0<2>:uw              r53.0<16;8,2>:uw          r[a0.4]<8;4,1>:uw
         add     (8|M0)     r53.1<2>:uw              r53.1<16;8,2>:uw          r[a0.7]<8;4,1>:uw
         and     (8|M0)     r54.0<2>:uw              r[a0.5,32]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r54.1<2>:uw              r[a0.6,32]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r54.0<1>:ud              r54.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r54.0<2>:uw              r54.0<16;8,2>:uw          r[a0.4,32]<8;4,1>:uw
         add     (8|M0)     r54.1<2>:uw              r54.1<16;8,2>:uw          r[a0.7,32]<8;4,1>:uw
         and     (8|M0)     r62.0<2>:uw              r[a0.5,8]<8;4,1>:uw       0xFFC0:uw               
         shr     (8|M0)     r62.1<2>:uw              r[a0.6,8]<8;4,1>:uw       0x6:uw                  
         shl     (8|M0)     r62.0<1>:ud              r62.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r62.0<2>:uw              r62.0<16;8,2>:uw          r[a0.4,8]<8;4,1>:uw
         add     (8|M0)     r62.1<2>:uw              r62.1<16;8,2>:uw          r[a0.7,8]<8;4,1>:uw
         and     (8|M0)     r63.0<2>:uw              r[a0.5,40]<8;4,1>:uw      0xFFC0:uw               
         shr     (8|M0)     r63.1<2>:uw              r[a0.6,40]<8;4,1>:uw      0x6:uw                  
         shl     (8|M0)     r63.0<1>:ud              r63.0<8;8,1>:ud           0x4:ud                  
         add     (8|M0)     r63.0<2>:uw              r63.0<16;8,2>:uw          r[a0.4,40]<8;4,1>:uw
         add     (8|M0)     r63.1<2>:uw              r63.1<16;8,2>:uw          r[a0.7,40]<8;4,1>:uw
         add     (4|M0)     a0.4<1>:w                a0.4<4;4,1>:w             0x200:uw                
         send    (8|M0)     null:d                   r28:ub                    0xC                     0x120A8018    
         send    (8|M0)     null:d                   r37:ub                    0xC                     0x120A8018    
         send    (8|M0)     null:d                   r46:ub                    0xC                     0x120A8018    
         send    (8|M0)     null:d                   r55:ub                    0xC                     0x120A8018    
