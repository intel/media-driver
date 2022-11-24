/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     decode_hevc_basic_feature.h
//! \brief    Defines the common interface for decode hevc basic feature
//!
#ifndef __DECODE_HEVC_BASIC_FEATURE_H__
#define __DECODE_HEVC_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_hevc.h"
#include "codec_def_decode_hevc.h"
#include "decode_hevc_reference_frames.h"
#include "decode_hevc_mv_buffers.h"
#include "decode_hevc_tile_coding.h"

namespace decode
{
class HevcBasicFeature :public DecodeBasicFeature
{
public:
    HevcBasicFeature(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) :
        DecodeBasicFeature(allocator, hwInterface, osInterface)
    {
        if (osInterface != nullptr)
        {
            m_osInterface = osInterface;
        }
    };

    virtual ~HevcBasicFeature();

    virtual MOS_STATUS Init(void *setting) override;

    virtual MOS_STATUS Update(void *params) override;

    PMOS_INTERFACE GetOsInterface()
    {
        return m_osInterface;
    }

    MOS_STATUS CreateReferenceBeforeLoopFilter();

    bool IsLastSlice(uint32_t sliceIdx);
    bool IsIndependentSlice(uint32_t sliceIdx);

    // Parameters passed from application
    PCODEC_HEVC_PIC_PARAMS          m_hevcPicParams = nullptr;      //!< Pointer to picture parameter
    PCODEC_HEVC_SLICE_PARAMS        m_hevcSliceParams = nullptr;    //!< Pointer to slice parameter
    PCODECHAL_HEVC_IQ_MATRIX_PARAMS m_hevcIqMatrixParams = nullptr; //!< Pointer to IQ matrix parameter
    PCODEC_HEVC_EXT_PIC_PARAMS      m_hevcRextPicParams = nullptr;  //!< Extended pic params for Rext
    PCODEC_HEVC_EXT_SLICE_PARAMS    m_hevcRextSliceParams = nullptr;//!< Extended slice params for Rext
    PCODEC_HEVC_SCC_PIC_PARAMS      m_hevcSccPicParams = nullptr;   //!< Pic params for SCC
    PCODEC_HEVC_SUBSET_PARAMS       m_hevcSubsetParams = nullptr;   //!< Hevc subset params for tile entrypoint offset

    bool                            m_isWPPMode    = false;         //!< Indicate current picture is WPP mode
    bool                            m_isSCCIBCMode = false;         //!< Indicate current picture is SCC IBC mode
    bool                            m_isSCCPLTMode = false;         //!< Indicate current picture is SCC PLT mode
    bool                            m_isSCCACTMode = false;         //!< Indicate current picture is SCC ACT mode

    PMOS_SURFACE                    m_referenceBeforeLoopFilter = nullptr; //!< For SCC IBC
    HevcReferenceFrames             m_refFrames;                    //!< Reference frames
    std::vector<uint32_t>           m_refFrameIndexList;            //!< Reference frame index list
    RefrenceAssociatedBuffer<MOS_BUFFER, HevcMvBufferOpInf, HevcBasicFeature>
                                    m_mvBuffers;                    //!< MV buffers
    HevcTileCoding                  m_tileCoding;                   //!< Tile coding

    uint32_t                        m_minCtbSize = 0;               //!< Min Luma Coding Block Size
    uint32_t                        m_ctbSize = 0;                  //!< Max coding tree block size
    uint32_t                        m_widthInCtb  = 0;              //!< Frame width in LCU
    uint32_t                        m_heightInCtb  = 0;             //!< Frame height in LCU

    bool                            m_dummyReferenceSlot[CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC];
    bool                            m_shortFormatInUse = false;     //!< Indicate if short format

protected:
    virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;

    MOS_STATUS SetPictureStructs();
    MOS_STATUS SetSliceStructs();
    MOS_STATUS ErrorDetectAndConceal();
    MOS_STATUS ErrorDetectAndConcealForLongFormat();
    MOS_STATUS SliceDataSizeCheck(uint32_t sliceIdx);
    MOS_STATUS SliceSegmentAddressCheck(uint32_t sliceIdx, std::vector<int> &sliceSegmentAddressVector);
    MOS_STATUS NumEntryPointOffsetsCheck(uint32_t sliceIdx);
    MOS_STATUS ReferenceParamCheck(uint32_t sliceIdx);
    MOS_STATUS CollocatedRefIdxCheck(uint32_t sliceIdx);

    PMOS_INTERFACE        m_osInterface  = nullptr;

MEDIA_CLASS_DEFINE_END(decode__HevcBasicFeature)
};

}  // namespace encode

#endif  // !__DECODE_HEVC_BASIC_FEATURE_H__
