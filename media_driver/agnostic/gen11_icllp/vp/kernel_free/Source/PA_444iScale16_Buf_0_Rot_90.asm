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
         mov     (8|M0)    r16.0<1>:ud    r0.0<8;8,1>:ud     
         mov     (1|M0)    r22.4<1>:ud    0x1000100:ud       
         mov     (4|M0)    acc0.0<1>:w    0x6420:v           
         add     (4|M0)    acc0.0<1>:w    acc0.0<4;4,1>:w    0x40:uw         
         shl     (4|M0)    r22.0<1>:w     acc0.0<4;4,1>:w    0x5:uw          
(W&~f0.1)jmpi    L288      
L96:
         mul     (1|M0)    r25.5<1>:f     r25.5<0;1,0>:f     2.0:f           
         mov     (8|M0)    r17.0<1>:ud    r25.0<8;8,1>:ud    
         mov     (1|M0)    r16.2<1>:ud    0x0:ud             
         add     (1|M0)    a0.0<1>:ud     r23.5<0;1,0>:ud    0x48EC100:ud    
         send    (1|M0)    r64:uw         r16:ub             0x2             a0.0    
         mov     (1|M0)    r17.2<1>:f     r10.0<0;1,0>:f     
         send    (1|M0)    r72:uw         r16:ub             0x2             a0.0    
         mov     (1|M0)    a0.8<1>:uw     0x800:uw           
         mov     (1|M0)    a0.9<1>:uw     0x840:uw           
         mov     (1|M0)    a0.10<1>:uw    0x880:uw           
         mov     (1|M0)    a0.11<1>:uw    0x8C0:uw           
         add     (4|M0)    a0.12<1>:uw    a0.8<4;4,1>:uw     0x100:uw        
L288:
         nop     
