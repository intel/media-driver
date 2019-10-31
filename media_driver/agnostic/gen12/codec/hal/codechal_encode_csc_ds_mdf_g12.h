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
//! \file     codechal_encode_csc_ds_mdf_g12.h
//! \brief    Gen12 class using MDF RT for CSC
//!

#ifndef __CodechalEncodeCscDsMdfG12__
#define __CodechalEncodeCscDsMdfG12__
#include "codechal_encode_csc_ds_g12.h"
#include "codechal_kernel_header_g12.h"

struct SurfaceParamsCscMdf
{
    CmSurface2D                *psInputSurface = nullptr;
    CmSurface2D                *psOutput4xDsSurface = nullptr;
    CmSurface2D                *psOutput2xDsSurface = nullptr;
    CmSurface2D                *psOutputCopiedSurface = nullptr;
    CmBuffer                   *presMBVProcStatsBuffer = nullptr;
    CmBuffer                   *presHistoryBuffer = nullptr;
    SurfaceIndex               *pHistBufSurfIdx = nullptr;
    CmBuffer                   *presHistorySumBuffer = nullptr;
    SurfaceIndex               *pHistSumBufSurfIdx = nullptr;
    CmBuffer                   *presMultiThreadTaskBuffer = nullptr;
    SurfaceIndex               *pMTTaskSumBufSurfIdx = nullptr;
};

class CodechalEncodeCscDsMdfG12 : public CodechalEncodeCscDsG12
{

protected:

    CmThreadSpace          *m_threadSpace4x = nullptr;
    CmThreadSpace          *m_threadSpace16x = nullptr;
    CmThreadSpace          *m_threadSpace32x = nullptr;
    CmKernel               *m_cmKrnCSCDS4x = nullptr;
    CmKernel               *m_cmKrnCSCDS16x = nullptr;
    CmKernel               *m_cmKrnCSCDS32x = nullptr;
    CmProgram              *m_cmProgramCSCDS = nullptr;

    SurfaceParamsCscMdf m_cmSurfParamsCscDs4x;
    SurfaceParamsCscMdf m_cmSurfParamsCscDs16x;
    SurfaceParamsCscMdf m_cmSurfParamsCscDs32x;

// redefine the Curbe without the binding table indices
    struct CscKernelCurbeData
    {
        
        CscKernelCurbeData()
        {
            MOS_ZeroMemory(this, sizeof(*this));
        }
        union
        {
            struct
            {
                // DWORD 0
                uint32_t    DW0_Reserved_0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW0_Reserved_1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t    DW0_OutputBitDepthForChroma : MOS_BITFIELD_RANGE(16, 23);
                uint32_t    DW0_OutputBitDepthForLuma : MOS_BITFIELD_RANGE(24, 30);
                uint32_t    DW0_RoundingEnable : MOS_BITFIELD_BIT(31);
            };
            uint32_t DW0;
        };

        union
        {
            struct
            {
                // DWORD 1
                uint32_t    DW1_PictureFormat : MOS_BITFIELD_RANGE(0, 7);
                uint32_t    DW1_ConvertFlag : MOS_BITFIELD_BIT(8);
                uint32_t    DW1_DownscaleStage : MOS_BITFIELD_RANGE(9, 11);
                uint32_t    DW1_MbStatisticsDumpFlag : MOS_BITFIELD_BIT(12);
                uint32_t    DW1_YUY2ConversionFlag : MOS_BITFIELD_BIT(13);
                uint32_t    DW1_HevcEncHistorySum : MOS_BITFIELD_BIT(14);
                uint32_t    DW1_LCUSize : MOS_BITFIELD_BIT(15);
                uint32_t    DW1_ChromaSitingLocation : MOS_BITFIELD_RANGE(16, 23);
                uint32_t    DW1_Reserved_0 : MOS_BITFIELD_RANGE(24, 31);
            };
            uint32_t DW1;
        };

        union
        {
            struct
            {
                // DWORD 2
                uint32_t    DW2_OriginalPicWidthInSamples : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW2_OriginalPicHeightInSamples : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW2;
        };

        // DWORD 3
        uint32_t    DW3_MBFlatnessThreshold = 0;

        // DWORD 4
        union
        {
            struct
            {
                uint32_t    DW4_CSC_Coefficient_C0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW4_CSC_Coefficient_C1 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW4;
        };

        // DWORD 5
        union
        {
            struct
            {
                uint32_t    DW5_CSC_Coefficient_C2 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW5_CSC_Coefficient_C3 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW5;
        };

        // DWORD 6
        union
        {
            struct
            {
                uint32_t    DW6_CSC_Coefficient_C4 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW6_CSC_Coefficient_C5 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW6;
        };

        // DWORD 7
        union
        {
            struct
            {
                uint32_t    DW7_CSC_Coefficient_C6 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW7_CSC_Coefficient_C7 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW7;
        };

        // DWORD 8
        union
        {
            struct
            {
                uint32_t    DW8_CSC_Coefficient_C8 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW8_CSC_Coefficient_C9 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW8;
        };

        // DWORD 9
        union
        {
            struct
            {
                uint32_t    DW9_CSC_Coefficient_C10 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t    DW9_CSC_Coefficient_C11 : MOS_BITFIELD_RANGE(16, 31);
            };
            uint32_t DW9;
        };
    };
    C_ASSERT(MOS_BYTES_TO_DWORDS(sizeof(CscKernelCurbeData)) == 10);

public:

    //!
    //! \brief    Constructor
    //!            
    CodechalEncodeCscDsMdfG12(CodechalEncoderState* encoder);
    
    //!
    //! \brief    Destructor
    //!
    virtual ~CodechalEncodeCscDsMdfG12();


protected:

     MOS_STATUS CscKernel(
        KernelParams* pParams) override;
    MOS_STATUS InitKernelStateCsc(KernelParams* pParams);
    MOS_STATUS SetupKernelArgsCSC(CmKernel *cmKrnCSCDS, SurfaceParamsCscMdf* surfaceparams);
    MOS_STATUS SetCurbeCscforMDF(CMRT_UMD::vector<uint32_t, 10> & curbeData);
    MOS_STATUS SetupSurfacesCSC(SurfaceParamsCscMdf& SurfaceParamsCsc);
    MOS_STATUS DestroySurfaces(SurfaceParamsCscMdf& surfaceparams);
    //!
    //! \brief    Release the MDF RT resources for Ds kernel
    //!
    MOS_STATUS ReleaseResources();

};

#endif  // __CodechalEncodeCscDsG12__
