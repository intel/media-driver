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
(W&~f0.1)jmpi    L416       
L16:
         mov     (8|M0)     r17.0<1>:ud     r25.0<8;8,1>:ud    
         add     (1|M0)     a0.0<1>:ud      r23.5<0;1,0>:ud    0x42EC100:ud    
         mov     (1|M0)     r16.2<1>:ud     0xE000:ud          
         mov     (1|M0)     r17.2<1>:f      r10.1<0;1,0>:f     
         mov     (1|M0)     r17.3<1>:f      r10.7<0;1,0>:f     
         send    (1|M0)     r112:uw         r16:ub             0x2             a0.0    
         mov     (1|M0)     r17.2<1>:f      r10.6<0;1,0>:f     
         mov     (1|M0)     r17.3<1>:f      r10.7<0;1,0>:f     
         send    (1|M0)     r120:uw         r16:ub             0x2             a0.0    
         add     (1|M0)     a0.0<1>:ud      r23.5<0;1,0>:ud    0x44EC201:ud    
         mov     (1|M0)     r16.2<1>:ud     0xC000:ud          
         mov     (1|M0)     r17.2<1>:f      r10.1<0;1,0>:f     
         mov     (1|M0)     r17.3<1>:f      r10.7<0;1,0>:f     
         send    (1|M0)     r116:uw         r16:ub             0x2             a0.0    
         mov     (1|M0)     r17.2<1>:f      r10.6<0;1,0>:f     
         mov     (1|M0)     r17.3<1>:f      r10.7<0;1,0>:f     
         send    (1|M0)     r124:uw         r16:ub             0x2             a0.0    
         mov     (16|M0)    r114.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)    r115.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)    r122.0<1>:uw    0xFFFF:uw          
         mov     (16|M0)    r123.0<1>:uw    0xFFFF:uw          
         mov     (1|M0)     a0.8<1>:uw      0xE00:uw           
         mov     (1|M0)     a0.9<1>:uw      0xE80:uw           
         mov     (1|M0)     a0.10<1>:uw     0xEC0:uw           
         add     (4|M0)     a0.12<1>:uw     a0.8<4;4,1>:uw     0x100:uw        
L416:
         nop     
