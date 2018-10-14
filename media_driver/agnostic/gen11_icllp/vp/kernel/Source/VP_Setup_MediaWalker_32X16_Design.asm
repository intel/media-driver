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
              mov     (1|M0)                 r19.4<1>:uw     0x0:uw               
              mov     (8|M0)                 r27.0<1>:ud     r0.0<8;8,1>:ud       
              and     (1|M0)                 r13.0<1>:ud     r0.1<0;1,0>:ud       0xFFFFFF:ud          
              and     (1|M0)                 r0.2<1>:uw      r13.0<0;1,0>:uw      0xFFF:uw             
              shr     (1|M0)                 r13.0<1>:ud     r13.0<0;1,0>:ud      0xC:uw               
              mov     (1|M0)                 r0.3<1>:uw      r13.0<0;1,0>:uw      
              cmp     (1|M0)     (eq)f0.0    null.0<1>:uw    r19.4<0;1,0>:uw      0x0:uw               
(W&f0.0)      jmpi    L304        
L128:
              mov     (8|M0)                 r14.0<1>:w      0x76543210:v         
              shl     (8|M0)                 r14.0<1>:w      r14.0<8;8,1>:w       4:w                  
              mov     (4|M0)                 r20.0<1>:ud     0x4000400:ud         
              mov     (4|M0)                 r20.4<1>:ud     0x4800480:ud         
              add     (8|M0)                 r20.0<1>:uw     r20.0<8;8,1>:uw      r14.0<8;8,1>:w       
              add     (8|M0)                 r20.8<1>:uw     r20.8<8;8,1>:uw      r14.0<8;8,1>:w       
              add     (8|M0)                 r21.0<1>:ud     r20.0<8;8,1>:ud      0x1000100:ud         
              send    (16|M0)                r2              r20:ub               0xC                  0x048050FE         
              mov     (1|M0)                 r0.1<1>:ud      r9.3<0;1,0>:ud       
              add     (1|M0)                 r0.2<1>:uw      r0.2<0;1,0>:uw       0x1:uw               
(W)           jmpi    L576        
L304:
              mov     (1|M0)                 r9.3<1>:ud      r0.1<0;1,0>:ud       
              mov     (8|M0)                 r14.0<1>:w      0x76543210:v         
              shl     (8|M0)                 r14.0<1>:w      r14.0<8;8,1>:w       4:w                  
              mov     (4|M0)                 r20.0<1>:ud     0x4000400:ud         
              mov     (4|M0)                 r20.4<1>:ud     0x4800480:ud         
              add     (8|M0)                 r20.0<1>:uw     r20.0<8;8,1>:uw      r14.0<8;8,1>:w       
              add     (8|M0)                 r20.8<1>:uw     r20.8<8;8,1>:uw      r14.0<8;8,1>:w       
              add     (8|M0)                 r21.0<1>:ud     r20.0<8;8,1>:ud      0x1000100:ud         
              mov     (8|M0)                 r22.0<1>:ud     r2.0<8;8,1>:ud       
              mov     (8|M0)                 r23.0<1>:ud     r3.0<8;8,1>:ud       
              mov     (8|M0)                 r24.0<1>:ud     r4.0<8;8,1>:ud       
              mov     (8|M0)                 r25.0<1>:ud     r5.0<8;8,1>:ud       
              mov     (8|M0)                 r26.0<1>:ud     r6.0<8;8,1>:ud       
              mov     (8|M0)                 r27.0<1>:ud     r7.0<8;8,1>:ud       
              mov     (8|M0)                 r28.0<1>:ud     r8.0<8;8,1>:ud       
              mov     (8|M0)                 r29.0<1>:ud     r9.0<8;8,1>:ud       
              send    (16|M0)                null:w          r20:ub               0xC                  0x140250FE         
L576:
              mov     (8|M0)                 r17.0<1>:ud     0xFFFFFFFF:ud        
              mov     (8|M0)                 r16.0<1>:ud     0xFFFFFFFF:ud        
              mov     (8|M0)                 r15.0<1>:ud     0xFFFFFFFF:ud        
              shl     (2|M0)                 r0.2<1>:uw      r0.2<2;2,1>:uw       0x4:uw               
              add     (2|M0)                 r13.0<1>:w      r0.2<2;2,1>:uw       0xF:uw               
              cmp     (2|M0)                 (lt)f0.0        null.0<1>:uw         r13.0<2;2,1>:uw      r9.10<2;2,1>:uw    
(W&f0.0.any2h)jmpi    L704        
L688:
(W)           jmpi    L736        
L704:
              mov     (8|M0)                 r127.0<1>:ud    r0.0<8;8,1>:ud       
              send    (1|M0)                 null:d          r127:ub              0x27                 0x02000010         {EOT} //  SPAWNER  wr:1, rd:0, fc: 0x10
L736:
              cmp     (2|M0)                 (lt)f0.0        null.0<1>:uw         r0.2<2;2,1>:uw       r9.10<2;2,1>:uw    
(f0.0)        cmp     (2|M0)                 (ge)f0.0        null.0<1>:uw         r13.0<2;2,1>:uw      r9.10<2;2,1>:uw    
(f0.0)        mov     (2|M0)                 r0.2<1>:uw      r9.10<2;2,1>:uw      
              cmp     (16|M0)                (lt)f0.0        null.0<1>:w          r0.2<0;2,1>:w        r7.0<16;16,1>:w    
(f0.0)        subb    (16|M0)                r16.0<1>:w      r7.0<16;16,1>:w      r0.2<0;2,1>:w        {AccWrEn}
              cmp     (16|M0)                (gt)f0.1        null.0<1>:w          r16.0<16;16,1>:w     16:w               
(f0.1)        mov     (16|M0)                r16.0<1>:w      16:w                 
(f0.0)        shl     (16|M0)                r16.0<1>:uw     r17.0<16;16,1>:uw    r16.0<16;16,1>:uw
              add     (16|M0)                r13.0<1>:w      r0.2<0;2,1>:uw       0xF:uw               
              cmp     (16|M0)                (gt)f0.0        null.0<1>:w          r13.0<16;16,1>:w     r8.0<16;16,1>:w    
(f0.0)        subb    (16|M0)                r14.0<1>:w      r13.0<16;16,1>:w     r8.0<16;16,1>:w      
              cmp     (16|M0)                (gt)f0.1        null.0<1>:w          r14.0<16;16,1>:w     16:w               
(f0.1)        mov     (16|M0)                r14.0<1>:w      16:w                 
(f0.0)        shr     (16|M0)                r15.0<1>:uw     r17.0<16;16,1>:uw    r14.0<16;16,1>:uw
              and     (16|M0)                r17.0<1>:uw     r16.0<16;16,1>:uw    r15.0<16;16,1>:uw
              add     (1|M0)                 r9.4<1>:uw      r9.4<0;1,0>:uw       0x8:uw               
              shr     (1|M0)                 r9.4<1>:uw      r9.4<0;1,0>:uw       0x4:uw               
              shl     (1|M0)                 r9.4<1>:uw      r9.4<0;1,0>:uw       0x4:uw               
              add     (1|M0)                 r9.5<1>:uw      r9.5<0;1,0>:uw       0x8:uw               
              shr     (1|M0)                 r9.5<1>:uw      r9.5<0;1,0>:uw       0x4:uw               
              shl     (1|M0)                 r9.5<1>:uw      r9.5<0;1,0>:uw       0x4:uw               
              mov     (2|M0)                 r8.3<1>:f       r9.4<2;2,1>:w        
              cmp     (1|M0)     (eq)f0.0    null.0<1>:w     r2.26<0;1,0>:ub      0x1:uw               
(W&~f0.0)     jmpi    L1920       
L1120:
              shr     (1|M0)                 r14.6<1>:w      r0.2<0;1,0>:w        4:w                  
              mov     (1|M0)                 r14.6<1>:f      r14.6<0;1,0>:w       
              mov     (1|M0)                 r8.5<1>:f       r14.6<0;1,0>:w       
              mov     (1|M0)                 r13.6<1>:f      r0.2<0;1,0>:w        
              shr     (2|M0)                 r13.0<1>:w      r9.4<2;2,1>:w        4:w                  
              mov     (2|M0)                 r13.1<1>:f      r13.0<2;2,1>:w       
              cmp     (1|M0)                 (lt)f0.0        null.0<1>:w          r0.2<0;1,0>:w        r9.4<0;1,0>:w      
(~f0.0)       mov     (1|M0)                 r14.6<1>:w      r13.0<0;1,0>:w       
(~f0.0)       mov     (1|M0)                 r13.6<1>:f      r9.4<0;1,0>:w        
              subb    (1|M0)                 r14.7<1>:d      r14.6<0;1,0>:w       1:w                  
              mov     (1|M0)                 r14.5<1>:f      r14.7<0;1,0>:d       
              mul     (1|M0)                 r14.5<1>:f      r14.5<0;1,0>:f       r9.1<0;1,0>:f        
              mul     (1|M0)                 r14.5<1>:f      r14.5<0;1,0>:f       0.5:f                
              add     (1|M0)                 r14.5<1>:f      r14.5<0;1,0>:f       r9.0<0;1,0>:f        
              mul     (1|M0)                 r14.5<1>:f      r13.6<0;1,0>:f       r14.5<0;1,0>:f       
              mul     (8|M0)                 r16.0<1>:f      r14.5<0;1,0>:f       r3.0<8;8,1>:f        
(W&~f0.0)     jmpi    L1456       
L1392:
              mov     (1|M0)                 acc0.0<1>:f     r9.0<0;1,0>:f        
              mac     (1|M0)                 r14.0<1>:f      r8.5<0;1,0>:f        r9.1<0;1,0>:f        
              mov     (1|M0)                 r9.0<1>:f       r14.0<0;1,0>:f       
(W)           jmpi    L1888       
L1456:
              cmp     (1|M0)                 (lt)f0.0        null.0<1>:w          r0.2<0;1,0>:w        r9.5<0;1,0>:w      
(~f0.0)       mov     (1|M0)                 r14.6<1>:w      r9.5<0;1,0>:w        
(f0.0)        mov     (1|M0)                 r14.6<1>:w      r0.2<0;1,0>:w        
              subb    (1|M0)                 r14.6<1>:w      r14.6<0;1,0>:w       r9.4<0;1,0>:w        
              mov     (1|M0)                 r14.7<1>:f      r14.6<0;1,0>:w       
              mul     (1|M0)                 r14.7<1>:f      r9.3<0;1,0>:f        r14.7<0;1,0>:f       
              mul     (8|M0)                 r15.0<1>:f      r14.7<0;1,0>:f       r3.0<8;8,1>:f        
              add     (8|M0)                 r16.0<1>:f      r15.0<8;8,1>:f       r16.0<8;8,1>:f       
(W&~f0.0)     jmpi    L1632       
L1600:
              mov     (1|M0)                 r9.0<1>:f       r9.3<0;1,0>:f        
(W)           jmpi    L1888       
L1632:
              shr     (1|M0)                 r14.6<1>:w      r0.2<0;1,0>:w        4:w                  
              subb    (1|M0)                 r14.6<1>:w      r13.1<0;1,0>:w       r14.6<0;1,0>:w       
              add     (1|M0)                 r14.7<1>:d      r14.6<0;1,0>:w       1:w                  
              mov     (1|M0)                 r13.5<1>:f      r14.7<0;1,0>:d       
              mul     (1|M0)                 r13.5<1>:f      r13.5<0;1,0>:f       r9.1<0;1,0>:f        
              mul     (1|M0)                 r13.5<1>:f      r13.5<0;1,0>:f       0.5:f                
              add     (1|M0)                 r13.5<1>:f      r13.5<0;1,0>:f       r9.4<0;1,0>:f        
              subb    (1|M0)                 r13.6<1>:w      r0.2<0;1,0>:w        r9.5<0;1,0>:w        
              mov     (1|M0)                 r13.6<1>:f      r13.6<0;1,0>:w       
              mul     (1|M0)                 r13.5<1>:f      r13.6<0;1,0>:f       r13.5<0;1,0>:f       
              mul     (8|M0)                 r15.0<1>:f      r13.5<0;1,0>:f       r3.0<8;8,1>:f        
              add     (8|M0)                 r16.0<1>:f      r16.0<8;8,1>:f       r15.0<8;8,1>:f       
              mov     (1|M0)                 r14.6<1>:f      r14.6<0;1,0>:w       
              mov     (1|M0)                 acc0.0<1>:f     r9.4<0;1,0>:f        
              mac     (1|M0)                 r14.0<1>:f      r14.6<0;1,0>:f       r9.1<0;1,0>:f        
              mov     (1|M0)                 r9.0<1>:f       r14.0<0;1,0>:f       
L1888:
              add     (8|M0)                 r6.0<1>:f       r6.0<8;8,1>:f        r16.0<8;8,1>:f       
              mov     (1|M0)                 r9.1<1>:f       0.0:f                
L1920:
              mov     (1|M0)                 r7.0<1>:ud      r0.1<0;1,0>:ud       
              mov     (2|M0)                 r7.1<1>:ud      r17.0<2;2,1>:ud      
              mov     (1|M0)                 r7.3<1>:ud      r17.2<0;1,0>:ud      
              mov     (4|M0)                 r8.0<1>:ud      r17.3<4;4,1>:ud      
              mov     (1|M0)                 r8.4<1>:ud      r17.7<0;1,0>:ud      
              mov     (2|M0)                 r7.4<1>:f       r9.0<2;2,1>:f        
              mov     (1|M0)                 r7.6<1>:ud      0x0:ud               
              mov     (1|M0)                 r7.7<1>:ud      0x0:ud               
              mov     (8|M0)                 r25.0<1>:ud     r0.0<8;8,1>:ud       
              mov     (8|M0)                 r26.0<1>:ud     r0.0<8;8,1>:ud       
              mul     (8|M0)                 r3.0<1>:f       r3.0<8;8,1>:f        r7.4<0;1,0>:f        
              mov     (2|M0)                 r8.5<1>:f       r7.0<2;2,1>:w        
              cmp     (8|M0)     (eq)f0.0    null.0<1>:w     r2.26<0;1,0>:ub      0x1:uw               
              shr     (1|M0)                 r13.0<1>:uw     r2.2<0;1,0>:ud       0x0:ud               
              shr     (1|M0)                 r13.1<1>:uw     r2.2<0;1,0>:ud       0x3:ud               
              shr     (1|M0)                 r13.2<1>:uw     r2.2<0;1,0>:ud       0x6:ud               
              shr     (1|M0)                 r13.3<1>:uw     r2.2<0;1,0>:ud       0x9:ud               
              shr     (1|M0)                 r13.4<1>:uw     r2.2<0;1,0>:ud       0xC:ud               
              shr     (1|M0)                 r13.5<1>:uw     r2.2<0;1,0>:ud       0xF:ud               
              shr     (1|M0)                 r13.6<1>:uw     r2.2<0;1,0>:ud       0x12:ud              
              shr     (1|M0)                 r13.7<1>:uw     r2.2<0;1,0>:ud       0x15:ud              
              and     (8|M0)                 r13.0<1>:uw     r13.0<8;8,1>:uw      0x7:uw               
              mov     (8|M0)                 r17.0<1>:f      r8.5<0;1,0>:f        
              mov     (8|M0)                 r15.0<1>:f      r8.6<0;1,0>:f        
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x11111111:uv        
(f0.1)        mov     (8|M0)                 r17.0<1>:f      r8.6<0;1,0>:f        
(f0.1)        mov     (8|M0)                 r14.0<1>:f      r2.0<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r15.0<1>:f      -r8.5<0;1,0>:f       r14.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r15.0<1>:f      r15.0<8;8,1>:f       -16.0:f              
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x22222222:uv        
(f0.1)        mov     (8|M0)                 r16.0<1>:f      r2.0<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r17.0<1>:f      -r8.5<0;1,0>:f       r16.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r17.0<1>:f      r17.0<8;8,1>:f       -16.0:f              
(f0.1)        mov     (8|M0)                 r14.0<1>:f      r2.1<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r15.0<1>:f      -r8.6<0;1,0>:f       r14.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r15.0<1>:f      r15.0<8;8,1>:f       -16.0:f              
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x33333333:uv        
(f0.1)        mov     (8|M0)                 r16.0<1>:f      r2.1<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r17.0<1>:f      -r8.6<0;1,0>:f       r16.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r17.0<1>:f      r17.0<8;8,1>:f       -16.0:f              
(f0.1)        mov     (8|M0)                 r15.0<1>:f      r8.5<0;1,0>:f        
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x44444444:uv        
(f0.1)        mov     (8|M0)                 r16.0<1>:f      r2.0<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r17.0<1>:f      -r8.5<0;1,0>:f       r16.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r17.0<1>:f      r17.0<8;8,1>:f       -16.0:f              
(f0.1)        mov     (8|M0)                 r15.0<1>:f      r8.6<0;1,0>:f        
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x77777777:uv        
(f0.1)        mov     (8|M0)                 r17.0<1>:f      r8.6<0;1,0>:f        
(f0.1)        mov     (8|M0)                 r15.0<1>:f      r8.5<0;1,0>:f        
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x55555555:uv        
(f0.1)        mov     (8|M0)                 r17.0<1>:f      r8.5<0;1,0>:f        
(f0.1)        mov     (8|M0)                 r14.0<1>:f      r2.1<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r15.0<1>:f      -r8.6<0;1,0>:f       r14.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r15.0<1>:f      r15.0<8;8,1>:f       -16.0:f              
              cmp     (8|M0)     (eq)f0.1    null.0<1>:w     r13.0<8;8,1>:uw      0x66666666:uv        
(f0.1)        mov     (8|M0)                 r16.0<1>:f      r2.1<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r17.0<1>:f      -r8.6<0;1,0>:f       r16.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r17.0<1>:f      r17.0<8;8,1>:f       -16.0:f              
(f0.1)        mov     (8|M0)                 r14.0<1>:f      r2.0<0;1,0>:uw       
(f0.1)        add     (8|M0)                 r15.0<1>:f      -r8.5<0;1,0>:f       r14.0<8;8,1>:f       
(f0.1)        add     (8|M0)                 r15.0<1>:f      r15.0<8;8,1>:f       -16.0:f              
(~f0.0)       mov     (8|M0)                 acc0.0<1>:f     r6.0<8;8,1>:f        
(~f0.0)       mac     (8|M0)                 r6.0<1>:f       r3.0<8;8,1>:f        r17.0<8;8,1>:f       
              mov     (8|M0)                 acc0.0<1>:f     r5.0<8;8,1>:f        
              mac     (8|M0)                 r5.0<1>:f       r4.0<8;8,1>:f        r15.0<8;8,1>:f       
