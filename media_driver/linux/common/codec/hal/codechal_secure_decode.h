/*
* Copyright (c) 2013-2017, Intel Corporation
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
//! \file      codechal_secure_decode.h  
//! \brief     Stub file for CodecHal Secure Decode  
//!
#ifndef __CODECHAL_SECURE_DECODE_H__
#define __CODECHAL_SECURE_DECODE_H__

#include "codechal_hw.h"

//!
//! \class  CodechalSecureDecode
//! \brief  Codechal secure decode
//!
class CodechalSecureDecode
{
public:

    //!
    //! \brief  Create Secure Decode
    //! \return MOS_STATUS
    //!
    static MOS_STATUS CreateSecureDecode(
        CodechalSetting           *codecHalSettings, 
        CodechalHwInterface       *hwInterfaceInput, 
        CodechalSecureDecode      **secureDecoder)
    
    {
        *secureDecoder = nullptr;
        return MOS_STATUS_SUCCESS;
    };

    //!
    //! \brief  Allocate Resource
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS AllocateResource(void *state)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Execute Huc StreamOut
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS Execute(void *state)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Set Bitstream Buffer
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS SetBitstreamBuffer(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS *indObjBaseAddrParams)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Set HEVC Huc Dmem S2L Bss
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS SetHevcHucDmemS2LBss(
        void          *state,
        void          *hucPicBss,
        void          *hucSliceBss)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Add Hevc Huc SecureState
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS AddHucSecureState(
        void                            *state,
        PMHW_VDBOX_HEVC_SLICE_STATE     hevcSliceState,
        PMOS_COMMAND_BUFFER             cmdBuffer)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Set Huc StreamObject Params
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS SetHucStreamObj(
        PMHW_VDBOX_HUC_STREAM_OBJ_PARAMS hucStreamObjParams)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Add Hevc Hcp SecureState
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS AddHcpSecureState(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        void                            *sliceState)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Reset VP9 SegId Buffer With Huc
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS ResetVP9SegIdBufferWithHuc(
        void                    *state,
        PMOS_COMMAND_BUFFER     cmdBuffer)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Update VP9 ProbBuffer With Huc
    //! \return MOS_STATUS_UNIMPLEMENTED
    //!
    MOS_STATUS UpdateVP9ProbBufferWithHuc(
        bool                     isFullUpdate,
        void                     *state,
        PMOS_COMMAND_BUFFER      cmdBuffer)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    };

    //!
    //! \brief  Is Dummy SteamOut Enabled
    //! \return false
    //!
    bool IsDummyStreamEnabled()
    {
        return false;
    };

    //!
    //! \brief  Destructor
    //!
    ~CodechalSecureDecode(){};

protected:

    //!
    //! \brief  Constructor
    //! \param  [in] hwInterfaceInput
    //!         Hardware interface
    //!
    CodechalSecureDecode(CodechalHwInterface *hwInterfaceInput){};
};

#endif // __CODECHAL_SECURE_DECODE_H__
