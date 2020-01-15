/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     codechal_encode_hevc_brc_g12.h
//! \brief    HEVC dual-pipe base class kernel interface for GEN12 platform.
//!


#ifndef __CODECHAL_ENCODE_HEVC_BRC_G12_H__
#define __CODECHAL_ENCODE_HEVC_BRC_G12_H__

#include "codechal_encode_hevc_g12.h"

class CodecHalHevcMbencG12;
namespace CMRT_UMD
{
    class CmDevice;
    class CmTask;
    class CmQueue;
    class CmThreadSpace;
    class CmKernel;
    class CmProgram;
    class SurfaceIndex;
    class CmSurface2D;
    class CmBuffer;
}

#define MAX_VME_BWD_REF           4
#define MAX_VME_FWD_REF           4

//!  HEVC dual-pipe BRC class for GEN12 HEVC BRC kernel with MDF support
class CodecHalHevcBrcG12
{

protected:
    CmThreadSpace *    m_threadSpaceBrcInit = nullptr;
    CmThreadSpace *    m_threadSpaceBrcUpdate = nullptr;
    CmThreadSpace *    m_threadSpaceBrcLCUQP = nullptr;
    CmKernel           *m_cmKrnBrc = nullptr;
    CmProgram          *m_cmProgramBrc = nullptr;
    CmKernel           *m_cmKrnBrcUpdate = nullptr;
    CmProgram          *m_cmProgramBrcUpdate = nullptr;
    CmKernel           *m_cmKrnBrcLCUQP = nullptr;
    CmProgram          *m_cmProgramBrcLCUQP = nullptr;

    //Internal surfaces
    CmBuffer            *m_histBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmBuffer            *m_PAKStatsBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmBuffer            *m_PICStateInBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmBuffer            *m_PICStateOutBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmBuffer            *m_CombinedEncBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmBuffer            *m_PixelMBStatsBufferBrc = nullptr;                         //!< Resource owned by BRC
    CmSurface2D         *m_ConstDataBufferBRC = nullptr;                      //!< distortion buffer input to BRC.
    CmSurface2D         *m_BrcMbQp = nullptr;                      //!< distortion buffer input to BRC.
    CmSurface2D         *m_BrcROISurf = nullptr;                      //!< ROI table buffer input to BRC.
public:
    //!
    //! \brief    Constructor
    //!

    uint32_t            m_brcNumPakPasses = 0;
    CodecHalHevcMbencG12* encoderBrc;
    CodecHalHevcBrcG12(CodecHalHevcMbencG12* encoder) { encoderBrc = encoder; };
    //!
    //! \brief    Destructor
    //!
    virtual ~CodecHalHevcBrcG12() {};

public:
    //!
    //! \brief    Allocate resources used by MBEnc
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AllocateBrcResources();

    //!
    //! \brief    Free resources
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeBrcResources();


    MOS_STATUS SetupThreadSpace(CmKernel *cmKernel, CmThreadSpace *threadSpace);
    MOS_STATUS SetupBrcLcuqpThreadSpace(CmKernel *cmKernel, CmThreadSpace *threadSpace);

    virtual MOS_STATUS InitBrcKernelState();
    virtual MOS_STATUS EncodeBrcInitResetKernel();
    virtual MOS_STATUS EncodeBrcFrameUpdateKernel();
    virtual MOS_STATUS EncodeBrcLcuUpdateKernel();


    MOS_STATUS InitCurbeDataBrcInit();
    MOS_STATUS SetupSurfacesBrcInit();
    MOS_STATUS BrcInitResetCurbe(CODECHAL_HEVC_BRC_KRNIDX  brcKrnIdx);
    MOS_STATUS BrcUpdateCurbe(CODECHAL_HEVC_BRC_KRNIDX  brcKrnIdx);
    MOS_STATUS SetupSurfacesBrcUpdate();
    MOS_STATUS SetupSurfacesBrcLcuQp();
    //!
    //! \brief    Setup Kernel Arguments for BRC init frame
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!    
    MOS_STATUS SetupKernelArgsBrcInit();
    MOS_STATUS SetupKernelArgsBrcUpdate();
    MOS_STATUS SetupKernelArgsBrcLcuQp();

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS DumpBrcInputBuffers();
#endif
};

#endif  // __CODECHAL_ENCODE_HEVC_BRC_G12_H__
