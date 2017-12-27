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
//! \file     codechal_fei_avc_g8.h
//! \brief    This file defines the C++ class/interface for Gen8 platform's AVC
//!           FEI encoding to be used across CODECHAL components.
//!

#ifndef __CODECHAL_FEI_AVC_G8_H__
#define __CODECHAL_FEI_AVC_G8_H__

#include "codechal_encode_avc_g8.h"

class CodechalEncodeAvcEncFeiG8 : public CodechalEncodeAvcEncG8
{
public:
    static const uint32_t m_feiMBEncCurbeSizeInDword = 95;

    static const uint32_t m_modeMvCost_Cm_PreProc[3][CODEC_AVC_NUM_QP][8];
    static const uint32_t m_meCurbeCmFei[39];
    static const uint32_t m_preProcCurbeCmNormalIFrame[49];
    static const uint32_t m_preProcCurbeCmNormalIfield[49];
    static const uint32_t m_preProcCurbeCmNormalPFrame[49];
    static const uint32_t m_preProcCurbeCmNormalPField[49];
    static const uint32_t m_preProcCurbeCmNormalBFrame[49];
    static const uint32_t m_preProcCurbeCmNormalBField[49];
    static const uint32_t m_feiMbEncCurbeNormalIFrame[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeNormalIField[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeNormalPFrame[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeNormalPfield[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeNormalBFrame[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeNormalBField[m_feiMBEncCurbeSizeInDword];
    static const uint32_t m_feiMbEncCurbeIFrameDist[m_feiMBEncCurbeSizeInDword];

    static const uint32_t m_mbencNumTargetUsagesCmFei = 1;
    static const uint32_t m_feiMeCurbeDataSize = 128;
    static const uint32_t m_feiMBEncCurbeDataSizeExcludeSurfaceIdx = 236;
    static const uint32_t m_feiPreProcCurbeDataSize = 160;

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

    static const uint32_t m_mdfMbencBufSize = 12;           //MBEnc is not in a context with preenc
    static const uint32_t m_mdfMbencSurfSize = 16;
    static const uint32_t m_mdfMbencVmeSurfSize = 2;

    CodechalEncodeMdfKernelResource m_resMBEncKernel;
    CodechalEncodeMdfKernelResource m_resPreProcKernel;
    CodechalEncodeMdfKernelResource m_resMEKernel;
    void                               *m_avcCmSurfIdx;
    uint32_t                            m_dsKernelIdx;
#endif
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeAvcEncFeiG8(
        CodechalHwInterface *   hwInterface,
        CodechalDebugInterface *debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);


    ~CodechalEncodeAvcEncFeiG8();

    void UpdateSSDSliceCount();

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
    MOS_STATUS InitKernelStateMe();
#ifdef FEI_ENABLE_CMRT
        // EncodeMeKernel functions
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
        virtual MOS_STATUS EncodeMeKernel(
            PCODECHAL_ENCODE_BRC_BUFFERS brcBuffers,
            CODECHAL_ENCODE_HME_LEVEL hmeLevel);

        //!
        //! \brief    Dispatch MDF Encode ME kernel
        //!
        //! \param [in] surfIdxArray
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
            SurfaceIndex** surfIdxArray,
            uint16_t width,
            uint16_t height,
            bool isBFrame);

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
        //! \brief    Dispatch MDF FEI preproc
        //!
        //! \param  [in] surfIndexArray
        //!           Pointer to the SurfaceIndex * list which is used to Surfaces
        //! \param  [in] width
        //!           input picture width
        //!\param   [in] height
        //!           input picture height
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!

        MOS_STATUS DispatchKernelPreProc(
            SurfaceIndex**  surfIndexArray,
            uint16_t            width,
            uint16_t            height);

        //!
        //! \brief    MBEnc Encode kernel functions
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS EncodeMbEncKernelFunctions();

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
        //! \param [in]  options
        //!           down scaling option
        //!             bit0 enable/disable flatness check
        //!             bit1 enable/disable variance output
        //!             bit2 enable/disable average output
        //!             bit3 eanble/disable 8x8 statistics output
        //! \param  [in]  sourceWidth
        //!           input picture width
        //!\param   [in]  sourceHeight
        //!           input sourceHeight
        //!\param   [in] kernelType
        //!           if input picture frame (0) or field (1)
        //!\param   [in]  surfIdxArray
        //!           Pointer to the SurfaceIndex * Array which is used to Surfaces
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success
        //!
        MOS_STATUS DispatchKernelScaling(
            uint32_t flatnessThreshold,
            uint32_t options,
            uint16_t sourceWidth, 
            uint16_t sourceHeight, 
            uint32_t kernelType,
            SurfaceIndex** surfIdxArray);

        //!
        //! \brief    MBEnc Encode kernel functions
        //!
        //! \param [in]  params
        //!           downscalling parameters
        //!
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!

        MOS_STATUS EncodeScalingKernel(PCODECHAL_ENCODE_CSC_SCALING_KERNEL_PARAMS params);

#endif
        //!
        //! \brief    Set AVC ME kernel curbe
        //!
        //! \param    [in] params
        //!           Pointer to the CODECHAL_ME_CURBE_PARAMS
        //! \return   MOS_STATUS
        //!           MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS SetCurbeMe (MeCurbeParams* params);

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

using PCodechalEncodeAvcEncFeiG8 = CodechalEncodeAvcEncFeiG8*;
#endif  // __CODECHAL_FEI_AVC_G8_H__
