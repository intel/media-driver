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
//!
//! \file     CMRTKernel_PB_Kernel_def.h
//! \brief    HEVC FEI PB parameter definition for GEN9 SKL.
//!

#ifndef _CMRTKERNEL_PB_KERNEL_DEF_
#define _CMRTKERNEL_PB_KERNEL_DEF_

#define NUM_MBENC_PB_32X32_IC_SURFACES       10
#define NUM_MBENC_PB_PAK_SURFACES            11
#define NUM_MBENC_PB_MB_SURFACES             27
#define NUM_MBENC_P_MB_SURFACES              26

#define CURBEDATA_SIZE_PB_32X32_IC           32
#define CURBEDATA_SIZE_PB_PAK                (32 * 2)
#define CURBEDATA_SIZE_PB_MB                 (32 * 7)

#define HEVC_PB_ISA_FILE_NAME_G9             "/opt/intel/mediasdk/lib64/HevcEnc_PB_genx.isa"
#define HEVCENCKERNELNAME_PB_32x32           "Hevc_LCUEnc_PB_32x32_IntraCheck"
#define HEVCENCKERNELNAME_PB_PAK             "HEVC_LCUEnc_PB_PAK"
#define HEVCENCKERNELNAME_PB_MB              "HEVC_LCUEnc_PB_MB"
#define HEVCENCKERNELNAME_P_MB               "HEVC_LCUEnc_P_MB"

typedef struct
{
    void     *m_cmSurfPer32x32ICOut;
    void     *m_cmSurfCurrY;
    void     *m_cmSurfCurrY2;
    void     *m_cmSurfSliceMap;
    void     *m_cmLCUQPSurf;
    void     *m_cmSurfSIF;
    void     *m_cmSurfPOCDbuf;
    void     *m_cmSurfCombinedQP;
    void     *m_cmBRCConstSurf;
    void     *m_cmSurfMVIndex;
    void     *m_cmSurfMVPred;
    void     *m_cmSurfColRefData;
    void     *m_cmSurfIntraDist;
    void     *m_cmSurfMinDist;
    void     *m_cmSurfVMEIN;
    void     *m_cmWaveFrontMap;
    void     *m_cmSurfHaarDist;
    void     *m_cmSurfFrameStats;
    void     *m_cmSurfStats;
    void     *m_cmSurfMVPredictor;
    void     *m_cmSurfRef0[8];
    void     *m_cmSurfRef1[8];
    void     *m_cmSurfPerCTBInput;
    uint8_t   m_ucRefNum0;
    uint8_t   m_ucRefNum1;
    uint32_t  m_width;
    uint32_t  m_height;
    uint32_t  m_bufSize;
    uint32_t  m_bufOffset;
} PBFrameKernelParams;

typedef struct
{
    CM_WALKING_PARAMETERS   m_walkParams;
    CM_DEPENDENCY       m_scoreboardParams;
    uint8_t  m_totalPhase;
    uint32_t m_currentLocalOuterLoopExecCount;
    uint32_t m_totalLocalOuterLoopExecCount;
    uint32_t m_deltaLocalOuterLoopExecCount;
} PBMbEncWalkParams;

#endif
