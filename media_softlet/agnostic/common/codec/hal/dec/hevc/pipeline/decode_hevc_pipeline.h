/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     decode_hevc_pipeline.h
//! \brief    Defines the interface for hevc decode pipeline
//!
#ifndef __DECODE_HEVC_PIPELINE_H__
#define __DECODE_HEVC_PIPELINE_H__

#include "decode_pipeline.h"
#include "decode_hevc_basic_feature.h"
#include "decode_hevc_scalability_defs.h"
#include "decode_hevc_scalability_option.h"
#include "decode_phase.h"

namespace decode {

class HevcPipeline : public DecodePipeline
{
public:
    enum HevcDecodeMode
    {
        baseDecodeMode,         //!< Legacy decode mode with single pipe
        virtualTileDecodeMode,  //!< virtual tile decode mode
        separateTileDecodeMode, //!< IBC/PAL multiple tile decode mode with single pipe
        realTileDecodeMode,     //!< Real tile decode mode
    };

    //!
    //! \brief  HevcPipeline constructor
    //! \param  [in] hwInterface
    //!         Pointer to CodechalHwInterface
    //! \param  [in] debugInterface
    //!         Pointer to CodechalDebugInterface
    //!
    HevcPipeline(CodechalHwInterfaceNext *hwInterface, CodechalDebugInterface *debugInterface);

    virtual ~HevcPipeline() {}

    //!
    //! \brief  Return if short format decode in use
    //! \return bool
    //!         True if short format in use, else false
    //!
    bool IsShortFormat();

    //!
    //! \brief  Return the Hevc decode mode
    //! \return bool
    //!         True if short format in use, else false
    //!
    HevcDecodeMode GetDecodeMode();

    //! \brief  Get FE separate submission flag
    //! \return bool
    //!         Return true if FE separate submission, else return false
    //!
    bool IsFESeparateSubmission();

    //!
    //! \brief  Return the slice level command buffer
    //! \return MHW_BATCH_BUFFER*
    //!         Point to slice level command buffer if success, else nullptr
    //!
    MHW_BATCH_BUFFER* GetSliceLvlCmdBuffer();
    //!
    //! \brief  Declare Regkeys in the scope of hevc decode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS InitUserSetting(MediaUserSettingSharedPtr userSettingPtr) override;

    DeclareDecodePacketId(hucS2lPacketId);
    DeclareDecodePacketId(hevcLongPacketId);
    DeclareDecodePacketId(hevcFrontEndPacketId);
    DeclareDecodePacketId(hevcBackEndPacketId);
    DeclareDecodePacketId(hevcRealTilePacketId);
    DeclareDecodePacketId(hevcPictureSubPacketId);
    DeclareDecodePacketId(hevcSliceSubPacketId);
    DeclareDecodePacketId(hevcTileSubPacketId);

protected:
    //!
    //! \brief  Initialize the decode pipeline
    //! \param  [in] settings
    //!         Pointer to the initialize settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(void *settings) override;

    //!
    //! \brief  Uninitialize the decode pipeline
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Uninitialize() override;

    //!
    //! \brief  Finish the execution for each frame
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Execute() override;

    //!
    //! \brief  User Feature Key Report
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UserFeatureReport() override;

    //!
    //! \brief  create media feature manager
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateFeatureManager() override;

    //!
    //! \brief  Create sub packets
    //! \param  [in] codecSettings
    //!         Point to codechal settings
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;

    //!
    //! \brief  Initialize context option
    //! \param  [in] scalPars
    //!         Hevc scalability parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitContexOption(HevcScalabilityPars& scalPars);

    //!
    //! \brief  Initialize Hevc decode mode
    //! \param  [in] scalabMode
    //!         Decode scalability mode
    //! \param  [in] basicFeature
    //!         Hevc decode basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS InitDecodeMode(ScalabilityMode scalabMode, HevcBasicFeature &basicFeature);

    //!
    //! \brief   Add one phase with pass number and pipe number
    //! \param  [in] pass
    //!         Pass number for phase
    //! \param  [in] pipe
    //!         Pipe number for phase
    //! \param  [in] activePipeNum
    //!         Acutive pipe number for current pass
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    template<typename T>
    MOS_STATUS CreatePhase(uint8_t pass = 0, uint8_t pipe = 0, uint8_t activePipeNum = 1);

    //!
    //! \brief  Create hevc decode phase list for current frame
    //! \param  [in] basicFeature
    //!         Hevc decode basic feature
    //! \param  [in] scalabMode
    //!         Decode scalability mode
    //! \param  [in] numPipe
    //!         Number of pipe for currently scalability mode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreatePhaseList(HevcBasicFeature &basicFeature, const ScalabilityMode scalabMode, const uint8_t numPipe);

    //!
    //! \brief  Destroy hevc decode phase list
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DestoryPhaseList();

    //!
    //! \brief  Store dest surface to RefList.
    //          If deblocking present with SCC IBC Mode, the reference is temporal surface HevcBasicFeature::m_referenceBeforeLoopFilter,
    //          need to recover with dest surface after decode finished.
    //! \param  [in] basicFeature
    //!         Hevc decode basic feature
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS StoreDestToRefList(HevcBasicFeature &basicFeature);

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief  Earlier stop for hw error status
    //! \param  [in] status
    //!         Status report from HW
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS HwStatusCheck(const DecodeStatusMfx &status) override;
#endif

#if USE_CODECHAL_DEBUG_TOOL
    //! \brief    Dump the picture parameters
    //!
    //! \param    [in] picParams
    //!           Pointer to CODEC_HEVC_PIC_PARAMS
    //! \param    [in] extPicParams
    //!           Pointer to CODEC_HEVC_EXT_PIC_PARAMS
    //! \param    [in] sccPicParams
    //!           Pointer to CODEC_HEVC_SCC_PIC_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpPicParams(
        PCODEC_HEVC_PIC_PARAMS     picParams,
        PCODEC_HEVC_EXT_PIC_PARAMS extPicParams,
        PCODEC_HEVC_SCC_PIC_PARAMS sccPicParams);

    //! \brief    Dump the slice parameters
    //!
    //! \param    [in] sliceParams
    //!           Pointer to CODEC_HEVC_SLICE_PARAMS
    //! \param    [in] extSliceParams
    //!           Pointer to CODEC_HEVC_EXT_SLICE_PARAMS
    //! \param    [in] numSlices
    //!           Number of slices
    //! \param    [in] shortFormatInUse
    //!           short format flag
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSliceParams(
        PCODEC_HEVC_SLICE_PARAMS     sliceParams,
        PCODEC_HEVC_EXT_SLICE_PARAMS extSliceParams,
        uint32_t                     numSlices,
        bool                         shortFormatInUse);

    //! \brief    Dump the subsets parameters
    //!
    //! \param    [in] subsetsParams
    //!           Pointer to CODEC_HEVC_SUBSET_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpSubsetsParams(PCODEC_HEVC_SUBSET_PARAMS subsetsParams);

    //! \brief    Dump the quantization matrix parameters
    //!
    //! \param    [in] matrixData
    //!           Pointer to CODECHAL_HEVC_IQ_MATRIX_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS DumpIQParams(PCODECHAL_HEVC_IQ_MATRIX_PARAMS matrixData);

    //! \brief    Dump the second level batch buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DumpSecondLevelBatchBuffer();
#endif

protected:
    HevcBasicFeature   *m_basicFeature = nullptr; //!< Point to hevc basic feature

    HevcDecodeMode      m_decodeMode = baseDecodeMode;

    BatchBufferArray   *m_secondLevelBBArray = nullptr; //!< Point to second level batch buffer
    const uint32_t      m_secondLevelBBNum   = 8;       //!< Number of second level batch buffer

    DecodeHevcScalabilityOption m_scalabOption; //!< Hevc decode scalability option

    std::vector<DecodePhase *>  m_phaseList;    //!< Phase list

    bool m_allowVirtualNodeReassign = false;    //!< Whether allow virtual node reassign

#if (_DEBUG || _RELEASE_INTERNAL)
    uint32_t m_rtFrameCount = 0; //!< frame count for real tile decoding
    uint32_t m_vtFrameCount = 0; //!< frame count for virtual tile decoding
    uint32_t m_spFrameCount = 0; //!< frame count for single pipe decoding
    bool     m_reportHucStatus = false; //!< Flag for reporting huc status to regkey
    bool     m_reportHucCriticalError = false; //!< Flag for reporting huc critical error to regkey
#endif

MEDIA_CLASS_DEFINE_END(decode__HevcPipeline)
};

}
#endif // !__DECODE_HEVC_PIPELINE_H__
