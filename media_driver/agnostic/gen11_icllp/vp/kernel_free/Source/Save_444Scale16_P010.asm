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
         shl     (2|M0)     r27.0<1>:d               r7.0<2;2,1>:w             0x1:v                    
         mov     (8|M0)     r28.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r37.0<1>:ud              r27.0<8;8,1>:ud           
         shr     (1|M0)     r27.1<1>:d               r7.1<0;1,0>:w             1:w                      
         mov     (8|M0)     r46.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)     r55.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (1|M0)     r28.2<1>:ud              0xF000F:ud                
         add     (1|M0)     r37.0<1>:d               r27.0<0;1,0>:d            16:d                     
         mov     (1|M0)     r37.2<1>:ud              0xF000F:ud                
         mov     (1|M0)     r46.2<1>:ud              0x7000F:ud                
         add     (1|M0)     r55.0<1>:d               r27.0<0;1,0>:d            16:d                     
         mov     (1|M0)     r55.2<1>:ud              0x7000F:ud                
         add     (4|M0)     a0.4<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w           
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0xFFC0:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0xFFC0:uw
         mov     (16|M0)    r29.0<1>:uw              r[a0.1]<16;16,1>:uw       
         mov     (16|M0)    r30.0<1>:uw              r[a0.1,32]<16;16,1>:uw    
         mov     (16|M0)    r38.0<1>:uw              r[a0.5]<16;16,1>:uw       
         mov     (16|M0)    r39.0<1>:uw              r[a0.5,32]<16;16,1>:uw    
         avg     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r47.0<2>:uw              r[a0.2]<16;8,2>:uw        
         mov     (8|M0)     r56.0<2>:uw              r[a0.6]<16;8,2>:uw        
         avg     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r47.1<2>:uw              r[a0.0]<16;8,2>:uw        
         mov     (8|M0)     r56.1<2>:uw              r[a0.4]<16;8,2>:uw        
         add     (8|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0xFFC0:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0xFFC0:uw
         mov     (16|M0)    r31.0<1>:uw              r[a0.1]<16;16,1>:uw       
         mov     (16|M0)    r32.0<1>:uw              r[a0.1,32]<16;16,1>:uw    
         mov     (16|M0)    r40.0<1>:uw              r[a0.5]<16;16,1>:uw       
         mov     (16|M0)    r41.0<1>:uw              r[a0.5,32]<16;16,1>:uw    
         avg     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r48.0<2>:uw              r[a0.2]<16;8,2>:uw        
         mov     (8|M0)     r57.0<2>:uw              r[a0.6]<16;8,2>:uw        
         avg     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r48.1<2>:uw              r[a0.0]<16;8,2>:uw        
         mov     (8|M0)     r57.1<2>:uw              r[a0.4]<16;8,2>:uw        
         add     (8|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0xFFC0:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0xFFC0:uw
         mov     (16|M0)    r33.0<1>:uw              r[a0.1]<16;16,1>:uw       
         mov     (16|M0)    r34.0<1>:uw              r[a0.1,32]<16;16,1>:uw    
         mov     (16|M0)    r42.0<1>:uw              r[a0.5]<16;16,1>:uw       
         mov     (16|M0)    r43.0<1>:uw              r[a0.5,32]<16;16,1>:uw    
         avg     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r49.0<2>:uw              r[a0.2]<16;8,2>:uw        
         mov     (8|M0)     r58.0<2>:uw              r[a0.6]<16;8,2>:uw        
         avg     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r49.1<2>:uw              r[a0.0]<16;8,2>:uw        
         mov     (8|M0)     r58.1<2>:uw              r[a0.4]<16;8,2>:uw        
         add     (8|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                 
         add     (16|M0)    (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.1]<1>:uw            r[a0.1]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.1,32]<1>:uw         r[a0.1,32]<16;16,1>:uw    0xFFC0:uw
         add     (16|M0)    (sat)r[a0.5]<1>:uw       r[a0.5]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.5]<1>:uw            r[a0.5]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.5,32]<1>:uw    r[a0.5,32]<16;16,1>:uw    0x20:uw
         and     (16|M0)    r[a0.5,32]<1>:uw         r[a0.5,32]<16;16,1>:uw    0xFFC0:uw
         mov     (16|M0)    r35.0<1>:uw              r[a0.1]<16;16,1>:uw       
         mov     (16|M0)    r36.0<1>:uw              r[a0.1,32]<16;16,1>:uw    
         mov     (16|M0)    r44.0<1>:uw              r[a0.5]<16;16,1>:uw       
         mov     (16|M0)    r45.0<1>:uw              r[a0.5,32]<16;16,1>:uw    
         avg     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;8,1>:uw        r[a0.2,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;8,1>:uw        r[a0.6,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.2]<1>:uw       r[a0.2]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.2]<1>:uw            r[a0.2]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.6]<1>:uw       r[a0.6]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.6]<1>:uw            r[a0.6]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r50.0<2>:uw              r[a0.2]<16;8,2>:uw        
         mov     (8|M0)     r59.0<2>:uw              r[a0.6]<16;8,2>:uw        
         avg     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;8,1>:uw        r[a0.0,16]<16;8,1>:uw
         avg     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;8,1>:uw        r[a0.4,16]<16;8,1>:uw
         add     (16|M0)    (sat)r[a0.0]<1>:uw       r[a0.0]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.0]<1>:uw            r[a0.0]<16;16,1>:uw       0xFFC0:uw                
         add     (16|M0)    (sat)r[a0.4]<1>:uw       r[a0.4]<16;16,1>:uw       0x20:uw
         and     (16|M0)    r[a0.4]<1>:uw            r[a0.4]<16;16,1>:uw       0xFFC0:uw                
         mov     (8|M0)     r50.1<2>:uw              r[a0.0]<16;8,2>:uw        
         mov     (8|M0)     r59.1<2>:uw              r[a0.4]<16;8,2>:uw        
         add     (8|M0)     a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw                 
         add     (1|M0)     a0.0<1>:ud               r2.3<0;1,0>:ub            0x120A8000:ud            
         send    (1|M0)     null:d                   r28:ub                    0xC                      a0.0    
         send    (1|M0)     null:d                   r37:ub                    0xC                      a0.0    
         add     (1|M0)     a0.0<1>:ud               r2.3<0;1,0>:ub            0xA0A8001:ud             
         send    (1|M0)     null:d                   r46:ub                    0xC                      a0.0    
         send    (1|M0)     null:d                   r55:ub                    0xC                      a0.0    
         nop     
