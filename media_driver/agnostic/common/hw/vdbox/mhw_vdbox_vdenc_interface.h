/*
* Copyright (c) 2017-2018, Intel Corporation
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

//! \file     mhw_vdbox_vdenc_interface.h
//! \brief    MHW interface for constructing Vdenc commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox Vdenc commands across all platforms 
//!

#ifndef _MHW_VDBOX_VDENC_INTERFACE_H_
#define _MHW_VDBOX_VDENC_INTERFACE_H_

#include "mhw_vdbox.h"
#include "mhw_mi.h"
#include "codec_def_encode_hevc.h"

typedef struct _MHW_VDBOX_VD_PIPE_FLUSH_PARAMS
{
    union
    {
        struct
        {
            uint16_t       bWaitDoneHEVC : 1;
            uint16_t       bWaitDoneVDENC : 1;
            uint16_t       bWaitDoneMFL : 1;
            uint16_t       bWaitDoneMFX : 1;
            uint16_t       bWaitDoneVDCmdMsgParser : 1;
            uint16_t       bFlushHEVC : 1;
            uint16_t       bFlushVDENC : 1;
            uint16_t       bFlushMFL : 1;
            uint16_t       bFlushMFX : 1;
            uint16_t                 : 7;
        };
        struct
        {
            uint16_t       Value;
        };
    }Flags;
} MHW_VDBOX_VD_PIPE_FLUSH_PARAMS, *PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS;

typedef struct _MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS
{
    uint8_t maxTuSize;
    uint8_t maxCuSize;
    uint8_t numImePredictors;
    uint8_t numMergeCandidateCu64x64;
    uint8_t numMergeCandidateCu32x32;
    uint8_t numMergeCandidateCu16x16;
    uint8_t numMergeCandidateCu8x8;
    bool setQpRoiCtrl;
    int8_t  forceQp;
    uint8_t roiCtrl;
    uint8_t puTypeCtrl;
}MHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS, *PMHW_VDBOX_VDENC_STREAMIN_STATE_PARAMS;

typedef struct _MHW_VDBOX_VDENC_CQPT_STATE_PARAMS
{
    uint16_t    wPictureCodingType;
    bool        bFTQEnabled;
    bool        bBlockBasedSkip;
    bool        bTransform8x8Flag;
} MHW_VDBOX_VDENC_CQPT_STATE_PARAMS, *PMHW_VDBOX_VDENC_CQPT_STATE_PARAMS;

typedef struct _MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS
{
    bool        bWeightedPredEnabled;
    uint32_t    dwDenom;
    uint8_t     ucList;
    char        LumaWeights[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    int16_t     LumaOffsets[2][CODEC_MAX_NUM_REF_FRAME_HEVC];
    char        ChromaWeights[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];
    int16_t     ChromaOffsets[2][CODEC_MAX_NUM_REF_FRAME_HEVC][2];
    uint32_t    dwChromaDenom;
    bool        isLowDelay = true;
} MHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS, *PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS;

typedef struct _MHW_VDBOX_VDENC_CMD1_PARAMS
{
    uint32_t                                Mode;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS       pHevcEncPicParams;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS         pHevcEncSlcParams;
    PCODEC_VP9_ENCODE_PIC_PARAMS            pVp9EncPicParams = nullptr;;
    uint8_t                                *pucVdencMvCosts;
    uint8_t                                *pucVdencRdMvCosts;
    uint8_t                                *pucVdencHmeMvCosts;
    uint8_t                                *pucVdencModeCosts;
    void                                   *pInputParams;
    uint16_t                                usSADQPLambda = 0;
    uint16_t                                usRDQPLambda = 0;
    bool                                    bHevcVisualQualityImprovement = false;  //!< VQI enable flag
} MHW_VDBOX_VDENC_CMD1_PARAMS, *PMHW_VDBOX_VDENC_CMD1_PARAMS;

struct MHW_VDBOX_VDENC_CMD2_STATE
{
    uint32_t                                Mode = 0;

    // HEVC
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS      pHevcEncSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS       pHevcEncPicParams = nullptr;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS         pHevcEncSlcParams = nullptr;
    bool                                    bSAOEnable = false;
    bool                                    bRoundingEnabled = false;
    bool                                    bStreamInEnabled = false;
    bool                                    bROIStreamInEnabled = false;
    bool                                    bUseDefaultQpDeltas = false;
    bool                                    bPanicEnabled = false;
    bool                                    bPartialFrameUpdateEnable = false;
    uint32_t                                roundInterValue = 0;
    uint32_t                                roundIntraValue = 0;

    // VP9
    PCODEC_VP9_ENCODE_PIC_PARAMS            pVp9EncPicParams = nullptr;
    bool                                    bSegmentationEnabled = false;
    PMHW_VDBOX_VP9_SEGMENT_STATE            pVp9SegmentState = nullptr;
    PCODEC_VP9_ENCODE_SEQUENCE_PARAMS       pVp9EncSeqParams = nullptr;
    bool                                    bPrevFrameSegEnabled;
    bool                                    bDynamicScalingEnabled = false;
    bool                                    temporalMVpEnable = false;

    // Common
    uint8_t                                 ucNumRefIdxL0ActiveMinus1 = 0;
    uint8_t                                 ucNumRefIdxL1ActiveMinus1 = 0;
    uint16_t                                usSADQPLambda = 0;
    uint16_t                                usRDQPLambda = 0;
    bool                                    bPakOnlyMultipassEnable = false;
    void                                    *pInputParams = nullptr;
    bool                                    bHevcVisualQualityImprovement = false;  //!< VQI enable flag
    
    bool                                    bTileReplayEnable = false;
    bool                                    bCaptureModeEnable = false;
    uint8_t                                 m_WirelessSessionID = 0;
    bool                                    bIsLowDelayB = false;
    int8_t                                  *pRefIdxMapping = nullptr;
    uint8_t                                 recNotFilteredID = 0;
    virtual ~MHW_VDBOX_VDENC_CMD2_STATE() {}
};
using PMHW_VDBOX_VDENC_CMD2_STATE = std::shared_ptr<MHW_VDBOX_VDENC_CMD2_STATE>;

struct MHW_VDBOX_VDENC_WALKER_STATE_PARAMS
{
    uint32_t                                Mode = 0;
    uint32_t                                slcIdx = 0;
    PCODEC_AVC_ENCODE_SEQUENCE_PARAMS       pAvcSeqParams = nullptr;
    PCODEC_AVC_ENCODE_PIC_PARAMS            pAvcPicParams = nullptr;
    PCODEC_AVC_ENCODE_SLICE_PARAMS          pAvcSlcParams = nullptr;
    PCODEC_HEVC_ENCODE_SEQUENCE_PARAMS      pHevcEncSeqParams = nullptr;
    PCODEC_HEVC_ENCODE_PICTURE_PARAMS       pHevcEncPicParams = nullptr;
    PCODEC_HEVC_ENCODE_SLICE_PARAMS         pEncodeHevcSliceParams = nullptr;
    PCODEC_VP9_ENCODE_PIC_PARAMS            pVp9EncPicParams = nullptr;
    virtual ~MHW_VDBOX_VDENC_WALKER_STATE_PARAMS() {}
};
using PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS = MHW_VDBOX_VDENC_WALKER_STATE_PARAMS * ;

//!  MHW Vdbox Vdenc interface
/*!
This class defines the interfaces for constructing Vdbox Vdenc commands across all platforms
*/
class MhwVdboxVdencInterface
{
protected:

    PMOS_INTERFACE              m_osInterface = nullptr; //!< Pointer to OS interface

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {}; //!< Cacheability settings

    bool                        m_rowstoreCachingSupported = 0;
    MHW_VDBOX_ROWSTORE_CACHE    m_vdencRowStoreCache = {};    //!< vdenc row store cache
    MHW_VDBOX_ROWSTORE_CACHE    m_vdencIpdlRowstoreCache = {}; //!< vdenc IntraPred row store cache
    bool                        m_rhoDomainStatsEnabled = false; //! indicate if rho domain stats is enabled
    bool                        m_perfModeSupported = true; //! indicate perf mode is supported

    static const bool m_vdencFTQEnabled[NUM_VDENC_TARGET_USAGE_MODES];
    static const bool m_vdencBlockBasedSkipEnabled[NUM_VDENC_TARGET_USAGE_MODES];

    //!
    //! \brief    Constructor
    //!
    MhwVdboxVdencInterface(PMOS_INTERFACE osInterface);

    //!
    //! \brief    Add a resource to the command buffer 
    //! \details  Internal function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer
    //!
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           Command buffer to which resource is added
    //! \param    [in] params
    //!           Parameters necessary to add the resource
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE osInterface,
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_RESOURCE_PARAMS params);

    virtual MOS_STATUS InitRowstoreUserFeatureSettings() = 0;

    //!
    //! \brief    Translate MOS type format to Mediastate surface format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    [in] format
    //!           MOS type format
    //! \return   uint32_t
    //!           media state surface format
    //!
    uint32_t MosToMediaStateFormat(MOS_FORMAT format);

public:

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterface() {}

    //!
    //! \brief    Judge if row store caching supported
    //!
    //! \return   bool
    //!           true if supported, else false
    //!
    inline bool IsRowStoreCachingSupported()
    {
        return m_rowstoreCachingSupported;
    }

    //!
    //! \brief    Judge if vdenc row store caching enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsvdencRowstoreCacheEnabled()
    {
        return m_vdencRowStoreCache.bEnabled ? true : false;
    }

    //!
    //! \brief    Judge if rho domain stats is enabled
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsRhoDomainStatsEnabled()
    {
        return m_rhoDomainStatsEnabled;
    }

    //!
    //! \brief    Judge if perf mode is supported
    //!
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool IsPerfModeSupported()
    {
        return m_perfModeSupported;
    }

    //!
    //! \brief    get vdenc FTQ supported
    //!
    //! \param    [in] idx
    //!           index of the array
    //! \return   bool
    //!           true if enabled, else false
    //!
    inline bool VdencFTQEnabled(uint32_t idx)
    {
        return m_vdencFTQEnabled[idx];
    }

    //!
    //! \brief    get Vdenc img state size
    //!
    //! \return   uint32_t
    //!           Vdenc img state size got
    //!
    virtual uint32_t GetVdencAvcImgStateSize() = 0;

    //!
    //! \brief    get Vdenc cost state size
    //!
    //! \return   uint32_t
    //!           Vdenc cost state size got
    //!
    virtual uint32_t GetVdencAvcCostStateSize() = 0;

    //!
    //! \brief    get cmd1
    //!
    //! \return   uint32_t
    //!           cmd1 size got
    //!
    virtual uint32_t GetVdencCmd1Size() = 0;

    //!
    //! \brief    get cmd2
    //!
    //! \return   uint32_t
    //!           cmd2 size got
    //!
    virtual uint32_t GetVdencCmd2Size() = 0;

    //!
    //! \brief    get Vdenc state commands data size
    //!
    //! \return   uint32_t
    //!           Vdenc state commands data size got
    //!
    virtual MOS_STATUS GetVdencStateCommandsDataSize(
        uint32_t                        mode,
        uint32_t                        waAddDelayInVDEncDynamicSlice,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize) = 0;

    //!
    //! \brief    Set cacheability settings
    //!
    //! \param    [in] cacheabilitySettings
    //!           Cacheability settings
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    MOS_STATUS SetCacheabilitySettings(
        MHW_MEMORY_OBJECT_CONTROL_PARAMS cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC])
    {
        MHW_FUNCTION_ENTER;

        uint32_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);
        return MOS_SecureMemcpy(m_cacheabilitySettings, size, cacheabilitySettings, size);
    }

    //!
    //! \brief    Programs base address of rowstore scratch buffers 
    //! \details  Internal function to get base address of rowstore scratch buffers 
    //!
    //! \param    [in] rowstoreParams
    //!           Rowstore parameters
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams) = 0;

    //!
    //! \brief    Adds VD Pipeline Flush command in command buffer
    //! \details  Client facing function to add VD Pipeline Flush command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdPipelineFlushCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_VD_PIPE_FLUSH_PARAMS  params) = 0;

    //!
    //! \brief    Adds VDENC Pipe Mode Select command in command buffer
    //! \details  Client facing function to add VDENC Pipe Mode Select command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params) = 0;

    //!
    //! \brief    Adds VDENC Pipe Buffer Address State command in command buffer
    //! \details  Client facing function to add VDENC Pipe Buffer Address State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS  params) = 0;

    //!
    //! \brief    Adds VDENC Src Surface State command in command buffer
    //! \details  Client facing function to add VDENC Src Surface State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params) = 0;

    //!
    //! \brief    Adds VDENC Ref Surface State command in command buffer
    //! \details  Client facing function to add VDENC Ref Surface State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params) = 0;

    //!
    //! \brief    Adds VDENC Dst Surface State command in command buffer
    //! \details  Client facing function to add VDENC Dst Surface State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \param    [in] numSurfaces
    //!           Number of surfaces
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencDsRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params,
        uint8_t                              numSurfaces) = 0;

    virtual MOS_STATUS AddVdencAvcCostStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_BATCH_BUFFER         batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS params)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Adds VDENC AVC Image State command in command buffer
    //! \details  Client facing function to add VDENC AVC Image State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencImgStateCmd(
        PMOS_COMMAND_BUFFER              cmdBuffer,
        PMHW_BATCH_BUFFER                batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS        params) = 0;

    //!
    //! \brief    Adds VDENC Walker State command in command buffer
    //! \details  Client facing function to add VDENC Walker State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params) = 0;

    //!
    //! \brief    Adds VDENC Const QPT State command in command buffer
    //! \details  Client facing function to add VDENC Const QPT State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencConstQPStateCmd(
        PMOS_COMMAND_BUFFER                cmdBuffer,
        PMHW_VDBOX_VDENC_CQPT_STATE_PARAMS params) = 0;

    //!
    //! \brief    Adds VDENC WeightsOffsets State command in command buffer
    //! \details  Client facing function to add VDENC WeightsOffsets State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!

    virtual MOS_STATUS AddVdencAvcWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS  params) = 0;

    //!
    //! \brief    Adds VDENC Costs State command in command buffer
    //! \details  Client facing function to add VDENC Costs State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                     cmdBuffer,
        PMHW_BATCH_BUFFER                       batchBuffer,
        PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS   params) = 0;

    virtual MOS_STATUS AddVdencSliceStateCmd(
        PMOS_COMMAND_BUFFER        cmdBuffer,
        PMHW_VDBOX_AVC_SLICE_STATE params)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS AddVdencControlStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Adds CMD1 command in command buffer
    //! \details  Client facing function to add CMD1 command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencCmd1Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS        params) = 0;

    //!
    //! \brief    Adds CMD2 command in command buffer
    //! \details  Client facing function to add VDENC HEVC VP9 IMG State command in command buffer
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] batchBuffer
    //!           Batch buffer to add to VDBOX_BUFFER_START
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail type
    //!
    virtual MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE         params) = 0;

};

#endif
