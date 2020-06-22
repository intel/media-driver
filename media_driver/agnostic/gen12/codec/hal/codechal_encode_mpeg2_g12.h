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
//! \file     codechal_encode_mpeg2_g12.h
//! \brief    MPEG2 dual-pipe encoder for GEN12 platform.
//!

#ifndef __CODECHAL_ENCODE_MPEG2_G12_H__
#define __CODECHAL_ENCODE_MPEG2_G12_H__

#include "codechal_encode_mpeg2.h"
#include "codechal_encode_sw_scoreboard_g12.h"
#include "codechal_encode_singlepipe_virtualengine.h"

//!
//! \class   CodechalEncodeMpeg2G12
//! \brief   MPEG2 dual-pipe encoder base class for GEN12
//! \details This class defines the member fields, functions for GEN12 platform
//!
class CodechalEncodeMpeg2G12 : public CodechalEncodeMpeg2
{
public:
    PCODECHAL_ENCODE_SINGLEPIPE_VIRTUALENGINE_STATE m_sinlgePipeVeState;  //!< single pipe virtual engine state
    //!
    //! \brief    Constructor
    //!
    CodechalEncodeMpeg2G12(
        CodechalHwInterface* hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    //!
    //! \brief    Copy constructor
    //!
    CodechalEncodeMpeg2G12(const CodechalEncodeMpeg2G12&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    CodechalEncodeMpeg2G12& operator=(const CodechalEncodeMpeg2G12&) = delete;

    //!
    //! \brief    Destructor
    //!
    ~CodechalEncodeMpeg2G12();

    //!
    //! \brief    Resize buffers due to resoluton change.
    //! \details  Resize buffers due to resoluton change.
    //!
    //! \return   void
    //!
    virtual void ResizeOnResChange() override;

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

    MOS_STATUS AddMediaVfeCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        SendKernelCmdsParams *params) override;

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

    MOS_STATUS SubmitCommandBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer,
        int32_t  nullRendering) override;

protected:
    MOS_STATUS EncodeMeKernel() override;

    MOS_STATUS Initialize(CodechalSetting * codecHalSettings) override;

    MOS_STATUS InitKernelState() override;

    MOS_STATUS InitKernelStateMbEnc();

    //!
    //! \brief    Get maximum BT count
    //!
    //! \return   uint32_t
    //!           Maximum BT count
    //!
    virtual uint32_t GetMaxBtCount() override;

    //!
    //! \brief    Encode kernel functions
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ExecuteKernelFunctions() override;

    MOS_STATUS SetCurbeMbEnc(
        bool mbEncIFrameDistEnabled,
        bool mbQpDataEnabled) override;

    MOS_STATUS SendMbEncSurfaces(
        PMOS_COMMAND_BUFFER  cmdBuffer,
        bool mbEncIFrameDistEnabled) override;

    MOS_STATUS SendPrologWithFrameTracking(
        PMOS_COMMAND_BUFFER         cmdBuffer,
        bool                        frameTracking,
        MHW_MI_MMIOREGISTERS       *mmioRegister = nullptr) override;

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
    static MOS_STATUS GetKernelHeaderAndSize(
        void                           *binary,
        EncOperation                   operation,
        uint32_t                       krnStateIdx,
        void                           *krnHeader,
        uint32_t                       *krnSize);

    MOS_STATUS InitMmcState() override;

    CodechalEncodeSwScoreboardG12 *m_swScoreboardState = nullptr;    //!< pointer to SW scoreboard ini state.

    MOS_STATUS UpdateCmdBufAttribute(
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                renderEngineInUse) override;
};

#endif  // __CODECHAL_ENCODE_MPEG2_G12_H__
