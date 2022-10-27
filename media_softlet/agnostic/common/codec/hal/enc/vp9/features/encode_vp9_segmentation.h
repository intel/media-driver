/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     encode_vp9_segmentation.h
//! \brief    Defines the common interface for vp9 encode segmentation features
//!

#ifndef __ENCODE_VP9_SEGMENTATION_H__
#define __ENCODE_VP9_SEGMENTATION_H__

#include "encode_vp9_basic_feature.h"

namespace encode
{

//!
//! \enum     VP9_MBBRC_MODE
//! \brief    VP9 mbbrc mode
//!
enum VP9_MBBRC_MODE
{
    //Target usage determines whether MBBRC is enabled or not.
    //Currently for all the target usages it is enabled.
    //once the performance is measured for performance TU mode, decision will be taken
    //whether to enable or disable MBBRC.
    MBBRC_ENABLED_TU_DEPENDENCY = 0,
    MBBRC_ENABLED               = 1,
    MBBRC_DISABLED              = 2
};


//!
//! \struct   Vp9VdencStreamInState
//! \brief    VP9 stream in buffer
//!
struct Vp9VdencStreamInState
{
        union
        {
            //!< DWORD 0
            struct
            {
                uint32_t                 Roi32X32016X1603                                 : MOS_BITFIELD_RANGE( 0,  7)    ; //!< ROI 32x32_0 16x16_03
                uint32_t                 Maxtusize                                        : MOS_BITFIELD_RANGE( 8,  9)    ; //!< MaxTUSize
                uint32_t                 Maxcusize                                        : MOS_BITFIELD_RANGE(10, 11)    ; //!< MaxCUSize
                uint32_t                 Numimepredictors                                 : MOS_BITFIELD_RANGE(12, 15)    ; //!< NUMIMEPREDICTORS
                uint32_t                 PuType32X32016X1603                              : MOS_BITFIELD_RANGE(24, 31)    ; //!< PU Type 32x32_0 16x16_03
            };
            uint32_t                     Value;
        } DW0;
        union
        {
            //!< DWORD 1
            struct
            {
                uint32_t                 ForceMvX32X32016X160                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_0
                uint32_t                 ForceMvY32X32016X160                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< force_mv.y 32x32_0 16x16_0
            };
            uint32_t                     Value;
        } DW1;
        union
        {
            //!< DWORD 2
            struct
            {
                uint32_t                 ForceMvX32X32016X161                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_1
                uint32_t                 ForceMvY32X32016X161                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< force_mv.y 32x32_0 16x16_1
            };
            uint32_t                     Value;
        } DW2;
        union
        {
            //!< DWORD 3
            struct
            {
                uint32_t                 ForceMvX32X32016X162                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_2
                uint32_t                 ForceMvY32X32016X162                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< force_mv.y 32x32_0 16x16_2
            };
            uint32_t                     Value;
        } DW3;
        union
        {
            //!< DWORD 4
            struct
            {
                uint32_t                 ForceMvX32X32016X163                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< force_mv.x 32x32_0 16x16_3
                uint32_t                 ForceMvY32X32016X163                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< force_mv.y 32x32_0 16x16_3
            };
            uint32_t                     Value;
        } DW4;
        union
        {
            //!< DWORD 5
            struct
            {
                uint32_t                 Reserved160                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW5;
        union
        {
            //!< DWORD 6
            struct
            {
                uint32_t                 ForceMvRefidx32X32016X160                        : MOS_BITFIELD_RANGE( 0,  3)    ; //!< force_mv refidx 32x32_0 16x16_0
                uint32_t                 ForceMvRefidx32X32016X1613                       : MOS_BITFIELD_RANGE( 4, 15)    ; //!< force_mv refidx 32x32_0 16x16_1-3
                uint32_t                 Nummergecandidatecu8X8                           : MOS_BITFIELD_RANGE(16, 19)    ; //!< NumMergeCandidateCU8x8
                uint32_t                 Nummergecandidatecu16X16                         : MOS_BITFIELD_RANGE(20, 23)    ; //!< NumMergeCandidateCU16x16
                uint32_t                 Nummergecandidatecu32X32                         : MOS_BITFIELD_RANGE(24, 27)    ; //!< NumMergeCandidateCU32x32
                uint32_t                 Nummergecandidatecu64X64                         : MOS_BITFIELD_RANGE(28, 31)    ; //!< NumMergeCandidateCU64x64
            };
            uint32_t                     Value;
        } DW6;
        union
        {
            //!< DWORD 7
            struct
            {
                uint32_t                 Segid32X32016X1603Vp9Only                        : MOS_BITFIELD_RANGE( 0, 15)    ; //!< SegID 32x32_0 16x16_03 (VP9 only)
                uint32_t                 QpEn32X32016X1603                                : MOS_BITFIELD_RANGE(16, 19)    ; //!< QP_En 32x32_0 16x16_03
                uint32_t                 SegidEnable                                      : MOS_BITFIELD_RANGE(20, 20)    ; //!< SegID Enable
                uint32_t                 Reserved245                                      : MOS_BITFIELD_RANGE(21, 22)    ; //!< Reserved
                uint32_t                 ForceRefidEnable32X320                           : MOS_BITFIELD_RANGE(23, 23)    ; //!< Force Refid Enable (32x32_0)
                uint32_t                 ImePredictorRefidSelect0332X320                  : MOS_BITFIELD_RANGE(24, 31)    ; //!< IME predictor/refid Select0-3  32x32_0
            };
            uint32_t                     Value;
        } DW7;
        union
        {
            //!< DWORD 8
            struct
            {
                uint32_t                 ImePredictor0X32X320                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< ime_predictor0.x 32x32_0
                uint32_t                 ImePredictor0Y32X320                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< ime_predictor0.y 32x32_0
            };
            uint32_t                     Value;
        } DW8;
        union
        {
            //!< DWORD 9
            struct
            {
                uint32_t                 ImePredictor0X32X321                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< ime_predictor0.x 32x32_1
                uint32_t                 ImePredictor0Y32X321                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< ime_predictor0.y 32x32_1
            };
            uint32_t                     Value;
        } DW9;
        union
        {
            //!< DWORD 10
            struct
            {
                uint32_t                 ImePredictor0X32X322                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< ime_predictor0.x 32x32_2
                uint32_t                 ImePredictor0Y32X322                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< ime_predictor0.y 32x32_2
            };
            uint32_t                     Value;
        } DW10;
        union
        {
            //!< DWORD 11
            struct
            {
                uint32_t                 ImePredictor0X32X323                             : MOS_BITFIELD_RANGE( 0, 15)    ; //!< ime_predictor0.x 32x32_3
                uint32_t                 ImePredictor0Y32X323                             : MOS_BITFIELD_RANGE(16, 31)    ; //!< ime_predictor0.y 32x32_3
            };
            uint32_t                     Value;
        } DW11;
        union
        {
            //!< DWORD 12
            struct
            {
                uint32_t                 ImePredictor0Refidx32X320                        : MOS_BITFIELD_RANGE( 0,  3)    ; //!< ime_predictor0 refidx 32x32_0
                uint32_t                 ImePredictor13Refidx32X3213                      : MOS_BITFIELD_RANGE( 4, 15)    ; //!< ime_predictor1-3 refidx 32x32_1-3
                uint32_t                 Reserved400                                      : MOS_BITFIELD_RANGE(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW12;
        union
        {
            //!< DWORD 13
            struct
            {
                uint32_t                 Panicmodelcuthreshold                            : MOS_BITFIELD_RANGE( 0, 15)    ; //!< PanicModeLCUThreshold
                uint32_t                 Reserved432                                      : MOS_BITFIELD_RANGE(16, 31)    ; //!< Reserved
            };
            uint32_t                     Value;
        } DW13;
        union
        {
            //!< DWORD 14
            struct
            {
                uint32_t                 ForceQpValue16X160                               : MOS_BITFIELD_RANGE( 0,  7)    ; //!< Force QP Value 16x16_0
                uint32_t                 ForceQpValue16X161                               : MOS_BITFIELD_RANGE( 8, 15)    ; //!< Force QP Value 16x16_1
                uint32_t                 ForceQpValue16X162                               : MOS_BITFIELD_RANGE(16, 23)    ; //!< Force QP Value 16x16_2
                uint32_t                 ForceQpValue16X163                               : MOS_BITFIELD_RANGE(24, 31)    ; //!< Force QP Value 16x16_3
            };
            uint32_t                     Value;
        } DW14;
        union
        {
            //!< DWORD 15
            struct
            {
                uint32_t                 Reserved480                                                                      ; //!< Reserved
            };
            uint32_t                     Value;
        } DW15;

        //! \name Local enumerations

        //! \brief NUMIMEPREDICTORS
        //! \details
        //!     <p>This parameter specifes the number of IME predictors to be processed
        //!     in stage3 IME.</p>
        //!     <p></p>
        enum NUMIMEPREDICTORS
        {
            NUMIMEPREDICTORS_UNNAMED0                                        = 0, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED4                                        = 4, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED8                                        = 8, //!< No additional details
            NUMIMEPREDICTORS_UNNAMED12                                       = 12, //!< No additional details
        };

        static const size_t dwSize = 16;
        static const size_t byteSize = 64;
    };

class Vp9Segmentation : public MediaFeature,
    public mhw::vdbox::hcp::Itf::ParSetting,
    public mhw::vdbox::vdenc::Itf::ParSetting
{
public:
    //!
    //! \brief  Vp9Segmentation feature constructor
    //!
    //! \param  [in] featureManager
    //!         Pointer to MediaFeatureManager
    //! \param  [in] allocator
    //!         Pointer to EncodeAllocator
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] constSettings
    //!         Pointer to const settings
    //!
    Vp9Segmentation(
        MediaFeatureManager *featureManager,
        EncodeAllocator *    allocator,
        CodechalHwInterfaceNext *hwInterface,
        void *               constSettings);

    //!
    //! \brief  Vp9Segmentation feature destructor
    //!
    virtual ~Vp9Segmentation();

    //!
    //! \brief  Init segmentation related parameter
    //! \param  [in] settings
    //!         Pointer to settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Init(void *settings) override;

    //!
    //! \brief  Update segmentation related parameter
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Update(void *params) override;

    //!
    //! \brief  Set Dmem buffer for brc update
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForUpdate(void *params);

    //!
    //! \brief  Set Dmem buffer for huc prob
    //! \param  [in] params
    //!         Pointer to parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetDmemForHucProb(void *params);

    //!
    //! \brief    Check if segmentation feature is enabled
    //! \param    [out] enabled
    //!           Enabled flag
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsEnabled(bool &enabled)
    {
        enabled = m_enabled;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Check if APP's segmentation map provided
    //! \param    [out] provided
    //!           Flag to indicate APP's segmentation map provided or not
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsSegmentMapProvided(bool &provided)
    {
        provided = m_segmentMapProvided;
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief  Set segment ID
    //! \param  [in] params
    //!         Segment ID
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSegmentId(uint8_t segmentId);

    static constexpr uint32_t m_segmentStateBlockSize = 32;

    //!
    //! \brief MHW parameters declaration
    //!
    MHW_SETPAR_DECL_HDR(HCP_VP9_SEGMENT_STATE);


    //!
    //! \brief MHW parameters declaration VDENC_CMD2
    //!
    MHW_SETPAR_DECL_HDR(VDENC_CMD2);

protected:
    //!
    //! \brief  Free resources
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS FreeResources();

    //!
    //! \brief  Set sequence structures
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSequenceStructs();

    //!
    //! \brief  Allocate mb brc segment map surface
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateMbBrcSegMapSurface();

    //!
    //! \brief  Setup segmentation stream in
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetupSegmentationStreamIn();

    //!
    //! \brief  initialize zig-zag to raster LUT per tile
    //! \param  [in] tileWidth
    //!         Tile width
    //! \param  [in] tileHeight
    //!         Tile height
    //! \param  [in] currTileStartXInFrame
    //!         Current tile start X in frame
    //! \param  [in] currTileStartYInFrame
    //!         Current tile start Y in frame
    //! \param  [in/out] blocksRasterized
    //!         Count of rasterized blocks for this frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitZigZagToRasterLUTPerTile(
        uint32_t  tileWidth,
        uint32_t  tileHeight,
        uint32_t  currTileStartXInFrame,
        uint32_t  currTileStartYInFrame,
        uint32_t &blocksRasterized);

    //!
    //! \brief  Returns the offset of 32x32 block in the frame based on
    //!         current x, y 32 block location in current tile
    //! \param  [in] frameWidth
    //!         Frame width
    //! \param  [in] curr32XInTile
    //!         Tile height
    //! \param  [in] curr32YInTile
    //!         Tile width
    //! \param  [in] currTileStartX64aligned
    //!         Current tile start X 64 aligned
    //! \param  [in] currTileStartY64aligned
    //!         Current tile start Y 64 aligned
    //! \return uint32_t
    //!         Segment block index in frame
    //!
    uint32_t GetSegmentBlockIndexInFrame(
        uint32_t frameWidth,
        uint32_t curr32XInTile,
        uint32_t curr32YInTile,
        uint32_t currTileStartX64aligned,
        uint32_t currTileStartY64aligned);

    //!
    //! \brief  Calculate buffer offset
    //! \param  [in] idx
    //!         Index
    //! \param  [in] width
    //!         Width
    //! \param  [in] blockSize
    //!         Block size
    //! \param  [in] bufferPitch
    //!         Buffer pitch
    //!
    //! \return uint32_t
    //!         Return 0 if call success, else -1 if fail
    //!
    uint32_t CalculateBufferOffset(
        uint32_t idx,
        uint32_t width,
        uint32_t blockSize,
        uint32_t bufferPitch);

    PCODEC_VP9_ENCODE_SEGMENT_PARAMS m_vp9SegmentParams = nullptr;  //!< Pointer to segment parameters

    EncodeAllocator *    m_allocator    = nullptr;
    Vp9BasicFeature *    m_basicFeature = nullptr;
    CodechalHwInterfaceNext *m_hwInterface  = nullptr;

    bool m_mbBrcEnabled   = false;  //!< MBBRC enable flag
    bool m_mbStatsEnabled = false;  //!< MB status enabled flag
    // Segmentation resources
    bool        m_segmentMapProvided  = false;  //!< Flag to indicate APP's segmentation map provided or not
    bool        m_segmentMapAllocated = false;
    MOS_SURFACE m_mbSegmentMapSurface = {0};

    uint32_t *m_mapBuffer          = nullptr; //!< Use for re-map recreate map buffer
    uint32_t  m_segStreamInHeight  = 0;
    uint32_t  m_segStreamInWidth   = 0;

    uint8_t m_segmentId = 0;

MEDIA_CLASS_DEFINE_END(encode__Vp9Segmentation)
};

}  // namespace encode

#endif  // __ENCODE_VP9_SEGMENTATION_H__