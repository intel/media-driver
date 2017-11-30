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
 
    and      (1)     r1.4<1>:uw      r0.2<0;1,0>:uw     0x7ff07ff:uw                                  { Align1, N1, NoMask, NoCompact }
    and      (1)     r1.5<1>:uw      r0.3<0;1,0>:uw     0x7ff07ff:uw                                  { Align1, N1, NoMask, NoCompact }
    mov      (8)     r20.0<1>:ud     r0.0<8;8,1>:ud                                                   { Align1, N1, NoMask, Compacted }
    mul      (1)     r1.3<1>:ud      r1.4<0;1,0>:uw     0x80008:uw                                    { Align1, N1, NoCompact }
    mul      (1)     r1.4<1>:ud      r1.5<0;1,0>:uw     0x100010:uw                                   { Align1, N1, NoCompact }
    add      (1)     a0.0<1>:ud      r1.0<0;1,0>:ud     0x02490000:ud                                 { Align1, N1, NoMask, NoCompact }
    mov      (1)     r20.2<1>:ud     0x000f0007:ud                                                    { Align1, N1, NoMask, NoCompact }
    mov      (1)     r20.0<1>:d      r1.3<0;1,0>:ud                                                   { Align1, N1, NoCompact }
    mov      (1)     r20.1<1>:d      r1.4<0;1,0>:ud                                                   { Align1, N1, NoCompact }
    send     (8)     r10.0<1>:ud     r20.0<0;1,0>:f     0x0000000c       a0.0<0;1,0>:ud               { Align1, N1, NoMask, NoCompact }
    mov      (16)    r8.0<1>:ud      0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (16)    r6.0<1>:ud      0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (16)    r4.0<1>:ud      0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (16)    r2.0<1>:ud      0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (8)     r21.0<1>:ud     r0.0<8;8,1>:ud                                                   { Align1, N1, NoMask, Compacted }
    mov      (1)     r21.2<1>:ud     0x000f000f:ud                                                    { Align1, N1, NoMask, NoCompact }
    add      (1)     a0.0<1>:ud      r1.1<0;1,0>:ud     0x020a8000:ud                                 { Align1, N1, NoMask, NoCompact }
    shl      (1)     r21.0<1>:d      r1.3<0;1,0>:ud     0x10001:uw                                    { Align1, N1, NoCompact }
    mov      (1)     r21.1<1>:d      r1.4<0;1,0>:ud                                                   { Align1, N1, NoCompact }
    mov      (32)    r8.1<2>:ub      r13.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    mov      (32)    r6.1<2>:ub      r12.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    mov      (32)    r4.1<2>:ub      r11.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    mov      (32)    r2.1<2>:ub      r10.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    sends    (8)     null:ud         r21:ud             r2:ud            0x0000020c:ud     a0.0:ud    { Align1, N1, NoMask, NoCompact }
    mov      (1)     r20.0<1>:d      r1.3<0;1,0>:ud                                                   { Align1, N1, NoCompact }
    asr      (1)     r20.1<1>:d      r1.4<0;1,0>:ud     0x10001:uw                                    { Align1, N1, NoCompact }
    mov      (1)     r20.2<1>:ud     0x00070007:ud                                                    { Align1, N1, NoMask, NoCompact }
    mov      (8)     r22.0<1>:ud     r0.0<8;8,1>:ud                                                   { Align1, N1, NoMask, Compacted }
    mov      (1)     r22.2<1>:ud     0x0007000f:ud                                                    { Align1, N1, NoMask, NoCompact }
    add      (1)     a0.0<1>:ud      r1.0<0;1,0>:ud     0x02290001:ud                                 { Align1, N1, NoMask, NoCompact }
    send     (8)     r18.0<1>:ud     r20.0<0;1,0>:f     0x0000000c       a0.0<0;1,0>:ud               { Align1, N1, NoMask, NoCompact }
    mov      (16)    r16.0<1>:ud     0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (16)    r14.0<1>:ud     0x0000:uw                                                        { Align1, N1, NoCompact }
    mov      (1)     r22.1<1>:d      r20.1<0;1,0>:d                                                   { Align1, N1, NoCompact }
    mov      (1)     r22.0<1>:d      r21.0<0;1,0>:d                                                   { Align1, N1, NoCompact }
    add      (1)     a0.0<1>:ud      r1.1<0;1,0>:ud     0x020a8001:ud                                 { Align1, N1, NoMask, NoCompact }
    mov      (32)    r16.1<2>:ub     r19.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    mov      (32)    r14.1<2>:ub     r18.0<8;8,1>:ub                                                  { Align1, N1, NoCompact }
    sends    (8)     null:ud         r22:ud             r14:ud           0x0000010c:ud     a0.0:ud    { Align1, N1, NoMask, NoCompact }
    mov      (8)     r127.0<1>:ud    r0.0<8;8,1>:ud                                                   { Align1, N1, NoMask, Compacted }
    send     (1)     null:ud         r127.0<0;1,0>:f    0x00000027       0x02000010:ud                { Align1, N1, EOT, NoCompact }
