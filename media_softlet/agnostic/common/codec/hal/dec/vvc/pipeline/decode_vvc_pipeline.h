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
//! \file     decode_vvc_pipeline.h
//! \brief    Defines the interface for vvc decode pipeline
//!
#ifndef __DECODE_VVC_PIPELINE_H__
#define __DECODE_VVC_PIPELINE_H__

#include "decode_pipeline.h"
#include "codec_def_decode_vvc.h"
#include "decode_vvc_basic_feature.h"
#include "media_cmd_packet.h"
#include "media_debug_utils.h"

namespace decode
{
    class VvcDecodePkt;
    class VvcPipeline : public DecodePipeline
    {
    public:
        //!
        //! \brief  DecodePipeline constructor
        //! \param  [in] hwInterface
        //!         Pointer to CodechalHwInterface
        //! \param  [in] debugInterface
        //!         Pointer to CodechalDebugInterface
        //!
        VvcPipeline(
            CodechalHwInterfaceNext *   hwInterface,
            CodechalDebugInterface *debugInterface);

        virtual ~VvcPipeline() {}

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \param  [in] params
        //!         Pointer to the input parameters
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare(void *params) final;

        //!
        //! \brief  Finish the execution for each frame
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute() final;

        virtual MOS_STATUS GetStatusReport(void *status, uint16_t numStatus) override;

        uint32_t GetCompletedReport();

        virtual MOS_STATUS Destroy() override;

        MHW_BATCH_BUFFER *GetSliceLvlCmdBuffer();

        PMHW_BATCH_BUFFER GetTileLvlCmdBuffer();
        uint32_t          GetSliceLvlBufSize() { return m_sliceLvlBufSize; }
        uint32_t          GetTileLvlBufSize() { return m_tileLvlBufSize; }

        DeclareDecodePacketId(vvcDecodePacketId);
        DeclareDecodePacketId(vvcDecodeS2LPktId);
        DeclareDecodePacketId(vvcPictureSubPacketId);
        DeclareDecodePacketId(vvcSliceSubPacketId);
        DeclareDecodePacketId(vvcCpSubPacketId);

    protected:
        virtual MOS_STATUS Initialize(void *settings) override;
        virtual MOS_STATUS Uninitialize() override;

        //!
        //! \brief  User Feature Key Report
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS UserFeatureReport() override;

        //!
        //! \brief  Create sub packets
        //! \param  [in] codecSettings
        //!         Codechal settings
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS CreateSubPackets(DecodeSubPacketManager& subPacketManager, CodechalSetting &codecSettings) override;

        //!
        //! \brief  Initialize media context for decode pipeline
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS InitContext();

        //!
        //! \brief    Initialize MMC state
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        virtual MOS_STATUS InitMmcState();


    #if USE_CODECHAL_DEBUG_TOOL
        //! \brief    Dump the parameters
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpParams(VvcBasicFeature &basicFeature);

        //! \brief    Dump the picture parameters
        //!
        //! \param    [in] picParams
        //!           Pointer to CodecVvcPicParams
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpPicParams(
            CodecVvcPicParams *picParams, 
            bool shortFormatInUse);

        //! \brief    Dump the APS ALF parameters
        //!
        //! \param    [in] alfDataBuf
        //!           Pointer to an array of CodecVvcAlfData
        //! \param    [in] numAlf
        //!           Number of ALF
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpApsAlfData(
            CodecVvcAlfData *alfDataBuf,
            uint32_t        numAlf);

        //! \brief    Dump the APS ALF parameters
        //!
        //! \param    [in] lmcsDataBuf
        //!           Pointer to an array of CodecVvcLmcsData
        //! \param    [in] numLmcs
        //!           Number of LMCS
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpApsLmcsData(
            CodecVvcLmcsData *lmcsDataBuf,
            uint32_t         numLmcs);

        //! \brief    Dump the APS quant matrix parameters
        //!
        //! \param    [in] quantMatrixBuf
        //!           Pointer to an array of CodecVvcQmData
        //! \param    [in] numScalingList
        //!           Number of scaling list
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpApsQuantMatrix(
            CodecVvcQmData *quantMatrixBuf,
            uint32_t       numScalingList);

        //! \brief    Dump the tile parameters
        //!
        //! \param    [in] tileParams
        //!           Pointer to an array of CodecVvcTileParam
        //! \param    [in] numElements
        //!           Number of dimention value
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpTileParams(
            CodecVvcTileParam *tileParams,
            uint32_t          numElements);

        //! \brief    Dump the subpic parameters
        //!
        //! \param    [in] subpicParamsBuf
        //!           Pointer to an array of CodecVvcSubpicParam
        //! \param    [in] numSubpics
        //!           Number of subpics
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpSubpicParams(
            CodecVvcSubpicParam *subpicParamsBuf,
            uint32_t            numSubpics);

        //! \brief    Dump the slice parameters
        //!
        //! \param    [in] sliceStructParamsBuf
        //!           Pointer to an array of CodecVvcSliceStructure
        //! \param    [in] numSlcStruct
        //!           Number of slice structs
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpSliceStructParams(
            CodecVvcSliceStructure *sliceStructParamsBuf,
            uint32_t               numSlcStruct);

        //! \brief    Dump the RPL parameters
        //!
        //! \param    [in] rplParams
        //!           Pointer to an array of CodecVvcRplStructure
        //! \param    [in] numRpls
        //!           Number of RPLs
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpRplStructParams(
            CodecVvcRplStructure *rplParams,
            uint32_t             numRpls,
            bool                 shortFormatInUse);

        //! \brief    Dump the slice control parameters
        //!
        //! \param    [in] sliceParams
        //!           Pointer to an array of CodecVvcSliceParams
        //! \param    [in] numSlices
        //!           Number of slices
        //! \param    [in] shortFormatInUse
        //!           short format flag
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpSliceParams(
            CodecVvcSliceParams* sliceParams,
            uint32_t             numSlices,
            bool                 shortFormatInUse);
    #endif

        //!
        //! \brief  Active decode packets
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS ActivateDecodePackets();

        //!
        //! \brief  Create VVC Decode feature manager for Gen12
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS CreateFeatureManager() override;

        virtual MOS_STATUS AllocateResources(VvcBasicFeature &basicFeature);



    private:
        CodechalHwInterfaceNext* m_hwInterface = nullptr;


    protected:
        VvcBasicFeature *m_basicFeature = nullptr;              //!< VVC Basic Feature
        uint16_t        m_passNum          = 1;                 //!< Decode pass number
        bool            m_isFirstSliceInFrm = true;             //!< First tile in the first frame
        BatchBufferArray *m_sliceLevelBBArray = nullptr;        //!< Point to slice level batch buffer
        BatchBufferArray *m_tileLevelBBArray  = nullptr;          //!< Pointer to third level batch buffer
        uint32_t          m_sliceLvlBufSize   = 0;
        uint32_t          m_tileLvlBufSize    = 0;
        VvcDecodePkt     *m_vvcDecodePkt      = nullptr;  //!< VVC decode packet
        CmdPacket        *m_vvcDecodeS2LPkt   = nullptr;  //!< VVC decode S2L packet

    MEDIA_CLASS_DEFINE_END(decode__VvcPipeline)
    };

#if USE_CODECHAL_DEBUG_TOOL
    void DumpDecodeVvcPicParams(CodecVvcPicParams *picParams, std::string fileName, bool shortFormatInUse);
    void DumpDecodeVvcAlfParams(CodecVvcAlfData *alfDataBuf, uint32_t numAlf, std::string fileName);
    void DumpDecodeVvcLmcsParams(CodecVvcLmcsData *lmcsDataBuf, uint32_t numLmcs, std::string fileName);
    void DumpDecodeVvcIqParams(CodecVvcQmData *quantMatrixBuf, uint32_t numScalingList, std::string fileName);
    void DumpDecodeVvcTileParams(CodecVvcTileParam *tileParams, uint32_t numElements, std::string fileName);
    void DumpDecodeVvcSubpicParams(CodecVvcSubpicParam *subpicParamsBuf, uint32_t numSubpics, std::string fileName);
    void DumpDecodeVvcSliceStructureParams(CodecVvcSliceStructure *sliceStructParamsBuf, uint32_t numSlcStruct, std::string fileName);
    void DumpDecodeVvcRplStructureParams(CodecVvcRplStructure *rplParams, uint32_t numRpls, std::string fileName);
    void DumpDecodeVvcSliceParams(CodecVvcSliceParams *sliceParams, uint32_t numSlices, std::string fileName, bool shortFormatInUse);
#endif
}
#endif // !__DECODE_VVC_PIPELINE_H__
