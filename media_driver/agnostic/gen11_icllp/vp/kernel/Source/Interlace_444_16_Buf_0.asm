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
         add     (4|M0)                 a0.0<1>:uw          r22.0<4;4,1>:w            0x0:uw            {AccWrEn}
         add     (4|M0)                 a0.4<1>:uw          a0.0<4;4,1>:uw            r22.8<0;2,1>:w    
         and     (1|M0)                 r10.0<1>:ud         r2.2<0;1,0>:ud            0x1:ud            
         cmp     (1|M0)     (eq)f0.1    null.0<1>:w         r10.0<0;1,0>:ud           0x0:ud            
(W&f0.1) jmpi    L2656       
L80:
         mov     (16|M0)                r9.0<1>:uw          r64.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r72.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r80.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r88.0<16;16,1>:uw         
         mov     (4|M0)                 r64.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r72.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r64.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r72.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r80.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r88.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r80.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r88.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r64.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r72.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r64.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r72.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r80.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r88.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r80.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r88.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r65.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r73.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r81.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r89.0<16;16,1>:uw         
         mov     (4|M0)                 r65.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r73.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r65.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r73.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r81.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r89.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r81.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r89.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r65.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r73.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r65.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r73.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r81.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r89.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r81.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r89.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r66.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r74.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r82.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r90.0<16;16,1>:uw         
         mov     (4|M0)                 r66.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r74.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r66.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r74.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r82.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r90.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r82.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r90.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r66.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r74.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r66.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r74.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r82.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r90.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r82.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r90.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r67.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r75.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r83.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r91.0<16;16,1>:uw         
         mov     (4|M0)                 r67.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r75.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r67.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r75.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r83.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r91.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r83.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r91.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r67.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r75.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r67.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r75.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r83.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r91.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r83.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r91.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r68.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r76.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r84.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r92.0<16;16,1>:uw         
         mov     (4|M0)                 r68.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r76.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r68.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r76.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r84.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r92.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r84.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r92.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r68.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r76.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r68.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r76.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r84.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r92.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r84.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r92.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r69.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r77.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r85.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r93.0<16;16,1>:uw         
         mov     (4|M0)                 r69.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r77.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r69.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r77.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r85.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r93.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r85.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r93.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r69.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r77.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r69.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r77.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r85.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r93.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r85.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r93.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r70.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r78.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r86.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r94.0<16;16,1>:uw         
         mov     (4|M0)                 r70.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r78.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r70.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r78.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r86.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r94.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r86.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r94.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r70.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r78.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r70.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r78.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r86.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r94.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r86.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r94.9<2>:uw         r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r71.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r79.0<16;16,1>:uw         
         mov     (16|M0)                r11.0<1>:uw         r87.0<16;16,1>:uw         
         mov     (16|M0)                r12.0<1>:uw         r95.0<16;16,1>:uw         
         mov     (4|M0)                 r71.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r79.0<2>:uw         r9.4<4;4,1>:uw            
         mov     (4|M0)                 r71.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r79.1<2>:uw         r11.4<4;4,1>:uw           
         mov     (4|M0)                 r87.0<2>:uw         r10.0<4;4,1>:uw           
         mov     (4|M0)                 r95.0<2>:uw         r10.4<4;4,1>:uw           
         mov     (4|M0)                 r87.1<2>:uw         r12.0<4;4,1>:uw           
         mov     (4|M0)                 r95.1<2>:uw         r12.4<4;4,1>:uw           
         mov     (4|M0)                 r71.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r79.8<2>:uw         r9.12<4;4,1>:uw           
         mov     (4|M0)                 r71.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r79.9<2>:uw         r11.12<4;4,1>:uw          
         mov     (4|M0)                 r87.8<2>:uw         r10.8<4;4,1>:uw           
         mov     (4|M0)                 r95.8<2>:uw         r10.12<4;4,1>:uw          
         mov     (4|M0)                 r87.9<2>:uw         r12.8<4;4,1>:uw           
         mov     (4|M0)                 r95.9<2>:uw         r12.12<4;4,1>:uw          
(W)      jmpi    L3600       
L2656:
         mov     (16|M0)                r9.0<1>:uw          r[a0.0,32]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw         r[a0.1,32]<16;16,1>:uw    
         mov     (16|M0)                r13.0<1>:uw         r[a0.2,32]<16;16,1>:uw    
         mov     (16|M0)                r15.0<1>:uw         r[a0.3,32]<16;16,1>:uw    
         mov     (16|M0)                r10.0<1>:uw         r[a0.4,32]<16;16,1>:uw    
         mov     (16|M0)                r12.0<1>:uw         r[a0.5,32]<16;16,1>:uw    
         mov     (16|M0)                r14.0<1>:uw         r[a0.6,32]<16;16,1>:uw    
         mov     (16|M0)                r16.0<1>:uw         r[a0.7,32]<16;16,1>:uw    
         mov     (8|M0)                 r[a0.0,32]<1>:uw    r[a0.0,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.1,32]<1>:uw    r[a0.1,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.2,32]<1>:uw    r[a0.2,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.3,32]<1>:uw    r[a0.3,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.4,32]<1>:uw    r[a0.4,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.5,32]<1>:uw    r[a0.5,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.6,32]<1>:uw    r[a0.6,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.7,32]<1>:uw    r[a0.7,16]<8;8,1>:uw      
         add     (4|M0)                 a0.4<1>:uw          r22.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)                 r[a0.0,16]<1>:uw    r[a0.4]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.0,48]<1>:uw    r[a0.4,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.1,16]<1>:uw    r[a0.5]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.1,48]<1>:uw    r[a0.5,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.2,16]<1>:uw    r[a0.6]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.2,48]<1>:uw    r[a0.6,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.3,16]<1>:uw    r[a0.7]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.3,48]<1>:uw    r[a0.7,16]<8;8,1>:uw      
         add     (8|M0)                 a0.0<1>:uw          a0.0<8;8,1>:uw            r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.0,16]<1>:uw    r[a0.4]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.0,48]<1>:uw    r[a0.4,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.1,16]<1>:uw    r[a0.5]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.1,48]<1>:uw    r[a0.5,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.2,16]<1>:uw    r[a0.6]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.2,48]<1>:uw    r[a0.6,16]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.3,16]<1>:uw    r[a0.7]<8;8,1>:uw         
         mov     (8|M0)                 r[a0.3,48]<1>:uw    r[a0.7,16]<8;8,1>:uw      
         add     (4|M0)                 a0.0<1>:uw          r22.0<4;4,1>:w            0x200:uw          
         mov     (8|M0)                 r[a0.0]<1>:uw       r9.0<8;8,1>:uw            
         mov     (8|M0)                 r[a0.1]<1>:uw       r11.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.2]<1>:uw       r13.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.3]<1>:uw       r15.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.4]<1>:uw       r10.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.5]<1>:uw       r12.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.6]<1>:uw       r14.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.7]<1>:uw       r16.0<8;8,1>:uw           
         mov     (8|M0)                 r[a0.0,16]<1>:uw    r[a0.0,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.1,16]<1>:uw    r[a0.1,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.2,16]<1>:uw    r[a0.2,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.3,16]<1>:uw    r[a0.3,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.4,16]<1>:uw    r[a0.4,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.5,16]<1>:uw    r[a0.5,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.6,16]<1>:uw    r[a0.6,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.7,16]<1>:uw    r[a0.7,32]<8;8,1>:uw      
         mov     (8|M0)                 r[a0.0,32]<1>:uw    r9.8<8;8,1>:uw            
         mov     (8|M0)                 r[a0.1,32]<1>:uw    r11.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.2,32]<1>:uw    r13.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.3,32]<1>:uw    r15.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.4,32]<1>:uw    r10.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.5,32]<1>:uw    r12.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.6,32]<1>:uw    r14.8<8;8,1>:uw           
         mov     (8|M0)                 r[a0.7,32]<1>:uw    r16.8<8;8,1>:uw           
L3600:
         nop     
