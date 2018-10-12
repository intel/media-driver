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
         add     (4|M0)                 a0.0<1>:uw           r22.0<4;4,1>:w             0x400:uw          {AccWrEn}
         add     (4|M0)                 a0.4<1>:uw           a0.0<4;4,1>:uw             r22.8<0;2,1>:w    
         and     (1|M0)                 r10.0<1>:ud          r2.2<0;1,0>:ud             0x1:ud            
         cmp     (1|M0)     (eq)f0.1    null.0<1>:w          r10.0<0;1,0>:ud            0x0:ud            
(W&f0.1) jmpi    L2032       
L80:
         add     (4|M0)                 a0.4<1>:uw           a0.4<4;4,1>:uw             r22.8<0;2,1>:w    
         mov     (16|M0)                r9.0<1>:uw           r[a0.2]<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw          r[a0.2,256]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.6]<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw          r[a0.6,256]<16;16,1>:uw    
         mov     (2|M0)                 r[a0.2]<2>:ud        r9.0<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,256]<2>:ud    r9.2<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,4]<2>:ud      r11.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,260]<2>:ud    r11.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6]<2>:ud        r10.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,256]<2>:ud    r10.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,4]<2>:ud      r12.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,260]<2>:ud    r12.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,16]<2>:ud     r9.4<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,272]<2>:ud    r9.6<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,20]<2>:ud     r11.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,276]<2>:ud    r11.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,16]<2>:ud     r10.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,272]<2>:ud    r10.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,20]<2>:ud     r12.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,276]<2>:ud    r12.6<2;2,1>:ud            
         mov     (16|M0)                r9.0<1>:uw           r[a0.2,32]<16;16,1>:uw     
         mov     (16|M0)                r10.0<1>:uw          r[a0.2,288]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.6,32]<16;16,1>:uw     
         mov     (16|M0)                r12.0<1>:uw          r[a0.6,288]<16;16,1>:uw    
         mov     (2|M0)                 r[a0.2,32]<2>:ud     r9.0<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,288]<2>:ud    r9.2<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,36]<2>:ud     r11.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,292]<2>:ud    r11.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,32]<2>:ud     r10.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,288]<2>:ud    r10.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,36]<2>:ud     r12.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,292]<2>:ud    r12.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,48]<2>:ud     r9.4<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,304]<2>:ud    r9.6<2;2,1>:ud             
         mov     (2|M0)                 r[a0.2,52]<2>:ud     r11.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.2,308]<2>:ud    r11.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,48]<2>:ud     r10.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,304]<2>:ud    r10.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,52]<2>:ud     r12.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.6,308]<2>:ud    r12.6<2;2,1>:ud            
         mov     (16|M0)                r9.0<1>:uw           r[a0.1]<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw          r[a0.1,256]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.5]<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw          r[a0.5,256]<16;16,1>:uw    
         mov     (4|M0)                 r[a0.1]<2>:uw        r9.0<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,256]<2>:uw    r9.4<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,2]<2>:uw      r11.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,258]<2>:uw    r11.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5]<2>:uw        r10.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,256]<2>:uw    r10.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,2]<2>:uw      r12.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,258]<2>:uw    r12.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,16]<2>:uw     r9.8<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,272]<2>:uw    r9.12<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,18]<2>:uw     r11.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,274]<2>:uw    r11.12<4;4,1>:uw           
         mov     (4|M0)                 r[a0.5,16]<2>:uw     r10.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,272]<2>:uw    r10.12<4;4,1>:uw           
         mov     (4|M0)                 r[a0.5,18]<2>:uw     r12.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,274]<2>:uw    r12.12<4;4,1>:uw           
         mov     (16|M0)                r9.0<1>:uw           r[a0.1,32]<16;16,1>:uw     
         mov     (16|M0)                r10.0<1>:uw          r[a0.1,288]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.5,32]<16;16,1>:uw     
         mov     (16|M0)                r12.0<1>:uw          r[a0.5,288]<16;16,1>:uw    
         mov     (4|M0)                 r[a0.1,32]<2>:uw     r9.0<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,288]<2>:uw    r9.4<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,34]<2>:uw     r11.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,290]<2>:uw    r11.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,32]<2>:uw     r10.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,288]<2>:uw    r10.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,34]<2>:uw     r12.0<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,290]<2>:uw    r12.4<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,48]<2>:uw     r9.8<4;4,1>:uw             
         mov     (4|M0)                 r[a0.1,304]<2>:uw    r9.12<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,50]<2>:uw     r11.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.1,306]<2>:uw    r11.12<4;4,1>:uw           
         mov     (4|M0)                 r[a0.5,48]<2>:uw     r10.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,304]<2>:uw    r10.12<4;4,1>:uw           
         mov     (4|M0)                 r[a0.5,50]<2>:uw     r12.8<4;4,1>:uw            
         mov     (4|M0)                 r[a0.5,306]<2>:uw    r12.12<4;4,1>:uw           
         mov     (16|M0)                r9.0<1>:uw           r[a0.0]<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw          r[a0.0,256]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.4]<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw          r[a0.4,256]<16;16,1>:uw    
         mov     (2|M0)                 r[a0.0]<2>:ud        r9.0<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,256]<2>:ud    r9.2<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,4]<2>:ud      r11.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,260]<2>:ud    r11.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4]<2>:ud        r10.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,256]<2>:ud    r10.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,4]<2>:ud      r12.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,260]<2>:ud    r12.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,16]<2>:ud     r9.4<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,272]<2>:ud    r9.6<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,20]<2>:ud     r11.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,276]<2>:ud    r11.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,16]<2>:ud     r10.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,272]<2>:ud    r10.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,20]<2>:ud     r12.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,276]<2>:ud    r12.6<2;2,1>:ud            
         mov     (16|M0)                r9.0<1>:uw           r[a0.0,32]<16;16,1>:uw     
         mov     (16|M0)                r10.0<1>:uw          r[a0.0,288]<16;16,1>:uw    
         mov     (16|M0)                r11.0<1>:uw          r[a0.4,32]<16;16,1>:uw     
         mov     (16|M0)                r12.0<1>:uw          r[a0.4,288]<16;16,1>:uw    
         mov     (2|M0)                 r[a0.0,32]<2>:ud     r9.0<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,288]<2>:ud    r9.2<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,36]<2>:ud     r11.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,292]<2>:ud    r11.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,32]<2>:ud     r10.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,288]<2>:ud    r10.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,36]<2>:ud     r12.0<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,292]<2>:ud    r12.2<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,48]<2>:ud     r9.4<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,304]<2>:ud    r9.6<2;2,1>:ud             
         mov     (2|M0)                 r[a0.0,52]<2>:ud     r11.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.0,308]<2>:ud    r11.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,48]<2>:ud     r10.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,304]<2>:ud    r10.6<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,52]<2>:ud     r12.4<2;2,1>:ud            
         mov     (2|M0)                 r[a0.4,308]<2>:ud    r12.6<2;2,1>:ud            
(W)      jmpi    L2496       
L2032:
         mov     (16|M0)                r13.0<1>:uw          r[a0.1,32]<16;16,1>:uw     
         mov     (16|M0)                r14.0<1>:uw          r[a0.5,32]<16;16,1>:uw     
         mov     (8|M0)                 r[a0.1,32]<1>:uw     r[a0.1,16]<8;8,1>:uw       
         mov     (8|M0)                 r[a0.5,32]<1>:uw     r[a0.5,16]<8;8,1>:uw       
         mov     (16|M0)                r9.0<1>:uw           r[a0.0,32]<16;16,1>:uw     
         mov     (16|M0)                r10.0<1>:uw          r[a0.2,32]<16;16,1>:uw     
         mov     (16|M0)                r11.0<1>:uw          r[a0.4,32]<16;16,1>:uw     
         mov     (16|M0)                r12.0<1>:uw          r[a0.6,32]<16;16,1>:uw     
         add     (4|M0)                 a0.4<1>:uw           r22.0<4;4,1>:w             0x600:uw          
         mov     (8|M0)                 r[a0.1,16]<1>:uw     r[a0.5]<8;8,1>:uw          
         mov     (8|M0)                 r[a0.1,48]<1>:uw     r[a0.5,16]<8;8,1>:uw       
         mov     (16|M0)                r[a0.0,32]<1>:uw     r[a0.4]<16;16,1>:uw        
         mov     (16|M0)                r[a0.2,32]<1>:uw     r[a0.6]<16;16,1>:uw        
         mov     (16|M0)                r[a0.4]<1>:uw        r9.0<16;16,1>:uw           
         mov     (16|M0)                r[a0.6]<1>:uw        r10.0<16;16,1>:uw          
         add     (8|M0)                 a0.0<1>:uw           a0.0<8;8,1>:uw             r22.8<0;2,1>:w    
         mov     (8|M0)                 r[a0.1,16]<1>:uw     r[a0.5]<8;8,1>:uw          
         mov     (8|M0)                 r[a0.1,48]<1>:uw     r[a0.5,16]<8;8,1>:uw       
         mov     (16|M0)                r[a0.0,32]<1>:uw     r[a0.4]<16;16,1>:uw        
         mov     (16|M0)                r[a0.2,32]<1>:uw     r[a0.6]<16;16,1>:uw        
         mov     (16|M0)                r[a0.4]<1>:uw        r11.0<16;16,1>:uw          
         mov     (16|M0)                r[a0.6]<1>:uw        r12.0<16;16,1>:uw          
         add     (4|M0)                 a0.0<1>:uw           r22.0<4;4,1>:w             0x600:uw          
         mov     (8|M0)                 r[a0.1]<1>:uw        r13.0<8;8,1>:uw            
         mov     (8|M0)                 r[a0.5]<1>:uw        r14.0<8;8,1>:uw            
         mov     (8|M0)                 r[a0.1,16]<1>:uw     r[a0.1,32]<8;8,1>:uw       
         mov     (8|M0)                 r[a0.5,16]<1>:uw     r[a0.5,32]<8;8,1>:uw       
         mov     (8|M0)                 r[a0.1,32]<1>:uw     r13.8<8;8,1>:uw            
         mov     (8|M0)                 r[a0.5,32]<1>:uw     r14.8<8;8,1>:uw            
L2496:
         nop     
