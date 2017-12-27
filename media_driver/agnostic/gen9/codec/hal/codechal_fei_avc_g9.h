/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     codechal_fei_avc_g9.h
//! \brief    This file defines the C++ class/interface for Gen9 platform's AVC
//!           FEI encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_FEI_AVC_G9_H__
#define __CODECHAL_FEI_AVC_G9_H__

#include "codechal_encode_avc_g9.h"

enum MbEncBufIndex
{
    mbEncMbCodeBuffer           = 0,
    mbEncMvDataBuffer           = 1,
    mbEncFwdMbCodeBuffer        = 2,
    mbEncFwdMvDataBuffer        = 3,
    mbEncMADDataBuffer          = 4,
    mbEncMBSpecficDataBuffer    = 5,
    mbEncMVPredictorBuffer      = 6,
    mbEncAuxVmeOutBuffer        = 7,
    mbEncMBBRCLutBuffer         = 8,
    mbEncMBQPBuffer             = 9,
    mbEncBRCCurbeBuffer         = 10,
};

enum MbEncSurfIndex
{
    mbEncSrcYSurf                       = 0,
    mbEncHMEMVPredFwdBwdSurf            = 1,
    mbEncHMEDistSurf                    = 2,
    mbEncVMEInterPredictionSurf         = 3,
    mbEncVMEInterPredictionMRSurf       = 9,
    mbEncMBDistSurf                     = 11,
    mbEncCurrRefPicSelSurf              = 12,
    mbEncMBstatsSurf                    = 13,
    mbEncSliceMapSurf                   = 14,
};

struct CodechalEncodeAvcSurfaceIdx{
    union{
        struct{
            SurfaceIndex * pCmSurfIdx[28];
        };
        struct{
            SurfaceIndex * MBDataSurfIndex;
            SurfaceIndex * MVDataSurfIndex;
            SurfaceIndex * MBDistIndex;
            SurfaceIndex * SrcYSurfIndex;
            SurfaceIndex * MBSpecficDataSurfIndex;
            SurfaceIndex * AuxVmeOutSurfIndex;
            SurfaceIndex * CurrRefPicSelSurfIndex;
            SurfaceIndex * HMEMVPredFwdBwdSurfIndex;
            SurfaceIndex * HMEDistSurfIndex;
            SurfaceIndex * SliceMapSurfIndex;
            SurfaceIndex * FwdFrmMBDataSurfIndex;
            SurfaceIndex * FwdFrmMVSurfIndex;
            SurfaceIndex * MBQPBuffer;
            SurfaceIndex * MBBRCLut;
            SurfaceIndex * MBVMEInterPredictionSurfIndex;
            SurfaceIndex * MBVMEInterPredictionMRSurfIndex;
            SurfaceIndex * MBstats;
            SurfaceIndex * MADSurfIndex;
            SurfaceIndex * ForceNonSkipMBMap;
            SurfaceIndex * ReservedIndex;
            SurfaceIndex * BRCCurbeSurfIndex;
            SurfaceIndex * StaticDetectionCostTableIndex;
            SurfaceIndex * FEI_MVPredSurfIndex;
            SurfaceIndex * L1RefPicSelSurfIndex;
            SurfaceIndex * Flatnesschk;
            SurfaceIndex * InterDistortionIndex;
            SurfaceIndex * BestInterIntraSurfIndex;
            SurfaceIndex * Reserved0;
        };
    };
};

class CodechalEncodeAvcEncFeiG9 : public CodechalEncodeAvcEncG9
{
public:
    static const uint32_t ModeMvCost_Cm_PreProc[3][CODEC_AVC_NUM_QP][8];
    static const uint32_t ME_CURBE_CM_FEI[39];
    static const uint32_t PreProc_CURBE_CM_normal_I_frame[49];
    static const uint32_t PreProc_CURBE_CM_normal_I_field[49];
    static const uint32_t PreProc_CURBE_CM_normal_P_frame[49];
    static const uint32_t PreProc_CURBE_CM_normal_P_field[49];
    static const uint32_t PreProc_CURBE_CM_normal_B_frame[49];
    static const uint32_t PreProc_CURBE_CM_normal_B_field[49];
    static const uint16_t RefCost_MultiRefQp_Fei[NUM_PIC_TYPES][64];
    static const uint32_t FEI_MBEnc_CURBE_normal_I_frame[104];
    static const uint32_t FEI_MBEnc_CURBE_normal_I_field[104];
    static const uint32_t FEI_MBEnc_CURBE_normal_P_frame[104];
    static const uint32_t FEI_MBEnc_CURBE_normal_P_field[104];
    static const uint32_t FEI_MBEnc_CURBE_normal_B_frame[104];
    static const uint32_t FEI_MBEnc_CURBE_normal_B_field[104];
    static const uint32_t FEI_MBEnc_CURBE_I_frame_DIST[104];
    static const uint32_t HMEBCombineLen_fei[NUM_TARGET_USAGE_MODES];
    static const uint32_t HMECombineLen_fei[NUM_TARGET_USAGE_MODES];
    static const uint32_t m_brcConstantSurfaceHeightFei = 44;
    static const uint32_t m_refThresholdFei = 400;
    static const uint32_t m_mbencNumTargetUsagesCmFei = 1;
    static const uint32_t m_meCurbeDataSizeFei = 128;
    static const uint32_t m_mbencCurbeDataSizeFei = 320;
    static const uint32_t m_preProcCurbeDataSizeFei = 160;

#ifdef FEI_ENABLE_CMRT
    static const uint32_t m_mdfDsBufSize = 2;           //ds use 2 buffer & 4 surface for each channel,  6 buffer and 12 totally
    static const uint32_t m_mdfDsSurfSize = 4;
    static const uint32_t m_mdfDsVmeSurfSize = 0;

    static const uint32_t m_mdfMeBufSize = 0;           //me use 0 buffer and 12 surface
    static const uint32_t m_mdfMeSurfSize = 12;
    static const uint32_t m_mdfMeVmeSurfSize = 2;

    static const uint32_t m_mdfPreProcBufSize = 6;           //preproc use 5 buffers and  4 surface
    static const uint32_t m_mdfPreProcSurfSize  = 4;
    static const uint32_t m_mdfPreProcVmeSurfSize = 2;
    CodechalEncodeMdfKernelResource m_resPreProcKernel;
    CodechalEncodeMdfKernelResource m_resMeKernel;
    uint32_t                            m_dsIdx;
#endif

    static const uint32_t m_mdfMbencBufSize = 12;           //MBEnc is not in a context with preenc
    static const uint32_t m_mdfMbencSurfSize = 16;
    static const uint32_t m_mdfMbencVmeSurfSize = 2;
    static const uint32_t m_vmeSurfacePerStreamSize = 2;
    static const uint32_t m_commonSurfacePerStreamSize = 24;
    static const uint32_t m_vmeSurfaceSize = m_vmeSurfacePerStreamSize * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9;
    static const uint32_t m_commonSurfaceSize = m_commonSurfacePerStreamSize * CODECHAL_ENCODE_AVC_MFE_MAX_FRAMES_G9;

    struct CodechalEncodeAvcSurfaceIdx *m_cmSurfIdx;
    CodechalEncodeMdfKernelResource    *m_resMbencKernel;
    CodechalEncodeMdfKernelResource    *m_origResMbencKernel;

    SurfaceIndex *m_vmeSurface;
    SurfaceIndex *m_commonSurface;


    CodechalEncodeAvcEncFeiG9(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    ~CodechalEncodeAvcEncFeiG9();

    //!
    //! \brief    Initializes the kernel.
    //! \details
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS InitializePicture(const EncoderParams& params);

    //!
    //! \brief    Initializes the FEI PreEnc kernel.
    //! \details  If PreEnc mode, initial PreEnc kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS EncodePreEncInitialize(const EncoderParams& params);

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExecuteKernelFunctions();

    //!
    //! \brief    Init ME kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMe();
#ifdef FEI_ENABLE_CMRT
        // GenericEncodeMeKernel functions
        //!
        //! \brief    Run Encode ME kernel
        //!
        //! \param [in] brcBuffers
        //!           Pointer to the brc buffer
        //! \param   [in] hmeLevel
        //!           Hme level
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        virtual MOS_STATUS GenericEncodeMeKernel(EncodeBrcBuffers* brcBuffers, HmeLevel hmeLevel);

        //!
        //! \brief    Dispatch MDF Encode ME kernel
        //!
        //! \param [in] surfIndex
        //!            Pointer to the SurfaceIndex * list which is used to Surfaces
        //! \param [in] width
        //!            input picture width
        //!\param  [in]  height
        //!            input picture height
        //!\param  [in]   isBFrame
        //!           if input picture is B frame
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!

        MOS_STATUS DispatchKernelMe(
            SurfaceIndex** surfIndex,
            uint16_t       width,
            uint16_t       height,
            bool           isBFrame);

        //!
        //! \brief    Dispatch MDF FEI preproc
        //!
        //! \param  [in] surfIndex
        //!           Pointer to the SurfaceIndex * list which is used to Surfaces
        //! \param  [in] width
        //!           input picture width
        //!\param   [in] height
        //!           input picture height
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!

        MOS_STATUS DispatchKernelPreProc(
            SurfaceIndex**  surfIndex,
            uint16_t        width,
            uint16_t        height);

        //!
        //! \brief    Init Scaling kernel state
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!

        static MOS_STATUS InitKernelStateScaling(PCODECHAL_ENCODER avcEncoder);

        //!
        //! \brief    Dispatch MDF FEI 4x DownScalling
        //! \param  [in]  flatnessThreshold
        //!           flatness threshold
        //! \param [in]  option
        //!           down scaling option
        //!             bit0 enable/disable flatness check
        //!             bit1 enable/disable variance output
        //!             bit2 enable/disable average output
        //!             bit3 eanble/disable 8x8 statistics output
        //! \param  [in]  sourceWidth
        //!           input picture width
        //!\param   [in]  sourceHeight
        //!           input picture height
        //!\param   [in] kernelType
        //!           if input picture frame (0) or field (1)
        //!\param   [in]  surfIndex
        //!           Pointer to the SurfaceIndex * list which is used to Surfaces
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        MOS_STATUS DispatchKernelScaling(
            uint32_t flatnessThreshold,
            uint32_t options,
            uint16_t sourceWidth,
            uint16_t sourceHeight,
            uint32_t kernelType,
            SurfaceIndex** surfIdx);

        //!
        //! \brief    MBEnc Encode kernel functions
        //!
        //! \param [in]  params
        //!           downscalling parameters
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!

        MOS_STATUS EncodeScalingKernel(CodechalEncodeCscDs::KernelParams* params);

#endif

        MOS_STATUS InitMfe();

        MOS_STATUS InitKernelStateMfeMbEnc();

        //!
        //! \brief    Send AVC Mfe MbEnc kernel Curbe data.
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS SendCurbeAvcMfeMbEnc();

        //!
        //! \brief    Dispatch MDF Encode MBEnc kernel
        //!
        //! \param  [in]  params
        //!           Dispatch Parameters
        //! \return   MOS_STATUS
        //!           MOSs_STATUS_SUCCESS if success
        //!
        MOS_STATUS DispatchKernelMbEnc(
            void      *params);

        //!
        //! \brief    MBEnc Encode kernel functions
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS EncodeMbEncKernelFunctions();

        //!
        //! \brief    Set AVC MbEnc kernel Surface data.
        //!
        //! \param    [in] cmdBuffer
        //!           Pointer to the MOS_COMMAND_BUFFER
        //! \param    [in] params
        //!           Pointer to the CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SendAvcMfeMbEncSurfaces(
            PMOS_COMMAND_BUFFER cmdBuffer,
            PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params);

        //!
        //! \brief    Set AVC ME kernel curbe
        //!
        //! \param    [in] params
        //!           Pointer to the MeCurbeParams
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetCurbeMe (
            MeCurbeParams* params);

    //!
    //! \brief    Send surface cmd to AVC ME kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMeSurfaces (
        PMOS_COMMAND_BUFFER cmdBuffer,
        MeSurfaceParams* params);

    //!
    //! \brief    Get encoder kernel header and kernel size
    //!
    //! \param    [in] pvBinary
    //!           Pointer to kernel binary
    //! \param    [in] operation
    //!           Enc kernel operation
    //! \param    [in] krnStateIdx
    //!           Kernel state index
    //! \param    [out] pvKrnHeader
    //!           Pointer to kernel header
    //! \param    [out] pdwKrnSize
    //!           Pointer to kernel size
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS EncodeGetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    //!
    //! \brief    PreEnc Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EncodePreEncKernelFunctions();

    // state related functions
    //!
    //! \brief    Initialize related states
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitializeState();

    //!
    //! \brief    Validate reference list L0 and L1.
    //!
    //! \param    [in] params
    //!           pointer to CODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ValidateNumReferences(
        PCODECHAL_ENCODE_AVC_VALIDATE_NUM_REFS_PARAMS params);

    //!
    //! \brief    Init MbEnc kernel State.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Initialize PreProc kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStatePreProc();

    //!
    //! \brief    Initialize WP kernel state
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitKernelStateWP();

    //!
    //! \brief    Get MbEnc kernel state idx
    //!
    //! \param    [in] params
    //!           Pointer to the CodechalEncodeIdOffsetParams
    //! \param    [in] kernelOffset
    //!           kernel offset
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetMbEncKernelStateIdx(
        CodechalEncodeIdOffsetParams       *params,
		uint32_t                           *kernelOffset);

    //!
    //! \brief    Set AVC MbEnc kernel Curbe data.
    //!
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcMbEnc(
        PCODECHAL_ENCODE_AVC_MBENC_CURBE_PARAMS params);

    //!
    //! \brief    Set AVC PreProc kernel Curbe data.
    //!
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetCurbeAvcPreProc(
        PCODECHAL_ENCODE_AVC_PREPROC_CURBE_PARAMS params);

    //!
    //! \brief    Set AVC MbEnc kernel Surface data.
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the MOS_COMMAND_BUFFER
    //! \param    [in] params
    //!           Pointer to the CODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcMbEncSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_MBENC_SURFACE_PARAMS params);

    //!
    //! \brief    Set AVC PreProc kernel Surface data.
    //!
    //! \param    [in] cmdBuffer
    //!           Pointer to the MOS_COMMAND_BUFFER
    //! \param    [in]  params
    //!           Pointer to the CODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendAvcPreProcSurfaces(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PCODECHAL_ENCODE_AVC_PREPROC_SURFACE_PARAMS params);

    //!
    //! \brief    Invoke FEI PreProc kernel.
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS PreProcKernel();
};

#endif  // __CODECHAL_ENC_AVC_FEI_G9_H__
