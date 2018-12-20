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
         mov     (8|M0)     r27.0<1>:ud         r0.0<8;8,1>:ud            
         shl     (2|M0)     r27.0<1>:d          r7.0<2;2,1>:w             0x2:v              
         mov     (1|M0)     r27.2<1>:ud         0x3001F:ud                
         add     (4|M0)     a0.4<1>:w           r2.28<4;4,1>:ub           0x3A0:uw           
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:w            0x0:uw             
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         mov     (8|M0)     r28.0<1>:ud         r27.0<8;8,1>:ud           
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:w            0x0:uw             
         add     (4|M0)     a0.0<1>:uw          a0.0<4;4,1>:w             r22.8<0;2,1>:w     
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         add     (1|M0)     r28.0<1>:d          r27.0<0;1,0>:d            32:d               
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:w           r22.0<4;4,1>:w            0x200:uw           
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         mov     (8|M0)     r28.0<1>:ud         r27.0<8;8,1>:ud           
         add     (1|M0)     r28.1<1>:d          r27.1<0;1,0>:d            4:d                
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:w            0x200:uw           
         add     (4|M0)     a0.0<1>:uw          a0.0<4;4,1>:uw            r22.8<0;2,1>:uw    
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         add     (1|M0)     r28.0<1>:ud         r27.0<0;1,0>:ud           32:d               
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:uw           0x400:uw           
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         mov     (8|M0)     r28.0<1>:ud         r27.0<8;8,1>:ud           
         add     (1|M0)     r28.1<1>:d          r27.1<0;1,0>:d            8:d                
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:uw           0x400:uw           
         add     (4|M0)     a0.0<1>:uw          a0.0<4;4,1>:uw            r22.8<0;2,1>:uw    
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         add     (1|M0)     r28.0<1>:ud         r27.0<0;1,0>:ud           32:d               
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:uw           0x600:uw           
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         mov     (8|M0)     r28.0<1>:ud         r27.0<8;8,1>:ud           
         add     (1|M0)     r28.1<1>:d          r27.1<0;1,0>:d            12:d               
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
         add     (4|M0)     a0.0<1>:uw          r22.0<4;4,1>:uw           0x600:uw           
         add     (4|M0)     a0.0<1>:uw          a0.0<4;4,1>:uw            r22.8<0;2,1>:uw    
         add     (16|M0)    (sat)r46.0<1>:uw    r[a0.1]<16;16,1>:uw       0x20:uw            
         add     (16|M0)    (sat)r47.0<1>:uw    r[a0.1,32]<16;16,1>:uw    0x20:uw
         add     (16|M0)    (sat)r48.0<1>:uw    r[a0.2]<16;8,2>:uw        0x20:uw            
         add     (16|M0)    (sat)r49.0<1>:uw    r[a0.0]<16;8,2>:uw        0x20:uw            
         and     (16|M0)    r46.0<1>:uw         r46.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r47.0<1>:uw         r47.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r48.0<1>:uw         r48.0<16;16,1>:uw         0xFFC0:uw          
         and     (16|M0)    r49.0<1>:uw         r49.0<16;16,1>:uw         0xFFC0:uw          
         mov     (16|M0)    r[a0.4]<2>:uw       r46.0<16;16,1>:uw         
         mov     (16|M0)    r[a0.4,64]<2>:uw    r47.0<16;16,1>:uw         
         mov     (8|M0)     r[a0.5]<4>:uw       r48.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.5,64]<4>:uw    r48.8<8;8,1>:uw           
         mov     (8|M0)     r[a0.6]<4>:uw       r49.0<8;8,1>:uw           
         mov     (8|M0)     r[a0.6,64]<4>:uw    r49.8<8;8,1>:uw           
         add     (1|M0)     r28.0<1>:ud         r27.0<0;1,0>:ud           32:d               
         add     (1|M0)     a0.0<1>:ud          r2.3<0;1,0>:ub            0xA0A8000:ud       
         send    (1|M0)     null:d              r28:ub                    0xC                a0.0    
