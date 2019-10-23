/*
* Copyright (c) 2013-2019, Intel Corporation
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
//! \file      codechal_secure_decode_interface.h 
//! \brief     Stub file for CodecHal Secure Decode 
//!
#ifndef __CODECHAL_SECURE_DECODE_INTERFACE_H__
#define __CODECHAL_SECURE_DECODE_INTERFACE_H__

#include "codechal_hw.h"

//!
//! \class  CodechalSecureDecode
//! \brief  Codechal secure decode
//!
class CodechalSecureDecodeInterface
{
public:

    //!
    //! \brief  Allocate Resource
    //!
    virtual MOS_STATUS AllocateResource(void *state) = 0;

    //!
    //! \brief  Execute Huc StreamOut
    //!
    virtual MOS_STATUS Execute(void *state) = 0;

    //!
    //! \brief  Set Bitstream Buffer
    //!
    virtual MOS_STATUS SetBitstreamBuffer(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS *indObjBaseAddrParams) = 0;

    //!
    //! \brief  Set HEVC Huc Dmem S2L Bss
    //!
    virtual MOS_STATUS SetHevcHucDmemS2LBss(
        void          *state,
        void          *hucPicBss,
        void          *hucSliceBss) = 0;

    //!
    //! \brief  Add Hevc Huc SecureState
    //!
    virtual MOS_STATUS AddHucSecureState(
        void *              state,
        void *              hevcSliceState,
        PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief  Set Huc StreamObject Params
    //!
    virtual MOS_STATUS SetHucStreamObj(
        PMHW_VDBOX_HUC_STREAM_OBJ_PARAMS hucStreamObjParams) = 0;

    //!
    //! \brief  Add Hevc Hcp SecureState
    //!
    virtual MOS_STATUS AddHcpSecureState(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        void                            *sliceState) = 0;

    //!
    //! \brief  Reset VP9 SegId Buffer With Huc
    //!
    virtual MOS_STATUS ResetVP9SegIdBufferWithHuc(
        void                    *state,
        PMOS_COMMAND_BUFFER     cmdBuffer) = 0;

    //!
    //! \brief  Update VP9 ProbBuffer With Huc
    //!
    virtual MOS_STATUS UpdateVP9ProbBufferWithHuc(
        bool                     isFullUpdate,
        void                     *state,
        PMOS_COMMAND_BUFFER      cmdBuffer) = 0;

    //!
    //! \brief Encrypt default cdf table buffer through HuC copy
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ProtectDefaultCdfTableBuffer(
        void            *state,
        uint32_t        bufIndex,
        PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief Return the temp cdf table buffer
    //! \return PMOS_RESOURCE
    //!
    virtual PMOS_RESOURCE GetTempCdfTableBuffer() = 0;

    //!
    //! \brief Encrypt aux buffer through partial copy
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitAuxSurface(
        PMOS_RESOURCE res,
        PMOS_COMMAND_BUFFER cmdBuffer) = 0;

    //!
    //! \brief  Is Dummy SteamOut Enabled
    //!
    virtual bool IsDummyStreamEnabled() = 0;

    //!
    //! \brief  Is secure decode is enabled
    //!
    virtual bool IsSecureDecodeEnabled() = 0;

    //!
    //! \brief  Destructor
    //!
    virtual ~CodechalSecureDecodeInterface(){} 

    //!
    //! \brief  Update huc streamout buffer index
    //!
    virtual MOS_STATUS UpdateHuCStreamoutBufferIndex() = 0;
};


//!
//! \brief    Create CodechalSecureDeocde Object according CPLIB loading status
//!           Must use Delete_CodechalSecureDecodeInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
CodechalSecureDecodeInterface *Create_SecureDecodeInterface(
    CodechalSetting *      codechalSettings,
    CodechalHwInterface *  hwInterfaceInput);

//!
//! \brief    Delete the CodecHalSecureDecode Object according CPLIB loading status
//!
//! \param    [in] *pMhwCpInterface 
//!           MhwCpInterface
//!
void Delete_SecureDecodeInterface(CodechalSecureDecodeInterface* pCodechalSecureDecodeInterface);

#endif // __CODECHAL_SECURE_DECODE_H__
