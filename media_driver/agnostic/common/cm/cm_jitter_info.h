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

#ifndef _CM_JITTER_INFO_
#define _CM_JITTER_INFO_
// Make JIT_INFO available in offline compile too,
// so we don't have to have annoying #ifdef DLL_MODE
typedef struct _CM_PROFILE_INFO {
    int kind;
    int index;
    int value;
} CM_PROFILE_INFO;

typedef struct _CM_BB_INFO {
    int id;
    unsigned staticCycle;
    unsigned sendStallCycle;
} CM_BB_INFO;

typedef struct _CM_JIT_INFO {
    // Common part
    bool isSpill;
    int numGRFUsed;
    int numAsmCount;

    // Kernel specific area.
    unsigned int spillMemUsed;

    // Debug info is callee allocated
    // and populated only if
    // switch is passed to JIT to emit
    // debug info.
    void* genDebugInfo;
    unsigned int genDebugInfoSize;

    // Number of flag spill and fill.
    unsigned numFlagSpillStore;
    unsigned numFlagSpillLoad;

    // whether kernel uses a barrier
    bool usesBarrier;

    unsigned bbNum;
    CM_BB_INFO *bbInfo;

    // number of spill/fill, weighted by loop
    unsigned int numGRFSpillFill;
} FINALIZER_INFO;

#define MAX_ERROR_MSG_LEN               511
#endif
