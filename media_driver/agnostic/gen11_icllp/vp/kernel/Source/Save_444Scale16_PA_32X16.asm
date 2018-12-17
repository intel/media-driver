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
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw            
         add     (4|M0)                 a0.4<1>:w                r2.28<4;4,1>:ub           0x3A0:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw            
         mov     (8|M0)                 r[a0.4]<2>:ub            r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,32]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5]<4>:ub            r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,32]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6]<4>:ub            r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,32]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,96]<2>:ub         r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,128]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,96]<4>:ub         r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,128]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,96]<4>:ub         r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,128]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (16|M0)                (sat)r[a0.1]<1>:uw       r[a0.1]<16;16,1>:uw       0x80:uw
         add     (16|M0)                (sat)r[a0.1,32]<1>:uw    r[a0.1,32]<16;16,1>:uw    0x80:uw
         add     (8|M0)                 (sat)r[a0.2]<2>:uw       r[a0.2]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.2,32]<2>:uw    r[a0.2,32]<16;8,2>:uw     0x80:uw
         add     (8|M0)                 (sat)r[a0.0]<2>:uw       r[a0.0]<16;8,2>:uw        0x80:uw           
         add     (8|M0)                 (sat)r[a0.0,32]<2>:uw    r[a0.0,32]<16;8,2>:uw     0x80:uw
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             0x200:uw          
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x0:uw            
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.4,16]<2>:ub         r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,48]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,16]<4>:ub         r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,48]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,16]<4>:ub         r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,48]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,112]<2>:ub        r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,144]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,112]<4>:ub        r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,144]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,112]<4>:ub        r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,144]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         cmp     (1|M0)     (eq)f0.0    null.0<1>:uw             r19.4<0;1,0>:uw           0x0:uw            
(W&f0.0) jmpi    L1488       
L1408:
         mov     (8|M0)                 r41.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r43.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r45.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r47.0<1>:ud              r33.0<8;8,1>:ud           
(W)      jmpi    L1552       
L1488:
         mov     (8|M0)                 r42.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r43.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r44.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r45.0<1>:ud              r33.0<8;8,1>:ud           
L1552:
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)                 r[a0.4]<2>:ub            r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,32]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5]<4>:ub            r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,32]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6]<4>:ub            r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,32]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,96]<2>:ub         r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,128]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,96]<4>:ub         r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,128]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,96]<4>:ub         r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,128]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.4,16]<2>:ub         r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,48]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,16]<4>:ub         r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,48]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,16]<4>:ub         r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,48]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,112]<2>:ub        r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,144]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,112]<4>:ub        r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,144]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,112]<4>:ub        r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,144]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         cmp     (1|M0)     (eq)f0.0    null.0<1>:uw             r19.4<0;1,0>:uw           0x0:uw            
(W&f0.0) jmpi    L2080       
L2000:
         mov     (8|M0)                 r49.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r51.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r53.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r55.0<1>:ud              r33.0<8;8,1>:ud           
(W)      jmpi    L2272       
L2080:
         mov     (8|M0)                 r46.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r47.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r48.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r49.0<1>:ud              r33.0<8;8,1>:ud           
         mov     (8|M0)                 r14.0<1>:w               0x76543210:v              
         shl     (8|M0)                 r14.0<1>:w               r14.0<8;8,1>:w            4:w               
         mov     (4|M0)                 r40.0<1>:ud              0x0:ud                    
         mov     (4|M0)                 r40.4<1>:ud              0x800080:ud               
         add     (8|M0)                 r40.0<1>:uw              r40.0<8;8,1>:uw           r14.0<8;8,1>:w    
         add     (8|M0)                 r40.8<1>:uw              r40.8<8;8,1>:uw           r14.0<8;8,1>:w    
         add     (8|M0)                 r41.0<1>:ud              r40.0<8;8,1>:ud           0x1000100:ud      
         send    (16|M0)                null:w                   r40                       0xC               0x140250FE    
L2272:
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x400:uw          
         mov     (8|M0)                 r[a0.4]<2>:ub            r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,32]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5]<4>:ub            r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,32]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6]<4>:ub            r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,32]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,96]<2>:ub         r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,128]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,96]<4>:ub         r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,128]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,96]<4>:ub         r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,128]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.4,16]<2>:ub         r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,48]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,16]<4>:ub         r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,48]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,16]<4>:ub         r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,48]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,112]<2>:ub        r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,144]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,112]<4>:ub        r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,144]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,112]<4>:ub        r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,144]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         cmp     (1|M0)     (eq)f0.0    null.0<1>:uw             r19.4<0;1,0>:uw           0x0:uw            
(W&f0.0) jmpi    L2800       
L2720:
         mov     (8|M0)                 r57.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r59.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r61.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r63.0<1>:ud              r33.0<8;8,1>:ud           
(W)      jmpi    L2864       
L2800:
         mov     (8|M0)                 r42.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r43.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r44.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r45.0<1>:ud              r33.0<8;8,1>:ud           
L2864:
         add     (4|M0)                 a0.0<1>:uw               r22.0<4;4,1>:w            0x600:uw          
         mov     (8|M0)                 r[a0.4]<2>:ub            r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,32]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5]<4>:ub            r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,32]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6]<4>:ub            r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,32]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,96]<2>:ub         r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,128]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,96]<4>:ub         r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,128]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,96]<4>:ub         r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,128]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         add     (4|M0)                 a0.0<1>:w                a0.0<4;4,1>:w             r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.4,16]<2>:ub         r[a0.1,1]<16;8,2>:ub      
         mov     (8|M0)                 r[a0.4,48]<2>:ub         r[a0.1,17]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,16]<4>:ub         r[a0.2,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.5,48]<4>:ub         r[a0.2,17]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,16]<4>:ub         r[a0.0,1]<16;4,4>:ub      
         mov     (4|M0)                 r[a0.6,48]<4>:ub         r[a0.0,17]<16;4,4>:ub     
         mov     (8|M0)                 r[a0.4,112]<2>:ub        r[a0.1,33]<16;8,2>:ub     
         mov     (8|M0)                 r[a0.4,144]<2>:ub        r[a0.1,49]<16;8,2>:ub     
         mov     (4|M0)                 r[a0.5,112]<4>:ub        r[a0.2,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.5,144]<4>:ub        r[a0.2,49]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,112]<4>:ub        r[a0.0,33]<16;4,4>:ub     
         mov     (4|M0)                 r[a0.6,144]<4>:ub        r[a0.0,49]<16;4,4>:ub     
         cmp     (1|M0)     (eq)f0.0    null.0<1>:uw             r19.4<0;1,0>:uw           0x0:uw            
(W&f0.0) jmpi    L4064       
L3312:
         mov     (8|M0)                 r65.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r67.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r69.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r71.0<1>:ud              r33.0<8;8,1>:ud           
         mov     (8|M0)                 r14.0<1>:w               0x76543210:v              
         shl     (8|M0)                 r14.0<1>:w               r14.0<8;8,1>:w            4:w               
         mov     (4|M0)                 r30.0<1>:ud              0x0:ud                    
         mov     (4|M0)                 r30.4<1>:ud              0x800080:ud               
         add     (8|M0)                 r30.0<1>:uw              r30.0<8;8,1>:uw           r14.0<8;8,1>:w    
         add     (8|M0)                 r30.8<1>:uw              r30.8<8;8,1>:uw           r14.0<8;8,1>:w    
         add     (8|M0)                 r31.0<1>:ud              r30.0<8;8,1>:ud           0x1000100:ud      
         add     (8|M0)                 r34.0<1>:ud              r30.0<8;8,1>:ud           0x2000200:ud      
         add     (8|M0)                 r35.0<1>:ud              r30.0<8;8,1>:ud           0x3000300:ud      
         send    (16|M0)                r20                      r30:ub                    0xC               0x048050FE    
         send    (16|M0)                r28                      r34:ub                    0xC               0x048050FE    
         mov     (8|M0)                 r40.0<1>:ud              r20.0<8;8,1>:ud           
         mov     (8|M0)                 r42.0<1>:ud              r21.0<8;8,1>:ud           
         mov     (8|M0)                 r44.0<1>:ud              r22.0<8;8,1>:ud           
         mov     (8|M0)                 r46.0<1>:ud              r23.0<8;8,1>:ud           
         mov     (8|M0)                 r48.0<1>:ud              r24.0<8;8,1>:ud           
         mov     (8|M0)                 r50.0<1>:ud              r25.0<8;8,1>:ud           
         mov     (8|M0)                 r52.0<1>:ud              r26.0<8;8,1>:ud           
         mov     (8|M0)                 r54.0<1>:ud              r27.0<8;8,1>:ud           
         mov     (8|M0)                 r56.0<1>:ud              r28.0<8;8,1>:ud           
         mov     (8|M0)                 r58.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r60.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r62.0<1>:ud              r31.0<8;8,1>:ud           
         mov     (8|M0)                 r64.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r66.0<1>:ud              r33.0<8;8,1>:ud           
         mov     (8|M0)                 r68.0<1>:ud              r34.0<8;8,1>:ud           
         mov     (8|M0)                 r70.0<1>:ud              r35.0<8;8,1>:ud           
         mov     (8|M0)                 r39.0<1>:ud              0x0:ud                    
         subb    (1|M0)                 r39.0<1>:ud              r0.2<0;1,0>:w             0x10:uw           
         shl     (2|M0)                 r39.0<1>:ud              r39.0<2;2,1>:ud           0x1:uw            
         mov     (1|M0)                 r39.1<1>:ud              r0.3<0;1,0>:uw            
         mov     (1|M0)                 r39.2<1>:ud              0x3003F:ud                
         send    (1|M0)                 null:d                   r39                       0xC               0x120A8018    
         mov     (8|M0)                 r47.0<1>:ud              r39.0<8;8,1>:ud           
         add     (1|M0)                 r47.1<1>:ud              r47.1<0;1,0>:ud           0x4:ud            
         send    (1|M0)                 null:d                   r47                       0xC               0x120A8018    
         mov     (8|M0)                 r55.0<1>:ud              r47.0<8;8,1>:ud           
         add     (1|M0)                 r55.1<1>:ud              r55.1<0;1,0>:ud           0x4:ud            
         send    (1|M0)                 null:d                   r55                       0xC               0x120A8018    
         mov     (8|M0)                 r63.0<1>:ud              r55.0<8;8,1>:ud           
         add     (1|M0)                 r63.1<1>:ud              r63.1<0;1,0>:ud           0x4:ud            
         send    (1|M0)                 null:d                   r63                       0xC               0x120A8018    
(W)      jmpi    L4224       
L4064:
         mov     (8|M0)                 r46.0<1>:ud              r29.0<8;8,1>:ud           
         mov     (8|M0)                 r47.0<1>:ud              r30.0<8;8,1>:ud           
         mov     (8|M0)                 r48.0<1>:ud              r32.0<8;8,1>:ud           
         mov     (8|M0)                 r49.0<1>:ud              r33.0<8;8,1>:ud           
         add     (8|M0)                 r40.0<1>:ud              r40.0<8;8,1>:ud           0x2000200:ud      
         add     (8|M0)                 r41.0<1>:ud              r41.0<8;8,1>:ud           0x2000200:ud      
         send    (16|M0)                null:w                   r40                       0xC               0x140250FE    
         mov     (1|M0)                 r19.4<1>:uw              0x1:uw                    
L4192:
         add     (1|M0)                 r8.7<1>:d                ip.0<0;1,0>:ud            32:d              
(W)      jmpi    L4192       
L4224:
         nop     
