/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_basic_feature_g12.h
//! \brief    Defines the common interface for decode av1 basic feature g12
//!
#ifndef __DECODE_AV1_BASIC_FEATURE_G12_H__
#define __DECODE_AV1_BASIC_FEATURE_G12_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_av1.h"
#include "decode_av1_reference_frames_g12.h"
#include "decode_av1_temporal_buffers_g12.h"
#include "decode_av1_tile_coding_g12.h"
#include "mhw_vdbox_avp_interface.h"
#include "decode_internal_target.h"
#include "codechal_hw.h"

namespace decode
{
    class Av1BasicFeatureG12 : public DecodeBasicFeature
    {
    public:
        //!
        //! \brief  Av1BasicFeatureG12 constructor
        //!
        Av1BasicFeatureG12(DecodeAllocator *allocator, void *hwInterface, PMOS_INTERFACE osInterface) :
            DecodeBasicFeature(allocator, hwInterface ? ((CodechalHwInterface*)hwInterface)->m_hwInterfaceNext : nullptr, osInterface)
        {
            if (osInterface != nullptr)
            {
                m_osInterface = osInterface;
            }
            m_hwInterface = (CodechalHwInterface*)hwInterface;
        };

        //!
        //! \brief  Av1BasicFeatureG12 deconstructor
        //!
        virtual ~Av1BasicFeatureG12();

        //!
        //! \brief  Initialize av1 basic feature CodechalSetting
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init(void *setting) override;

        //!
        //! \brief  Update av1 decodeParams
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Update(void *params) override;

        //!
        //! \brief  Detect conformance conflict and do error concealment
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ErrorDetectAndConceal();

        //!
        //! \brief    Initialize one of AV1 Decode frame context buffers with default values
        //! \param    [in] ctxBuffer
        //!           Pointer to frame context buffer
        //! \param    [in] index
        //!           flag to indicate the coeff CDF table index
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS InitDefaultFrameContextBuffer(
            uint16_t              *ctxBuffer,
            uint8_t               index);

        //!
        //! \brief    Initialize CDF tables for one Syntax Element
        //! \details  Initialize CDF tables for one Syntax Element according to its CDF table layout and the initialization buffer
        //! \param    [in] ctxBuffer
        //!           Pointer to frame context buffer
        //! \param    [in] SyntaxElement
        //!           CDF table layout info and the initialization buffer for this syntax element
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SyntaxElementCdfTableInit(
            uint16_t                    *ctxBuffer,
            SyntaxElementCdfTableLayout SyntaxElement);

        //!
        //! \brief    Update default cdfTable buffers
        //! \details  Update default cdfTable buffers for AV1 decoder
        //! \param    [in] cmdBuffer
        //!           Command buffer to hold HW commands
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS UpdateDefaultCdfTable();

        PMOS_INTERFACE GetOsInterface()
        {
            return m_osInterface;
        }

        // Parameters passed from application
        uint16_t                        m_frameWidthAlignedMinBlk  = 0;            //!< Picture Width aligned to minBlock
        uint16_t                        m_frameHeightAlignedMinBlk = 0;            //!< Picture Height aligned to minBlock
        uint8_t                         m_av1DepthIndicator        = 0;            //!< Indicate it is 8/10/12 bit AV1
        CodecAv1PicParams               *m_av1PicParams            = nullptr;      //!< Pointer to AV1 picture parameter
        CodecAv1SegmentsParams          *m_segmentParams           = nullptr;      //!< Pointer to AV1 segments parameter
        CodecAv1TileParams              *m_av1TileParams           = nullptr;      //!< Pointer to AV1 tiles parameter

        PMOS_BUFFER                     m_defaultCdfBuffers[4]     = {};           //!< 4 default frame contexts per base_qindex
        PMOS_BUFFER                     m_defaultCdfBufferInUse    = nullptr;      //!< default cdf table used base on current base_qindex
        uint8_t                         m_curCoeffCdfQCtx          = 0;            //!< Coeff CDF Q context ID for current frame
        static const uint32_t           m_cdfMaxNumBytes           = 15104;        //!< Max number of bytes for CDF tables buffer, which equals to 236*64 (236 Cache Lines)
        static const uint32_t           av1DefaultCdfTableNum      = 4;            //!< Number of inited cdf table
                                                                                   //for Internal buffer upating
        bool                            m_defaultFcInitialized     = false;        //!< default Frame context initialized flag. default frame context should be initialized only once, and set this flag to 1 once initialized.

        Av1ReferenceFramesG12           m_refFrames;                               //!< Reference frames
        Av1DecodeTileG12                m_tileCoding;                              //!< Tile coding
        std::vector<uint32_t>           m_refFrameIndexList;                       //!< Reference frame index list
        RefrenceAssociatedBuffer<Av1RefAssociatedBufs, Av1TempBufferOpInfG12, Av1BasicFeatureG12> m_tempBuffers; //!< Reference associated buffers

        InternalTargets                 m_internalTarget;                          //!< Internal decode out surface
        FilmGrainProcParams            *m_filmGrainProcParams       = nullptr;     //!< Film grain processing params
        bool                            m_frameCompletedFlag        = false;
        bool                            m_filmGrainEnabled          = false;       //!< Per-frame film grain enable flag
        bool                            m_usingDummyWl              = false;       //!< Indicate using dummy workload flag
        PMOS_SURFACE                    m_destSurfaceForDummyWL     = nullptr;     //!< Internal Dummy dest surface
        bool                            m_singleKernelPerfFlag      = true;        //!< Defaut to capture whole kernel execution timing for perf
        PMOS_SURFACE                    m_fgInternalSurf            = nullptr;     //!< Internal film grain surface for AVP+FilmGrain+SFC case
        MOS_SURFACE                     m_fgOutputSurf              = {};          //!< Film Grain output surface from App

    protected:
        virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
        MOS_STATUS SetPictureStructs(CodechalDecodeParams *decodeParams);
        MOS_STATUS SetTileStructs();
        MOS_STATUS SetSegmentData(CodecAv1PicParams &picParams);
        MOS_STATUS GetDecodeTargetFormat(MOS_FORMAT &format);

        //!
        //! \brief    Calculate global motion params
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CalculateGlobalMotionParams();

        MhwVdboxAvpInterface *m_avpInterface = nullptr;
        PMOS_INTERFACE        m_osInterface  = nullptr;
        CodechalHwInterface  *m_hwInterface  = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__Av1BasicFeatureG12)
    };

}  // namespace decode

#endif  // !__DECODE_AV1_BASIC_FEATURE_G12_H__
