/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file     decode_vvc_basic_feature.h
//! \brief    Defines the common interface for decode vvc basic feature
//!
#ifndef __DECODE_VVC_BASIC_FEATURE_H__
#define __DECODE_VVC_BASIC_FEATURE_H__

#include "decode_basic_feature.h"
#include "codec_def_decode_vvc.h"
#include "codec_def_common_vvc.h"
#include "decode_vvc_reference_frames.h"
#include "decode_vvc_mv_buffers.h"
#include "mhw_vdbox_vvcp_itf.h"
#include "decode_internal_target.h"

namespace decode
{
    class VvcBasicFeature : public DecodeBasicFeature
    {
    public:
        //!
        //! \brief  VvcBasicFeature constructor
        //!
        VvcBasicFeature(DecodeAllocator *allocator, CodechalHwInterfaceNext *hwInterface, PMOS_INTERFACE osInterface) : DecodeBasicFeature(allocator, hwInterface, osInterface)
        {
            m_hwInterface = hwInterface;
            if (osInterface != nullptr)
            {
                m_osInterface = osInterface;
            }
            if (hwInterface != nullptr)
            {
                m_vvcpItf = std::static_pointer_cast<mhw::vdbox::vvcp::Itf>(m_hwInterface->GetVvcpInterfaceNext());
            }
            MOS_ZeroMemory(m_scalingListArray, sizeof(CodecVvcQmData) * 8);
            MOS_ZeroMemory(m_tileRow, sizeof(TileRowDesc) * 440);
            MOS_ZeroMemory(m_tileCol, sizeof(TileColDesc) * 20);
            MOS_ZeroMemory(m_sliceDesc, sizeof(SliceDescriptor) * 600);
            MOS_ZeroMemory(m_alfApsArray, sizeof(CodecVvcAlfData) * 8);
            MOS_ZeroMemory(m_lmcsApsArray, sizeof(CodecVvcLmcsData) * 4);
            MOS_ZeroMemory(m_lmcsReshaperInfo, sizeof(ApsLmcsReshapeInfo) * 4);
        };

        //!
        //! \brief  VvcBasicFeature deconstructor
        //!
        virtual ~VvcBasicFeature();

        //!
        //! \brief  Initialize vvc basic feature CodechalSetting
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init(void *setting) override;

        //!
        //! \brief  Update vvc decodeParams
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Update(void *params) override;

        //!
        //! \brief  Update APS
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS UpdateAPS(void *params);

        //!
        //! \brief  Check if ALF params are out of valid range
        //! \param    [in] alfData
        //!           Pointer to an ALF data structure
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CheckAlfRange(CodecVvcAlfData* alfData);

        //!
        //! \brief  Detect conformance conflict and do error concealment
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ErrorDetectAndConceal();

        //!
        //! \brief  Detect slice duplication and reorder and do error concealment for long format decoding
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SliceErrorHandlingLF();

        //!
        //! \brief    Get subpic Index from Subpic ID
        //! \param    [in] subPicId
        //!           Subpic ID value
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        int16_t GetSubPicIdxFromSubPicId(uint16_t subPicId);

        MOS_STATUS UpdateNumRefForList();  //Update Correct NumRefForList

        // Parameters passed from application
        uint16_t                        m_frameWidthAlignedMinBlk  = 0;            //!< Picture Width aligned to minBlock
        uint16_t                        m_frameHeightAlignedMinBlk = 0;            //!< Picture Height aligned to minBlock
        uint8_t                         m_vvcDepthIndicator        = 0;            //!< Indicate it is 8/10/12 bit VVC
        uint8_t                         m_numRefForList0           = 0;
        uint8_t                         m_numRefForList1           = 0;
        CodecVvcPicParams               *m_vvcPicParams            = nullptr;      //!< Pointer to VVC picture parameter
        CodecVvcSliceParams             *m_vvcSliceParams          = nullptr;      //!< Pointer to VVC slice parameter
        CodecVvcSubpicParam             *m_subPicParams            = nullptr;      //!< Pointer to VVC SubPic Parameter
        CodecVvcSliceStructure          *m_sliceStructParams       = nullptr;      //!< Pointer to Slice Structure parameter
        CodecVvcRplStructure            *m_rplParams               = nullptr;      //!< Pointer to RPL parameter
        CodecVvcTileParam               *m_tileParams              = nullptr;      //!< pointer to Tile Parameter

        //Internal parameters
        //APS buffer arrays
        CodecVvcAlfData                 m_alfApsArray[8];                          //!< 8 internal ALF APS arrays
        CodecVvcLmcsData                m_lmcsApsArray[4];                         //!< 4 internal LMCS APS arrays
        CodecVvcQmData                  m_scalingListArray[8];                     //!< 8 internal Scaling List arrays
        ApsLmcsReshapeInfo              m_lmcsReshaperInfo[4];                     //!< 4 internal LMCS reshaper info
        uint8_t                         m_lmcsReshaperReady = 0;                   //!< bit0 corresponds to Lmcs0
        uint8_t                         m_activeAlfMask = 0;                       //!< valid flags for each ALF table
        uint8_t                         m_activeLmcsMask = 0;                      //!< valid flags for each LMCS table
        uint8_t                         m_activeScalingListMask = 0;               //!< valid flags for each scaling list table
        uint8_t                         m_concealAlfMask = 0;                      //!< out-of-range flag for each ALF table
        uint8_t                         m_numAlf = 0;                              //!< Accumulated valid ALF number
        uint8_t                         m_numLmcs = 0;                             //!< Accumulated valid LMCS number
        uint8_t                         m_numScalingList = 0;                      //!< Accumulated valid scaling list number

        //Picture
        uint16_t                        m_picWidthInCtu = 0;
        uint16_t                        m_picHeightInCtu = 0;

        //Tile structure
        uint16_t                        m_tileCols = 0;
        uint16_t                        m_tileRows = 0;
        uint16_t                        m_maxTileWidth = 0;
        TileRowDesc                     m_tileRow[440];
        TileColDesc                     m_tileCol[20];

        //Sub-Pic Structure
        uint16_t                        m_sliceIdxInPicScanOrder[600];             //!< Internal buffer to store global slice scan order in picture

        //Slice structure
        int16_t                         m_curSlice = 0;                            //!< Current slice index
        SliceDescriptor                 m_sliceDesc[600];
        std::vector<int16_t>            m_sliceIdxInOrder;                         //!< Valid slice index in order with duplicated slice removed and reordered slices reversed

        //DPB+RPL
        VvcReferenceFrames              m_refFrames;                               //!< Reference frames
        std::vector<uint32_t>           m_refFrameIndexList;                       //!< Reference frame index list
        RefrenceAssociatedBuffer<MOS_BUFFER, VvcMvBufferOpInf, VvcBasicFeature>
                                        m_mvBuffers;                               //!< Reference associated buffers

        bool                            m_frameCompletedFlag        = false;       //!< Command packet preparation done for the whole frame
        bool                            m_shortFormatInUse          = false;       //!< flag to indicate if short format in use

        CodechalHwInterfaceNext         *m_hwInterface              = nullptr;

    protected:
        virtual MOS_STATUS SetRequiredBitstreamSize(uint32_t requiredSize) override;
        //!
        //! \brief    Reconstruct picture partition, including slice/tile/subpic
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ReconstructPartition(CodechalDecodeParams *decodeParams);
        MOS_STATUS ReconstructSlice();
        MOS_STATUS ReconstructTile();
        MOS_STATUS SetSubPicStruct();
        int16_t GetSubpicWidthInTile(uint16_t startCtu, uint16_t endCtu, int16_t &startTile, int16_t &endTile);
        int16_t GetSubpicHeightInTile(uint16_t startCtu, uint16_t endCtu, int16_t &startTile, int16_t &endTile);

        MOS_STATUS SetPictureStructs(CodechalDecodeParams *decodeParams);
        virtual MOS_STATUS CheckProfileCaps();

        std::shared_ptr<mhw::vdbox::vvcp::Itf> m_vvcpItf     = nullptr;
        PMOS_INTERFACE                         m_osInterface = nullptr;

    MEDIA_CLASS_DEFINE_END(decode__VvcBasicFeature)
    };

}  // namespace decode

#endif  // !__DECODE_VVC_BASIC_FEATURE_H__
