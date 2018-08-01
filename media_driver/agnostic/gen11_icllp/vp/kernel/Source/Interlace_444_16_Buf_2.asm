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
         add     (4|M0)                 a0.0<1>:uw          r22.0<4;4,1>:w            0x400:uw          {AccWrEn}
         add     (4|M0)                 a0.4<1>:uw          a0.0<4;4,1>:uw            r22.8<0;2,1>:w    
         and     (1|M0)                 r10.0<1>:ud         r2.2<0;1,0>:ud            0x1:ud            
         cmp     (1|M0)     (eq)f0.1    null.0<1>:w         r10.0<0;1,0>:ud           0x0:ud            
(W&f0.1) jmpi    L2656       
L80:
         mov     (16|M0)                r9.0<1>:uw          r96.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r104.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r112.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r120.0<16;16,1>:uw        
         mov     (4|M0)                 r96.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r104.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r96.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r104.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r112.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r120.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r112.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r120.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r96.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r104.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r96.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r104.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r112.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r120.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r112.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r120.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r97.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r105.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r113.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r121.0<16;16,1>:uw        
         mov     (4|M0)                 r97.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r105.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r97.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r105.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r113.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r121.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r113.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r121.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r97.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r105.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r97.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r105.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r113.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r121.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r113.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r121.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r98.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r106.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r114.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r122.0<16;16,1>:uw        
         mov     (4|M0)                 r98.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r106.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r98.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r106.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r114.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r122.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r114.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r122.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r98.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r106.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r98.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r106.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r114.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r122.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r114.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r122.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r99.0<16;16,1>:uw         
         mov     (16|M0)                r10.0<1>:uw         r107.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r115.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r123.0<16;16,1>:uw        
         mov     (4|M0)                 r99.0<2>:uw         r9.0<4;4,1>:uw            
         mov     (4|M0)                 r107.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r99.1<2>:uw         r11.0<4;4,1>:uw           
         mov     (4|M0)                 r107.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r115.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r123.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r115.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r123.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r99.8<2>:uw         r9.8<4;4,1>:uw            
         mov     (4|M0)                 r107.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r99.9<2>:uw         r11.8<4;4,1>:uw           
         mov     (4|M0)                 r107.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r115.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r123.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r115.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r123.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r100.0<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw         r108.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r116.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r124.0<16;16,1>:uw        
         mov     (4|M0)                 r100.0<2>:uw        r9.0<4;4,1>:uw            
         mov     (4|M0)                 r108.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r100.1<2>:uw        r11.0<4;4,1>:uw           
         mov     (4|M0)                 r108.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r116.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r124.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r116.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r124.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r100.8<2>:uw        r9.8<4;4,1>:uw            
         mov     (4|M0)                 r108.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r100.9<2>:uw        r11.8<4;4,1>:uw           
         mov     (4|M0)                 r108.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r116.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r124.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r116.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r124.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r101.0<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw         r109.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r117.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r125.0<16;16,1>:uw        
         mov     (4|M0)                 r101.0<2>:uw        r9.0<4;4,1>:uw            
         mov     (4|M0)                 r109.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r101.1<2>:uw        r11.0<4;4,1>:uw           
         mov     (4|M0)                 r109.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r117.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r125.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r117.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r125.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r101.8<2>:uw        r9.8<4;4,1>:uw            
         mov     (4|M0)                 r109.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r101.9<2>:uw        r11.8<4;4,1>:uw           
         mov     (4|M0)                 r109.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r117.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r125.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r117.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r125.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r102.0<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw         r110.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r118.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r126.0<16;16,1>:uw        
         mov     (4|M0)                 r102.0<2>:uw        r9.0<4;4,1>:uw            
         mov     (4|M0)                 r110.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r102.1<2>:uw        r11.0<4;4,1>:uw           
         mov     (4|M0)                 r110.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r118.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r126.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r118.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r126.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r102.8<2>:uw        r9.8<4;4,1>:uw            
         mov     (4|M0)                 r110.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r102.9<2>:uw        r11.8<4;4,1>:uw           
         mov     (4|M0)                 r110.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r118.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r126.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r118.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r126.9<2>:uw        r12.12<4;4,1>:uw          
         mov     (16|M0)                r9.0<1>:uw          r103.0<16;16,1>:uw        
         mov     (16|M0)                r10.0<1>:uw         r111.0<16;16,1>:uw        
         mov     (16|M0)                r11.0<1>:uw         r119.0<16;16,1>:uw        
         mov     (16|M0)                r12.0<1>:uw         r127.0<16;16,1>:uw        
         mov     (4|M0)                 r103.0<2>:uw        r9.0<4;4,1>:uw            
         mov     (4|M0)                 r111.0<2>:uw        r9.4<4;4,1>:uw            
         mov     (4|M0)                 r103.1<2>:uw        r11.0<4;4,1>:uw           
         mov     (4|M0)                 r111.1<2>:uw        r11.4<4;4,1>:uw           
         mov     (4|M0)                 r119.0<2>:uw        r10.0<4;4,1>:uw           
         mov     (4|M0)                 r127.0<2>:uw        r10.4<4;4,1>:uw           
         mov     (4|M0)                 r119.1<2>:uw        r12.0<4;4,1>:uw           
         mov     (4|M0)                 r127.1<2>:uw        r12.4<4;4,1>:uw           
         mov     (4|M0)                 r103.8<2>:uw        r9.8<4;4,1>:uw            
         mov     (4|M0)                 r111.8<2>:uw        r9.12<4;4,1>:uw           
         mov     (4|M0)                 r103.9<2>:uw        r11.8<4;4,1>:uw           
         mov     (4|M0)                 r111.9<2>:uw        r11.12<4;4,1>:uw          
         mov     (4|M0)                 r119.8<2>:uw        r10.8<4;4,1>:uw           
         mov     (4|M0)                 r127.8<2>:uw        r10.12<4;4,1>:uw          
         mov     (4|M0)                 r119.9<2>:uw        r12.8<4;4,1>:uw           
         mov     (4|M0)                 r127.9<2>:uw        r12.12<4;4,1>:uw          
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
         add     (4|M0)                 a0.4<1>:uw          r22.0<4;4,1>:w            0x600:uw          
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
         add     (4|M0)                 a0.0<1>:uw          r22.0<4;4,1>:w            0x600:uw          
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
