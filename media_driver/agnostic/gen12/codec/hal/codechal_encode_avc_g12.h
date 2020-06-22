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
//! \file     codechal_encode_avc_g12.h
//! \brief    This file defines the C++ class/interface for Gen12 platform's AVC
//!           DualPipe encoding to be used across CODECHAL components.
//!
#ifndef __CODECHAL_ENCODE_AVC_G12_H__
#define __CODECHAL_ENCODE_AVC_G12_H__

#include "codechal_encode_avc.h"
#include "codechal_kernel_intra_dist.h"
#include "codechal_encode_sw_scoreboard_g12.h"
#include "codechal_encode_singlepipe_virtualengine.h"

enum MbBrcUpdateBindingTableOffset
{
    mbBrcUpdateHistory     = 0,
    mbBrcUpdateMbQp        = 1,
    mbBrcUpdateRoi         = 2,
    mbBrcUpdateMbStat      = 3,
    mbBrcUpdateNumSurfaces = 4
};

enum BrcUpdateBindingTableOffset
{
    frameBrcUpdateHistory             = 0,
    frameBrcUpdatePakStatisticsOutput = 1,
    frameBrcUpdateImageStateRead      = 2,
    frameBrcUpdateImageStateWrite     = 3,
    frameBrcUpdateMbencCurbeWrite     = 4,
    frameBrcUpdateDistortion          = 5,
    frameBrcUpdateConstantData        = 6,
    frameBrcUpdateMbStat              = 7,
    frameBrcUpdateMvStat              = 8,
    frameBrcUpdateNumSurfaces         = 9
};

class CodechalEncodeAvcEncG12 : public CodechalEncodeAvcEnc
{
   public:
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncG12(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeAvcEncG12(const CodechalEncodeAvcEncG12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeAvcEncG12& operator=(const CodechalEncodeAvcEncG12&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeAvcEncG12();

    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] binary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] krnHeader
    //!           Pointer to kernel header
    //! \param    [out] krnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS GetKernelHeaderAndSize(
        void                         *binary,
        EncOperation                 operation,
        uint32_t                     krnStateIdx,
        void                         *krnHeader,
        uint32_t                     *krnSize);

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        int32_t  nullRendering) override;

    MOS_STATUS ExecuteKernelFunctions() override;

    virtual MOS_STATUS SceneChangeReport(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params) override;


    MOS_STATUS GenericEncodePictureLevel(
        PCODECHAL_ENCODE_AVC_GENERIC_PICTURE_LEVEL_PARAMS   params) override;

    MOS_STATUS GetTrellisQuantization(
        PCODECHAL_ENCODE_AVC_TQ_INPUT_PARAMS params,
        PCODECHAL_ENCODE_AVC_TQ_PARAMS       trellisQuantParams) override;

    MOS_STATUS InitializeState() override;

    MOS_STATUS InitKernelStateMbEnc() override;

    MOS_STATUS InitKernelStateBrc() override;

    MOS_STATUS InitBrcConstantBuffer(
        PCODECHAL_ENCODE_AVC_INIT_BRC_CONSTANT_BUFFER_PARAMS params) override;

    MOS_STATUS InitKernelStateWP() override;

    MOS_STATUS InitMbBrcConstantDataBuffer(
        PCODECHAL_ENCODE_AVC_INIT_MBBRC_CONSTANT_DATA_BUFFER_PARAMS params) override;

    MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams*        params,
        uint32_t*                            kernelOffset) override;

    MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params) override;

    MOS_STATUS SetCurbeAvcWP(
        PCODECHAL_ENCODE_AVC_WP_CURBE_PARAMS params) override;

    MOS_STATUS SetCurbeAvcBrcInitReset(
        PCODECHAL_ENCODE_AVC_BRC_INIT_RESET_CURBE_PARAMS params) override;

    MOS_STATUS SetCurbeAvcFrameBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;

    MOS_STATUS SetCurbeAvcMbBrcUpdate(
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_CURBE_PARAMS params) override;

    MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER                       cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params) override;

    MOS_STATUS SendAvcWPSurfaces(
        PMOS_COMMAND_BUFFER                    cmdBuffer,
        PCODECHAL_ENCODE_AVC_WP_SURFACE_PARAMS params) override;

    MOS_STATUS SendAvcBrcFrameUpdateSurfaces(
        PMOS_COMMAND_BUFFER                            cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;

    MOS_STATUS SendAvcBrcMbUpdateSurfaces(
        PMOS_COMMAND_BUFFER                            cmdBuffer,
        PCODECHAL_ENCODE_AVC_BRC_UPDATE_SURFACE_PARAMS params) override;

    MOS_STATUS SetupROISurface() override;

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

    MOS_STATUS InitMmcState() override;

    //!
    //! \brief    Run MbEnc Kernel.
    //!
    //! \param    [in] mbEncIFrameDistInUse
    //!           MbEncIFrameDist in use or not
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS MbEncKernel(
        bool mbEncIFrameDistInUse) override;

    //!
    //! \brief    Resize buffers due to resoluton change.
    //! \details  Resize buffers due to resoluton change.
    //!
    //! \return   void
    //!
    virtual void ResizeOnResChange() override;

    MOS_STATUS InitKernelStateMe() override;

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;

    //!
    //! \brief    Set And Populate VE Hint parameters
    //! \details  Set Virtual Engine hint parameter and populate it to primary cmd buffer attributes
    //! \param    [in] cmdBuffer
    //!               Pointer to primary cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS  SetAndPopulateVEHintParams(
        PMOS_COMMAND_BUFFER  cmdBuffer);

    bool        m_useMdf = false;                    //!< Use MDF for MBEnc kernels.
    CodechalEncodeSwScoreboardG12 *m_swScoreboardState = nullptr;    //!< pointer to SW scoreboard ini state.

    //!
    //! \brief  Set up params for gpu context creation
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetGpuCtxCreatOption() override;

    //!
    //! \brief    Encode User Feature Key Report.
    //! \details  Report user feature values set by encode.
    //! 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    virtual MOS_STATUS UserFeatureKeyReport() override;

    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state

   protected:
    class SfdCurbe;
    class MbencCurbe;
    class BrcInitResetCurbe;
    class MbBrcUpdateCurbe;
    class FrameBrcUpdateCurbe;
    class WpCurbe;
    class EncKernelHeader;
   protected:

#if USE_CODECHAL_DEBUG_TOOL
    MOS_STATUS KernelDebugDumps();

    virtual MOS_STATUS PopulateBrcInitParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateBrcUpdateParam(
        void *cmd) override;

    virtual MOS_STATUS PopulateEncParam(
        uint8_t meMethod,
        void    *cmd) override;

    virtual MOS_STATUS PopulatePakParam(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_BATCH_BUFFER   secondLevelBatchBuffer) override;
#endif

   //private:
    static constexpr uint32_t m_sfdCostTableBufferSize                   = 52;
    static constexpr uint32_t m_refThreshold                             = 400;
    static constexpr uint32_t m_brcConstantsurfaceEarlySkipTableSize     = 128;
    static constexpr uint32_t m_brcConstantsurfaceModeMvCostSize         = 1664;
    static constexpr uint32_t m_brcConstantsurfaceRefcostSize            = 128;
    static constexpr uint32_t m_brcConstantsurfaceQpList0                = 32;
    static constexpr uint32_t m_brcConstantsurfaceQpList0Reserved        = 32;
    static constexpr uint32_t m_brcConstantsurfaceQpList1                = 32;
    static constexpr uint32_t m_brcConstantsurfaceQpList1Reserved        = 160;
    static constexpr uint32_t m_brcConstantsurfaceIntracostScalingFactor = 64;
    static constexpr uint32_t m_initBrcHistoryBufferSize                 = 880;
    static constexpr uint32_t m_brcConstantsurfaceWidth                  = 64;
    static constexpr uint32_t m_brcConstantsurfaceHeight                 = 53;
    static constexpr uint32_t m_brcConstantsurfaceLambdaSize             = 512;
    static constexpr uint32_t m_brcConstantsurfaceFtq25Size              = 64;
    static constexpr uint32_t m_defaultTrellisQuantIntraRounding         = 5;
    static constexpr uint32_t m_maxLambda                                = 0xEFFF;
    static constexpr uint32_t m_mbencNumTargetUsages                     = 3;
    static constexpr uint32_t m_initMbencBrcBufferSize                   = 128;
    static constexpr uint32_t m_mbTextureThreshold                       = 1024;
    static constexpr uint32_t m_adaptiveTxDecisionThreshold              = 128;
    static constexpr uint32_t brcHistoryBufferOffsetSceneChanged         = 0x2FC;

    static const uint32_t                           m_intraModeCostForHighTextureMB[CODEC_AVC_NUM_QP];
    static const uint8_t                            m_QPAdjustmentDistThresholdMaxFrameThresholdIPB[576];
    static const CODECHAL_ENCODE_AVC_IPCM_THRESHOLD m_IPCMThresholdTable[5];
    static const int32_t                            m_brcBTCounts[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const int32_t                            m_brcCurbeSize[CODECHAL_ENCODE_BRC_IDX_NUM];
    static const uint16_t                           m_lambdaData[256];
    static const uint8_t                            m_ftQ25[64];
    static const uint16_t                           m_refCostMultiRefQp[NUM_PIC_TYPES][64];
    static const uint32_t                           m_multiPred[NUM_TARGET_USAGE_MODES];
    static const uint32_t                           m_multiRefDisableQPCheck[NUM_TARGET_USAGE_MODES];

    CodechalKernelIntraDist *m_intraDistKernel = nullptr;
};

class CodechalEncodeAvcEncG12::MbencCurbe
{
   public:
    enum MBEncCurbeInitType
    {
        typeIDist,
        typeIFrame,
        typeIField,
        typePFrame,
        typePField,
        typeBFrame,
        typeBField
    };

    static const uint32_t m_mbEncCurbeNormalIFrame[89];
    static const uint32_t m_mbEncCurbeNormalIField[89];
    static const uint32_t m_mbEncCurbeNormalPFrame[89];
    static const uint32_t m_mbEncCurbeNormalPField[89];
    static const uint32_t m_mbEncCurbeNormalBFrame[89];
    static const uint32_t m_mbEncCurbeNormalBField[89];
    static const uint32_t m_mbEncCurbeIFrameDist[89];

    uint32_t GetMBEncCurbeDataSizeExcludeSurfaceIdx() { return 66*4; };

    void SetDefaultMbencCurbe(MBEncCurbeInitType initType)
    {
        switch (initType)
        {
        case typeIDist:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeIFrameDist),
                m_mbEncCurbeIFrameDist,
                sizeof(m_mbEncCurbeIFrameDist));
            break;

        case typeIFrame:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalIFrame),
                m_mbEncCurbeNormalIFrame,
                sizeof(m_mbEncCurbeNormalIFrame));
            break;

        case typeIField:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalIField),
                m_mbEncCurbeNormalIField,
                sizeof(m_mbEncCurbeNormalIField));
            break;

        case typePFrame:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalPFrame),
                m_mbEncCurbeNormalPFrame,
                sizeof(m_mbEncCurbeNormalPFrame));
            break;

        case typePField:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalPField),
                m_mbEncCurbeNormalPField,
                sizeof(m_mbEncCurbeNormalPField));
            break;

        case typeBFrame:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalBFrame),
                m_mbEncCurbeNormalBFrame,
                sizeof(m_mbEncCurbeNormalBFrame));
            break;

        case typeBField:
            MOS_SecureMemcpy(
                (void *)&m_curbe,
                sizeof(m_mbEncCurbeNormalBField),
                m_mbEncCurbeNormalBField,
                sizeof(m_mbEncCurbeNormalBField));
            break;

        default:
            CODECHAL_ENCODE_ASSERTMESSAGE("Invalid curbe type.");
            break;
        }
    };

    struct
    {
        // DW0
        union
        {
            struct
            {
                uint32_t m_skipModeEn         : MOS_BITFIELD_BIT(0);
                uint32_t m_adaptiveEn         : MOS_BITFIELD_BIT(1);
                uint32_t m_biMixDis           : MOS_BITFIELD_BIT(2);
                uint32_t                      : MOS_BITFIELD_RANGE(3, 4);
                uint32_t m_earlyImeSuccessEn  : MOS_BITFIELD_BIT(5);
                uint32_t                      : MOS_BITFIELD_BIT(6);
                uint32_t m_t8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
                uint32_t                      : MOS_BITFIELD_RANGE(8, 23);
                uint32_t m_earlyImeStop       : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        // DW1
        union
        {
            struct
            {
                uint32_t m_maxNumMVs           : MOS_BITFIELD_RANGE(0, 5);
                uint32_t m_extendedMvCostRange : MOS_BITFIELD_BIT(6);
                uint32_t                       : MOS_BITFIELD_RANGE(7, 15);
                uint32_t m_biWeight            : MOS_BITFIELD_RANGE(16, 21);
                uint32_t                       : MOS_BITFIELD_RANGE(22, 27);
                uint32_t m_uniMixDisable       : MOS_BITFIELD_BIT(28);
                uint32_t                       : MOS_BITFIELD_RANGE(29, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        // DW2
        union
        {
            struct
            {
                uint32_t m_lenSP    : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_maxNumSU : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_picWidth : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        // DW3
        union
        {
            struct
            {
                uint32_t m_srcSize                : MOS_BITFIELD_RANGE(0, 1);
                uint32_t                          : MOS_BITFIELD_RANGE(2, 3);
                uint32_t m_mbTypeRemap            : MOS_BITFIELD_RANGE(4, 5);
                uint32_t m_srcAccess              : MOS_BITFIELD_BIT(6);
                uint32_t m_refAccess              : MOS_BITFIELD_BIT(7);
                uint32_t m_searchCtrl             : MOS_BITFIELD_RANGE(8, 10);
                uint32_t m_dualSearchPathOption   : MOS_BITFIELD_BIT(11);
                uint32_t m_subPelMode             : MOS_BITFIELD_RANGE(12, 13);
                uint32_t m_skipType               : MOS_BITFIELD_BIT(14);
                uint32_t m_disableFieldCacheAlloc : MOS_BITFIELD_BIT(15);
                uint32_t m_interChromaMode        : MOS_BITFIELD_BIT(16);
                uint32_t m_fTEnable               : MOS_BITFIELD_BIT(17);
                uint32_t m_bmeDisableFBR          : MOS_BITFIELD_BIT(18);
                uint32_t m_blockBasedSkipEnable   : MOS_BITFIELD_BIT(19);
                uint32_t m_interSAD               : MOS_BITFIELD_RANGE(20, 21);
                uint32_t m_intraSAD               : MOS_BITFIELD_RANGE(22, 23);
                uint32_t m_subMbPartMask          : MOS_BITFIELD_RANGE(24, 30);
                uint32_t                          : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        // DW4
        union
        {
            struct
            {
                uint32_t m_picHeightMinus1                      : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_mvRestrictionInSliceEnable           : MOS_BITFIELD_BIT(16);
                uint32_t m_deltaMvEnable                        : MOS_BITFIELD_BIT(17);
                uint32_t m_trueDistortionEnable                 : MOS_BITFIELD_BIT(18);
                uint32_t m_enableWavefrontOptimization          : MOS_BITFIELD_BIT(19);
                uint32_t m_enableFBRBypass                      : MOS_BITFIELD_BIT(20);
                uint32_t m_enableIntraCostScalingForStaticFrame : MOS_BITFIELD_BIT(21);
                uint32_t m_enableIntraRefresh                   : MOS_BITFIELD_BIT(22);
                uint32_t m_reserved                             : MOS_BITFIELD_BIT(23);
                uint32_t m_enableDirtyRect                  : MOS_BITFIELD_BIT(24);
                uint32_t m_bCurFldIDR                           : MOS_BITFIELD_BIT(25);
                uint32_t m_constrainedIntraPredFlag             : MOS_BITFIELD_BIT(26);
                uint32_t m_fieldParityFlag                      : MOS_BITFIELD_BIT(27);
                uint32_t m_hmeEnable                            : MOS_BITFIELD_BIT(28);
                uint32_t m_pictureType                          : MOS_BITFIELD_RANGE(29, 30);
                uint32_t m_useActualRefQPValue                  : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        // DW5
        union
        {
            struct
            {
                uint32_t m_sliceMbHeight : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_refWidth      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_refHeight     : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        // DW6
        union
        {
            struct
            {
                uint32_t m_batchBufferEnd;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW6;

        // DW7
        union
        {
            struct
            {
                uint32_t m_intraPartMask          : MOS_BITFIELD_RANGE(0, 4);
                uint32_t m_nonSkipZMvAdded        : MOS_BITFIELD_BIT(5);
                uint32_t m_nonSkipModeAdded       : MOS_BITFIELD_BIT(6);
                uint32_t m_lumaIntraSrcCornerSwap : MOS_BITFIELD_BIT(7);
                uint32_t                          : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_mvCostScaleFactor      : MOS_BITFIELD_RANGE(16, 17);
                uint32_t m_bilinearEnable         : MOS_BITFIELD_BIT(18);
                uint32_t m_srcFieldPolarity       : MOS_BITFIELD_BIT(19);
                uint32_t m_weightedSADHAAR        : MOS_BITFIELD_BIT(20);
                uint32_t m_AConlyHAAR             : MOS_BITFIELD_BIT(21);
                uint32_t m_refIDCostMode          : MOS_BITFIELD_BIT(22);
                uint32_t                          : MOS_BITFIELD_BIT(23);
                uint32_t m_skipCenterMask         : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW7;

        struct
        {
            // DW8
            union
            {
                struct
                {
                    uint32_t m_mode0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mode1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mode2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mode3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW8;

            // DW9
            union
            {
                struct
                {
                    uint32_t m_mode4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mode5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mode6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mode7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW9;

            // DW10
            union
            {
                struct
                {
                    uint32_t m_mode8Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mode9Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_refIDCost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_chromaIntraModeCost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW10;

            // DW11
            union
            {
                struct
                {
                    uint32_t m_mv0Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mv1Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mv2Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mv3Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW11;

            // DW12
            union
            {
                struct
                {
                    uint32_t m_mv4Cost : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_mv5Cost : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_mv6Cost : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_mv7Cost : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW12;

            // DW13
            union
            {
                struct
                {
                    uint32_t m_qpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_qpPrimeCb : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_qpPrimeCr : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_targetSizeInWord : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW13;

            // DW14
            union
            {
                struct
                {
                    uint32_t m_SICFwdTransCoeffThreshold_0 : MOS_BITFIELD_RANGE(0, 15);
                    uint32_t m_SICFwdTransCoeffThreshold_1 : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_SICFwdTransCoeffThreshold_2 : MOS_BITFIELD_RANGE(24, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW14;

            // DW15
            union
            {
                struct
                {
                    uint32_t m_SICFwdTransCoeffThreshold_3 : MOS_BITFIELD_RANGE(0, 7);
                    uint32_t m_SICFwdTransCoeffThreshold_4 : MOS_BITFIELD_RANGE(8, 15);
                    uint32_t m_SICFwdTransCoeffThreshold_5 : MOS_BITFIELD_RANGE(16, 23);
                    uint32_t m_SICFwdTransCoeffThreshold_6 : MOS_BITFIELD_RANGE(24, 31);  // Highest Freq
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW15;
        } ModeMvCost;

        struct
        {
            // DW16
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_0;
                    SearchPathDelta m_spDelta_1;
                    SearchPathDelta m_spDelta_2;
                    SearchPathDelta m_spDelta_3;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW16;

            // DW17
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_4;
                    SearchPathDelta m_spDelta_5;
                    SearchPathDelta m_spDelta_6;
                    SearchPathDelta m_spDelta_7;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW17;

            // DW18
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_8;
                    SearchPathDelta m_spDelta_9;
                    SearchPathDelta m_spDelta_10;
                    SearchPathDelta m_spDelta_11;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW18;

            // DW19
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_12;
                    SearchPathDelta m_spDelta_13;
                    SearchPathDelta m_spDelta_14;
                    SearchPathDelta m_spDelta_15;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW19;

            // DW20
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_16;
                    SearchPathDelta m_spDelta_17;
                    SearchPathDelta m_spDelta_18;
                    SearchPathDelta m_spDelta_19;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW20;

            // DW21
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_20;
                    SearchPathDelta m_spDelta_21;
                    SearchPathDelta m_spDelta_22;
                    SearchPathDelta m_spDelta_23;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW21;

            // DW22
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_24;
                    SearchPathDelta m_spDelta_25;
                    SearchPathDelta m_spDelta_26;
                    SearchPathDelta m_spDelta_27;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW22;

            // DW23
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_28;
                    SearchPathDelta m_spDelta_29;
                    SearchPathDelta m_spDelta_30;
                    SearchPathDelta m_spDelta_31;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW23;

            // DW24
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_32;
                    SearchPathDelta m_spDelta_33;
                    SearchPathDelta m_spDelta_34;
                    SearchPathDelta m_spDelta_35;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW24;

            // DW25
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_36;
                    SearchPathDelta m_spDelta_37;
                    SearchPathDelta m_spDelta_38;
                    SearchPathDelta m_spDelta_39;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW25;

            // DW26
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_40;
                    SearchPathDelta m_spDelta_41;
                    SearchPathDelta m_spDelta_42;
                    SearchPathDelta m_spDelta_43;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW26;

            // DW27
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_44;
                    SearchPathDelta m_spDelta_45;
                    SearchPathDelta m_spDelta_46;
                    SearchPathDelta m_spDelta_47;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW27;

            // DW28
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_48;
                    SearchPathDelta m_spDelta_49;
                    SearchPathDelta m_spDelta_50;
                    SearchPathDelta m_spDelta_51;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW28;

            // DW29
            union
            {
                struct
                {
                    SearchPathDelta m_spDelta_52;
                    SearchPathDelta m_spDelta_53;
                    SearchPathDelta m_spDelta_54;
                    SearchPathDelta m_spDelta_55;
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW29;

            // DW30
            union
            {
                struct
                {
                    uint32_t m_intra4x4ModeMask : MOS_BITFIELD_RANGE(0, 8);
                uint32_t: MOS_BITFIELD_RANGE(9, 15);
                    uint32_t m_intra8x8ModeMask : MOS_BITFIELD_RANGE(16, 24);
                uint32_t: MOS_BITFIELD_RANGE(25, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW30;

            // DW31
            union
            {
                struct
                {
                    uint32_t m_intra16x16ModeMask : MOS_BITFIELD_RANGE(0, 3);
                    uint32_t m_intraChromaModeMask : MOS_BITFIELD_RANGE(4, 7);
                    uint32_t m_intraComputeType : MOS_BITFIELD_RANGE(8, 9);
                uint32_t: MOS_BITFIELD_RANGE(10, 31);
                };
                struct
                {
                    uint32_t m_value;
                };
            } DW31;
        } SPDelta;

        // DW32
        union
        {
            struct
            {
                uint32_t m_skipVal            : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_multiPredL0Disable : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_multiPredL1Disable : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW32;

        // DW33
        union
        {
            struct
            {
                uint32_t m_intra16x16NonDCPredPenalty : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_intra8x8NonDCPredPenalty   : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_intra4x4NonDCPredPenalty   : MOS_BITFIELD_RANGE(16, 23);
                uint32_t                              : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW33;

        // DW34
        union
        {
            struct
            {
                uint32_t m_list0RefID0FieldParity           : MOS_BITFIELD_BIT(0);
                uint32_t m_list0RefID1FieldParity           : MOS_BITFIELD_BIT(1);
                uint32_t m_list0RefID2FieldParity           : MOS_BITFIELD_BIT(2);
                uint32_t m_list0RefID3FieldParity           : MOS_BITFIELD_BIT(3);
                uint32_t m_list0RefID4FieldParity           : MOS_BITFIELD_BIT(4);
                uint32_t m_list0RefID5FieldParity           : MOS_BITFIELD_BIT(5);
                uint32_t m_list0RefID6FieldParity           : MOS_BITFIELD_BIT(6);
                uint32_t m_list0RefID7FieldParity           : MOS_BITFIELD_BIT(7);
                uint32_t m_list1RefID0FrameFieldFlag        : MOS_BITFIELD_BIT(8);
                uint32_t m_list1RefID1FrameFieldFlag        : MOS_BITFIELD_BIT(9);
                uint32_t m_IntraRefreshEn               : MOS_BITFIELD_RANGE(10, 11);
                uint32_t m_arbitraryNumMbsPerSlice          : MOS_BITFIELD_BIT(12);
                uint32_t m_tqEnable                         : MOS_BITFIELD_BIT(13);
                uint32_t m_forceNonSkipMbEnable             : MOS_BITFIELD_BIT(14);
                uint32_t m_disableEncSkipCheck              : MOS_BITFIELD_BIT(15);
                uint32_t m_enableDirectBiasAdjustment       : MOS_BITFIELD_BIT(16);
                uint32_t m_bForceToSkip                     : MOS_BITFIELD_BIT(17);
                uint32_t m_enableGlobalMotionBiasAdjustment : MOS_BITFIELD_BIT(18);
                uint32_t m_enableAdaptiveTxDecision         : MOS_BITFIELD_BIT(19);
                uint32_t m_enablePerMBStaticCheck           : MOS_BITFIELD_BIT(20);
                uint32_t m_enableAdaptiveSearchWindowSize   : MOS_BITFIELD_BIT(21);
                uint32_t m_removeIntraRefreshOverlap        : MOS_BITFIELD_BIT(22);
                uint32_t m_cqpFlag                          : MOS_BITFIELD_BIT(23);
                uint32_t m_list1RefID0FieldParity           : MOS_BITFIELD_BIT(24);
                uint32_t m_list1RefID1FieldParity           : MOS_BITFIELD_BIT(25);
                uint32_t m_madEnableFlag                    : MOS_BITFIELD_BIT(26);
                uint32_t m_roiEnableFlag                    : MOS_BITFIELD_BIT(27);
                uint32_t m_enableMBFlatnessChkOptimization  : MOS_BITFIELD_BIT(28);
                uint32_t m_bDirectMode                      : MOS_BITFIELD_BIT(29);
                uint32_t m_mbBrcEnable                      : MOS_BITFIELD_BIT(30);
                uint32_t m_bOriginalBff                     : MOS_BITFIELD_BIT(31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW34;

        // DW35
        union
        {
            struct
            {
                uint32_t m_panicModeMBThreshold : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_smallMbSizeInWord    : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_largeMbSizeInWord    : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW35;

        // DW36
        union
        {
            struct
            {
                uint32_t m_numRefIdxL0MinusOne      : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_hmeCombinedExtraSUs      : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_numRefIdxL1MinusOne      : MOS_BITFIELD_RANGE(16, 23);
                uint32_t                            : MOS_BITFIELD_RANGE(24, 26);
                uint32_t m_mbInputEnable            : MOS_BITFIELD_BIT(27);
                uint32_t m_isFwdFrameShortTermRef   : MOS_BITFIELD_BIT(28);
                uint32_t m_checkAllFractionalEnable : MOS_BITFIELD_BIT(29);
                uint32_t m_hmeCombineOverlap        : MOS_BITFIELD_RANGE(30, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW36;

        // DW37
        union
        {
            struct
            {
                uint32_t m_skipModeEn         : MOS_BITFIELD_BIT(0);
                uint32_t m_adaptiveEn         : MOS_BITFIELD_BIT(1);
                uint32_t m_biMixDis           : MOS_BITFIELD_BIT(2);
                uint32_t                      : MOS_BITFIELD_RANGE(3, 4);
                uint32_t m_earlyImeSuccessEn  : MOS_BITFIELD_BIT(5);
                uint32_t                      : MOS_BITFIELD_BIT(6);
                uint32_t m_t8x8FlagForInterEn : MOS_BITFIELD_BIT(7);
                uint32_t                      : MOS_BITFIELD_RANGE(8, 23);
                uint32_t m_earlyImeStop       : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW37;

        // DW38
        union
        {
            struct
            {
                uint32_t m_lenSP        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_maxNumSU     : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_refThreshold : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW38;

        // DW39
        union
        {
            struct
            {
                uint32_t                              : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_hmeRefWindowsCombThreshold : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_refWidth                   : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_refHeight                  : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW39;

        // DW40
        union
        {
            struct
            {
                uint32_t m_distScaleFactorRefID0List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_distScaleFactorRefID1List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW40;

        // DW41
        union
        {
            struct
            {
                uint32_t m_distScaleFactorRefID2List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_distScaleFactorRefID3List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW41;

        // DW42
        union
        {
            struct
            {
                uint32_t m_distScaleFactorRefID4List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_distScaleFactorRefID5List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW42;

        // DW43
        union
        {
            struct
            {
                uint32_t m_distScaleFactorRefID6List0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_distScaleFactorRefID7List0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW43;

        // DW44
        union
        {
            struct
            {
                uint32_t m_actualQPValueForRefID0List0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_actualQPValueForRefID1List0 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_actualQPValueForRefID2List0 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_actualQPValueForRefID3List0 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW44;

        // DW45
        union
        {
            struct
            {
                uint32_t m_actualQPValueForRefID4List0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_actualQPValueForRefID5List0 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_actualQPValueForRefID6List0 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_actualQPValueForRefID7List0 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW45;

        // DW46
        union
        {
            struct
            {
                uint32_t m_actualQPValueForRefID0List1 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_actualQPValueForRefID1List1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_refCost                     : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW46;

        // DW47
        union
        {
            struct
            {
                uint32_t m_mbQpReadFactor : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_intraCostSF    : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_maxVmvR        : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW47;

        //DW48
        union
        {
            struct
            {
                uint32_t m_IntraRefreshMBx            : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_IntraRefreshUnitInMBMinus1 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_IntraRefreshQPDelta        : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW48;

        // DW49
        union
        {
            struct
            {
                uint32_t m_roi1XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi1YTop  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW49;

        // DW50
        union
        {
            struct
            {
                uint32_t m_roi1XRight  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi1YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW50;

        // DW51
        union
        {
            struct
            {
                uint32_t m_roi2XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi2YTop  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW51;

        // DW52
        union
        {
            struct
            {
                uint32_t m_roi2XRight  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi2YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW52;

        // DW53
        union
        {
            struct
            {
                uint32_t m_roi3XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi3YTop  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW53;

        // DW54
        union
        {
            struct
            {
                uint32_t m_roi3XRight  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi3YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW54;

        // DW55
        union
        {
            struct
            {
                uint32_t m_roi4XLeft : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi4YTop  : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW55;

        // DW56
        union
        {
            struct
            {
                uint32_t m_roi4XRight  : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_roi4YBottom : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW56;

        // DW57
        union
        {
            struct
            {
                uint32_t m_roi1dQpPrimeY : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_roi2dQpPrimeY : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_roi3dQpPrimeY : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_roi4dQpPrimeY : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW57;

        // DW58
        union
        {
            struct
            {
                uint32_t m_lambda8x8Inter : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_lambda8x8Intra : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW58;

        // DW59
        union
        {
            struct
            {
                uint32_t m_lambdaInter : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_lambdaIntra : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW59;

        // DW60
        union
        {
            struct
            {
                uint32_t m_mbTextureThreshold : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_txDecisonThreshold : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW60;

        // DW61
        union
        {
            struct
            {
                uint32_t m_hmeMVCostScalingFactor : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_reserved               : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_IntraRefreshMBy    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW61;

        // DW62
        union
        {
            struct
            {
                uint32_t m_IPCMQP0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_IPCMQP1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_IPCMQP2 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_IPCMQP3 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW62;

        // DW63
        union
        {
            struct
            {
                uint32_t m_IPCMQP4     : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_reserved    : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_IPCMThresh0 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW63;

        // DW64
        union
        {
            struct
            {
                uint32_t m_IPCMThresh1 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_IPCMThresh2 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW64;

        // DW65
        union
        {
            struct
            {
                uint32_t m_IPCMThresh3 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_IPCMThresh4 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW65;

        // DW66
        union
        {
            struct
            {
                uint32_t m_mbDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW66;

        // DW67
        union
        {
            struct
            {
                uint32_t m_mvDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW67;

        // DW68
        union
        {
            struct
            {
                uint32_t m_IDistSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW68;

        // DW69
        union
        {
            struct
            {
                uint32_t m_srcYSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW69;

        // DW70
        union
        {
            struct
            {
                uint32_t m_mbSpecificDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW70;

        // DW71
        union
        {
            struct
            {
                uint32_t m_auxVmeOutSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW71;

        // DW72
        union
        {
            struct
            {
                uint32_t m_currRefPicSelSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW72;

        // DW73
        union
        {
            struct
            {
                uint32_t m_hmeMVPredFwdBwdSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW73;

        // DW74
        union
        {
            struct
            {
                uint32_t m_hmeDistSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW74;

        // DW75
        union
        {
            struct
            {
                uint32_t m_sliceMapSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW75;

        // DW76
        union
        {
            struct
            {
                uint32_t m_fwdFrmMBDataSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW76;

        // DW77
        union
        {
            struct
            {
                uint32_t m_fwdFrmMVSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW77;

        // DW78
        union
        {
            struct
            {
                uint32_t m_mbQPBuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW78;

        // DW79
        union
        {
            struct
            {
                uint32_t m_mbBRCLut;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW79;

        // DW80
        union
        {
            struct
            {
                uint32_t m_vmeInterPredictionSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW80;

        // DW81
        union
        {
            struct
            {
                uint32_t m_vmeInterPredictionMRSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW81;

        // DW82
        union
        {
            struct
            {
                uint32_t m_mbStatsSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW82;

        // DW83
        union
        {
            struct
            {
                uint32_t m_madSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW83;

        // DW84
        union
        {
            struct
            {
                uint32_t m_brcCurbeSurfIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW84;

        // DW85
        union
        {
            struct
            {
                uint32_t m_forceNonSkipMBmapSurface;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW85;

        // DW86
        union
        {
            struct
            {
                uint32_t m_reservedIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW86;

        // DW87
        union
        {
            struct
            {
                uint32_t m_staticDetectionCostTableIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW87;

        // DW88
        union
        {
            struct
            {
                uint32_t m_swScoreboardIndex;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW88;
    } m_curbe;
};

class CodechalEncodeAvcEncG12::BrcInitResetCurbe
{
   public:
    BrcInitResetCurbe()
    {
        m_brcInitResetCurbeCmd.m_dw0.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw1.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw2.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw3.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw4.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw5.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw6.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw7.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw8.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw9.m_value                      = 0;
        m_brcInitResetCurbeCmd.m_dw10.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw11.m_avbrConvergence           = 0;
        m_brcInitResetCurbeCmd.m_dw11.m_minQP                     = 1;
        m_brcInitResetCurbeCmd.m_dw12.m_maxQP                     = 51;
        m_brcInitResetCurbeCmd.m_dw12.m_noSlices                  = 0;
        m_brcInitResetCurbeCmd.m_dw13.m_instantRateThreshold0ForP = 40;
        m_brcInitResetCurbeCmd.m_dw13.m_instantRateThreshold1ForP = 60;
        m_brcInitResetCurbeCmd.m_dw13.m_instantRateThreshold2ForP = 80;
        m_brcInitResetCurbeCmd.m_dw13.m_instantRateThreshold3ForP = 120;
        m_brcInitResetCurbeCmd.m_dw14.m_instantRateThreshold0ForB = 35;
        m_brcInitResetCurbeCmd.m_dw14.m_instantRateThreshold1ForB = 60;
        m_brcInitResetCurbeCmd.m_dw14.m_instantRateThreshold2ForB = 80;
        m_brcInitResetCurbeCmd.m_dw14.m_instantRateThreshold3ForB = 120;
        m_brcInitResetCurbeCmd.m_dw15.m_instantRateThreshold0ForI = 40;
        m_brcInitResetCurbeCmd.m_dw15.m_instantRateThreshold1ForI = 60;
        m_brcInitResetCurbeCmd.m_dw15.m_instantRateThreshold2ForI = 90;
        m_brcInitResetCurbeCmd.m_dw15.m_instantRateThreshold3ForI = 115;
        m_brcInitResetCurbeCmd.m_dw16.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw17.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw18.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw19.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw20.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw21.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw22.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw23.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw24.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw25.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw26.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw27.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw28.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw29.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw30.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw31.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw32.m_value                     = 0;
        m_brcInitResetCurbeCmd.m_dw33.m_value                     = 0;
    }

    struct
    {
        union
        {
            struct
            {
                uint32_t m_profileLevelMaxFrame;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw0;

        union
        {
            struct
            {
                uint32_t m_initBufFullInBits;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw1;

        union
        {
            struct
            {
                uint32_t m_bufSizeInBits;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw2;

        union
        {
            struct
            {
                uint32_t m_averageBitRate;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw3;

        union
        {
            struct
            {
                uint32_t m_maxBitRate;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw4;

        union
        {
            struct
            {
                uint32_t m_minBitRate;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw5;

        union
        {
            struct
            {
                uint32_t m_frameRateM;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw6;

        union
        {
            struct
            {
                uint32_t m_frameRateD;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw7;

        union
        {
            struct
            {
                uint32_t m_brcFlag : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_gopP    : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw8;

        union
        {
            struct
            {
                uint32_t m_gopB              : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_frameWidthInBytes : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw9;

        union
        {
            struct
            {
                uint32_t m_frameHeightInBytes : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_avbrAccuracy       : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw10;

        union
        {
            struct
            {
                uint32_t m_avbrConvergence : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_minQP           : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw11;

        union
        {
            struct
            {
                uint32_t m_maxQP    : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_noSlices : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw12;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForP : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_instantRateThreshold1ForP : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_instantRateThreshold2ForP : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_instantRateThreshold3ForP : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw13;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForB : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_instantRateThreshold1ForB : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_instantRateThreshold2ForB : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_instantRateThreshold3ForB : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw14;

        union
        {
            struct
            {
                uint32_t m_instantRateThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_instantRateThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_instantRateThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_instantRateThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw15;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForPandB : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold1ForPandB : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold2ForPandB : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold3ForPandB : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw16;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForPandB : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold5ForPandB : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold6ForPandB : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold7ForPandB : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw17;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForVBR : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold1ForVBR : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold2ForVBR : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold3ForVBR : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw18;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForVBR : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold5ForVBR : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold6ForVBR : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold7ForVBR : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw19;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold0ForI : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold1ForI : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold2ForI : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold3ForI : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw20;

        union
        {
            struct
            {
                uint32_t m_deviationThreshold4ForI : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_deviationThreshold5ForI : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_deviationThreshold6ForI : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_deviationThreshold7ForI : MOS_BITFIELD_RANGE(24, 31);  //<! Signed byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw21;

        union
        {
            struct
            {
                uint32_t m_initialQPForI     : MOS_BITFIELD_RANGE(0, 7);    //<! Signed byte
                uint32_t m_initialQPForP     : MOS_BITFIELD_RANGE(8, 15);   //<! Signed byte
                uint32_t m_initialQPForB     : MOS_BITFIELD_RANGE(16, 23);  //<! Signed byte
                uint32_t m_slidingWindowSize : MOS_BITFIELD_RANGE(24, 31);  //<! unsigned byte
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw22;

        union
        {
            struct
            {
                uint32_t m_aCQP;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw23;

        union
        {
            struct
            {
                uint32_t m_longTermInterval : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_reserved         : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw24;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw25;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw26;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t   m_value;
            };
        } m_dw27;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw28;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw29;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw30;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw31;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexHistorybuffer;
            };
            struct
            {
                uint32_t m_value;
            };
    } m_dw32;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexDistortionbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw33;
    } m_brcInitResetCurbeCmd;
};

class CodechalEncodeAvcEncG12::MbBrcUpdateCurbe
{
   public:
    MbBrcUpdateCurbe()
    {
        memset((void *)&m_mbBrcUpdateCurbeCmd, 0, sizeof(MbBrcUpdateCurbe));
    }

    struct
    {
        union
        {
            struct
            {
                uint32_t m_currFrameType : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_enableROI : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_roiRatio : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_reserved : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } DW0;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW1;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW2;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW3;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW4;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } DW5;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
    } DW6;

        union
        {
            struct
            {
                uint32_t   m_reserved;;
            };
            struct
            {
                uint32_t   m_value;
            };
        } DW7;
        
        union
        {
            struct
            {
                uint32_t   m_historyBufferIndex;
            };
            struct
            {
                uint32_t   m_value;
            };
        } DW8;
        
        union
        {
            struct
            {
                uint32_t    m_mbqpBufferIndex;
            };
            struct
            {
                uint32_t    m_value;
            };
        } DW9;
        
        union
        {
            struct
            {
                uint32_t    m_roiBufferIndex;
            };
            struct
            {
                uint32_t    m_value;
            };
        } DW10;
        
        union
        {
            struct
            {
                uint32_t    m_mbStatisticalBufferIndex;
            };
            struct
            {
                uint32_t    m_value;
            };
        } DW11;

    } m_mbBrcUpdateCurbeCmd;
};

class CodechalEncodeAvcEncG12::FrameBrcUpdateCurbe
{
public:

    struct
    {
        union
        {
            struct
            {
                uint32_t m_targetSize;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw0;

        union
        {
            struct
            {
                uint32_t m_frameNumber;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw1;

        union
        {
            struct
            {
                uint32_t m_sizeofPicHeaders;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw2;

        union
        {
            struct
            {
                uint32_t m_startGAdjFrame0 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_startGAdjFrame1 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw3;

        union
        {
            struct
            {
                uint32_t m_startGAdjFrame2 : MOS_BITFIELD_RANGE(0, 15);
                uint32_t m_startGAdjFrame3 : MOS_BITFIELD_RANGE(16, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw4;

        union
        {
            struct
            {
                uint32_t m_targetSizeFlag : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_brcFlag        : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_maxNumPAKs     : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_currFrameType  : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw5;

        union
        {
            struct
            {
                uint32_t m_numSkipFrames        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_minimumQP            : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_maximumQP            : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_enableForceToSkip    : MOS_BITFIELD_BIT(24);
                uint32_t m_enableSlidingWindow  : MOS_BITFIELD_BIT(25);
                uint32_t m_enableExtremLowDelay : MOS_BITFIELD_BIT(26);
                uint32_t m_disableVarCompute    : MOS_BITFIELD_BIT(27 );
                uint32_t m_reserved             : MOS_BITFIELD_RANGE(28, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw6;

        union
        {
            struct
            {
                uint32_t m_sizeSkipFrames;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw7;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustMult0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_startGlobalAdjustMult1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_startGlobalAdjustMult2 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_startGlobalAdjustMult3 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw8;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustMult4 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_startGlobalAdjustDiv0  : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_startGlobalAdjustDiv1  : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_startGlobalAdjustDiv2  : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw9;

        union
        {
            struct
            {
                uint32_t m_startGlobalAdjustDiv3 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_startGlobalAdjustDiv4 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_qpThreshold0          : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_qpThreshold1          : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw10;

        union
        {
            struct
            {
                uint32_t m_qpThreshold2         : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_qpThreshold3         : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_gRateRatioThreshold0 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_gRateRatioThreshold1 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw11;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThreshold2 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_gRateRatioThreshold3 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_gRateRatioThreshold4 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_gRateRatioThreshold5 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw12;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThresholdQP0 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_gRateRatioThresholdQP1 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_gRateRatioThresholdQP2 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_gRateRatioThresholdQP3 : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw13;

        union
        {
            struct
            {
                uint32_t m_gRateRatioThresholdQP4 : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_gRateRatioThresholdQP5 : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_gRateRatioThresholdQP6 : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_qpIndexOfCurPic        : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw14;

        union
        {
            struct
            {
                uint32_t m_reserved        : MOS_BITFIELD_RANGE(0, 7);
                uint32_t m_enableROI       : MOS_BITFIELD_RANGE(8, 15);
                uint32_t m_roundingIntra   : MOS_BITFIELD_RANGE(16, 23);
                uint32_t m_roundingInter   : MOS_BITFIELD_RANGE(24, 31);
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw15;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw16;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw17;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw18;

        union
        {
            struct
            {
                uint32_t m_userMaxFrame;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw19;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw20;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw21;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw22;

        union
        {
            struct
            {
                uint32_t m_reserved;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw23;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexBRCHistorybuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw24;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexPreciousPAKStatisticsOutputbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw25;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexAVCIMGStateInputbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw26;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexAVCIMGStateOutputbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw27;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexAVC_Encbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw28;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexAVCDistortionbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw29;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexBRCConstdatabuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw30;

        union
        {
            struct
            {
                uint32_t m_surfaceIndexMBStatsbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw31;
        
        union
        {
            struct
            {
                uint32_t m_surfaceIndexMotionVectorbuffer;
            };
            struct
            {
                uint32_t m_value;
            };
        } m_dw32;
    } m_frameBrcUpdateCurbeCmd;

    FrameBrcUpdateCurbe()
    {
        m_frameBrcUpdateCurbeCmd.m_dw0.m_value           = 0;
        m_frameBrcUpdateCurbeCmd.m_dw1.m_value           = 0;
        m_frameBrcUpdateCurbeCmd.m_dw2.m_value           = 0;
        m_frameBrcUpdateCurbeCmd.m_dw3.m_startGAdjFrame0 = 10;
        m_frameBrcUpdateCurbeCmd.m_dw3.m_startGAdjFrame1 = 50;
        m_frameBrcUpdateCurbeCmd.m_dw4.m_startGAdjFrame2 = 100;
        m_frameBrcUpdateCurbeCmd.m_dw4.m_startGAdjFrame3 = 150;
        m_frameBrcUpdateCurbeCmd.m_dw5.m_value           = 0;
        m_frameBrcUpdateCurbeCmd.m_dw6.m_value           = 0;
        m_frameBrcUpdateCurbeCmd.m_dw7.m_value           = 0;

        m_frameBrcUpdateCurbeCmd.m_dw8.m_startGlobalAdjustMult0 = 1;
        m_frameBrcUpdateCurbeCmd.m_dw8.m_startGlobalAdjustMult1 = 1;
        m_frameBrcUpdateCurbeCmd.m_dw8.m_startGlobalAdjustMult2 = 3;
        m_frameBrcUpdateCurbeCmd.m_dw8.m_startGlobalAdjustMult3 = 2;

        m_frameBrcUpdateCurbeCmd.m_dw9.m_startGlobalAdjustMult4 = 1;
        m_frameBrcUpdateCurbeCmd.m_dw9.m_startGlobalAdjustDiv0  = 40;
        m_frameBrcUpdateCurbeCmd.m_dw9.m_startGlobalAdjustDiv1  = 5;
        m_frameBrcUpdateCurbeCmd.m_dw9.m_startGlobalAdjustDiv2  = 5;

        m_frameBrcUpdateCurbeCmd.m_dw10.m_startGlobalAdjustDiv3 = 3;
        m_frameBrcUpdateCurbeCmd.m_dw10.m_startGlobalAdjustDiv4 = 1;
        m_frameBrcUpdateCurbeCmd.m_dw10.m_qpThreshold0          = 7;
        m_frameBrcUpdateCurbeCmd.m_dw10.m_qpThreshold1          = 18;

        m_frameBrcUpdateCurbeCmd.m_dw11.m_qpThreshold2         = 25;
        m_frameBrcUpdateCurbeCmd.m_dw11.m_qpThreshold3         = 37;
        m_frameBrcUpdateCurbeCmd.m_dw11.m_gRateRatioThreshold0 = 40;
        m_frameBrcUpdateCurbeCmd.m_dw11.m_gRateRatioThreshold1 = 75;

        m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold2 = 97;
        m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold3 = 103;
        m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold4 = 125;
        m_frameBrcUpdateCurbeCmd.m_dw12.m_gRateRatioThreshold5 = 160;

        m_frameBrcUpdateCurbeCmd.m_dw13.m_gRateRatioThresholdQP0 = MOS_BITFIELD_VALUE((uint32_t)-3, 8);
        m_frameBrcUpdateCurbeCmd.m_dw13.m_gRateRatioThresholdQP1 = MOS_BITFIELD_VALUE((uint32_t)-2, 8);
        m_frameBrcUpdateCurbeCmd.m_dw13.m_gRateRatioThresholdQP2 = MOS_BITFIELD_VALUE((uint32_t)-1, 8);
        m_frameBrcUpdateCurbeCmd.m_dw13.m_gRateRatioThresholdQP3 = 0;

        m_frameBrcUpdateCurbeCmd.m_dw14.m_gRateRatioThresholdQP4 = 1;
        m_frameBrcUpdateCurbeCmd.m_dw14.m_gRateRatioThresholdQP5 = 2;
        m_frameBrcUpdateCurbeCmd.m_dw14.m_gRateRatioThresholdQP6 = 3;
        m_frameBrcUpdateCurbeCmd.m_dw14.m_qpIndexOfCurPic        = 0xff;

        m_frameBrcUpdateCurbeCmd.m_dw15.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw16.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw17.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw18.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw19.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw20.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw21.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw22.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw23.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw24.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw25.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw26.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw27.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw28.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw29.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw30.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw31.m_value = 0;
        m_frameBrcUpdateCurbeCmd.m_dw32.m_value = 0;
    };
};

#endif  //__CODECHAL_ENCODE_AVC_G12_H__
