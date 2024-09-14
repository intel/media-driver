/*
* Copyright (c) 2021-2024, Intel Corporation
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
//! \file     encode_aqm_feature.h
//! \brief    Defines the common interface for encode aqm feature
//!
#ifndef __ENCODE_AQM_FEATURE_H__
#define __ENCODE_AQM_FEATURE_H__

#include <queue>

#include "media_feature.h"
#include "encode_allocator.h"
#include "codec_hw_next.h"
#include "codechal_debug.h"
#include "encode_basic_feature.h"
#include "mhw_vdbox_aqm_itf.h"
#include "encode_mem_compression.h"
#if _MEDIA_RESERVED
#include "encode_aqm_feature_ext.h"
#endif 

#define ENCODE_VDENC_MAX_TILE_NUM               4096 

namespace encode
{
class EncodeAqmFeature : public MediaFeature, public mhw::vdbox::aqm::Itf::ParSetting
{
public:
    EncodeAqmFeature(MediaFeatureManager *featureManager,
        EncodeAllocator *                 allocator,
        CodechalHwInterfaceNext *         hwInterface,
        void *                            constSettings);

    virtual ~EncodeAqmFeature();

    virtual MOS_STATUS Update(void *params) override;

    MHW_SETPAR_DECL_HDR(AQM_PIPE_BUF_ADDR_STATE);
    MHW_SETPAR_DECL_HDR(AQM_PIC_STATE);
    MHW_SETPAR_DECL_HDR(AQM_FRAME_START);

    virtual void SetAQMMode(const uint8_t aqmMode)
    {
        m_aqmMode = aqmMode;
    }

    virtual uint8_t GetAQMMode()
    {
        return m_aqmMode;
    }

    void SetCurrentPipe(const uint8_t currPipeNum)
    {
        m_currPipeNum = currPipeNum;
    }
#if USE_CODECHAL_DEBUG_TOOL
    virtual MOS_STATUS UpdateFrameDisplayOrder(const uint16_t pictureCodingType, const uint32_t framePOC, const uint32_t gopPicSize);
#endif    
    virtual MOS_STATUS ReportQualityInfoFrame(uint32_t statBufIdx, EncodeStatusReportData& statusReportData);

#if _MEDIA_RESERVED
#define AQM_FEATURE_HEADER_EXT
#include "encode_aqm_feature_ext.h"
#undef AQM_FEATURE_HEADER_EXT
#else
    struct AQM_Ouput_Format
    {
        union
        {
            uint32_t DWord0;
            struct
            {
                uint32_t SSEY;  // [31:0]
            };
        };

        //DW1
        union
        {
            uint32_t DWord1;
            struct
            {
                uint32_t SSEU;  // [31:0]
            };
        };

        //DW2
        union
        {
            uint32_t DWord2;
            struct
            {
                uint32_t SSEV;  // [31:0]
            };
        };
        uint32_t DWord[47];
    };
#endif

protected:
    //!
    //! \enum AqmSurfaceId
    //! Aqm surface ID
    //!
    enum AqmSurfaceId
    {
        reconPic    = 0,    //!< reconstructed picture
        srcInputPic = 1,    //!< input source picture
    };

    enum LCU_SIZE
    {
        LCU_SIZE_16X16 = 0,
        LCU_SIZE_32X32 = 1,
        LCU_SIZE_64X64 = 2,
    };

    enum CODECTYPE
    {
        CODECTYPE_AVC  = 0,
        CODECTYPE_HEVC = 1,
        CODECTYPE_AV1  = 2,
        CODECTYPE_VP9  = 3,
        CODECTYPE_MP2  = 4,
    };

    static const uint32_t AQMBlockSizeLog2 = 2;
    static const uint32_t CL_SIZE_BYTES    = 64;
    static const uint32_t AQM_INDEX        = 5;

    virtual MOS_STATUS AllocateResources() override;
    virtual MOS_STATUS FreeResources();

    uint32_t EncodeAqmFeatureFunction0(uint32_t frameWidth, uint32_t frameHeight, uint8_t index);

    MOS_STATUS GetFrameMSE(AQM_Ouput_Format* pDataFrame, uint32_t(&MSE)[3]);

    CodechalHwInterfaceNext *m_hwInterface        = nullptr;
    EncodeAllocator *    m_allocator          = nullptr;
    EncodeBasicFeature * m_basicFeature       = nullptr;  //!< EncodeBasicFeature
    bool                 m_AllocatedResources = false;
    MOS_CONTEXT_HANDLE   m_mosCtx             = nullptr;

    PMOS_RESOURCE EncodeAqmFeatureMember0[5]  = {};
    uint32_t      EncodeAqmFeatureMember1[5]  = {};

    uint32_t      EncodeAqmFeatureMember2     = 0;
    uint32_t      EncodeAqmFeatureMember3[5]  = {};

    uint32_t m_numTiles        = 1;  //!< Total tile numbers
    uint16_t m_tile_width[ENCODE_VDENC_MAX_TILE_NUM]    = {};
    uint16_t m_tile_height[ENCODE_VDENC_MAX_TILE_NUM]   = {};
    bool     m_tileBasedEngine = false;
    uint8_t  m_aqmMode         = 0;
    uint8_t  m_metricsDumpMode = 0;

    uint8_t m_numRowStore = 1;
    uint8_t m_currPipeNum = 0;

    MOS_SURFACE   m_rawSurface = {};  //!< Pointer to MOS_SURFACE of raw surface
    MOS_SURFACE m_reconSurface = {};  //!< Pointer to MOS_SURFACE of reconstructed surface

#if USE_CODECHAL_DEBUG_TOOL
    std::queue<uint32_t>    m_frameIdxQueue;
    uint32_t                m_gopSizePrevious   = 0;
    uint32_t                m_frameNumPrevious  = 0;
#endif

MEDIA_CLASS_DEFINE_END(encode__EncodeAqmFeature)
};

}  // namespace encode

#endif  // !__ENCODE_AQM_FEATURE_H__
