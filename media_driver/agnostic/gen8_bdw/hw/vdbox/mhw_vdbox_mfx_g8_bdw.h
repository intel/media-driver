/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     mhw_vdbox_mfx_g8_bdw.h
//! \brief    Defines functions for constructing Vdbox MFX commands on BDW
//!

#ifndef __MHW_VDBOX_MFX_G8_BDW_H__
#define __MHW_VDBOX_MFX_G8_BDW_H__

#include "mhw_vdbox_mfx_g8_X.h"
#include "mhw_vdbox_mfx_hwcmd_g8_bdw.h"

//!  MHW Vdbox Mfx interface for Gen8 BDW platform
/*!
This class defines the Mfx command construction functions for Gen8 BDW platform
*/
class MhwVdboxMfxInterfaceG8Bdw : public MhwVdboxMfxInterfaceG8<mhw_vdbox_mfx_g8_bdw>
{
public:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxMfxInterfaceG8Bdw(
        PMOS_INTERFACE osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool decodeInUse) :
        MhwVdboxMfxInterfaceG8(osInterface, miInterface, cpInterface, decodeInUse)
    {
        MHW_FUNCTION_ENTER;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxMfxInterfaceG8Bdw() { MHW_FUNCTION_ENTER; }

protected:
    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams)
    {
        MOS_UNUSED(rowstoreParams);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddMfxPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params);

    MOS_STATUS AddMfxBspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS params);

    MOS_STATUS AddMfxJpegPicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_JPEG_PIC_STATE params);

    MOS_STATUS AddMfxJpegEncodePicStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegEncodePicState *params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;;
    }

    MOS_STATUS AddMfxJpegFqmCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_QM_PARAMS params,
        uint32_t numQuantTables)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddMfcJpegHuffTableStateCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_ENCODE_HUFF_TABLE_PARAMS params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddMfcJpegScanObjCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        MhwVdboxJpegScanParams *params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddMfxDecodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params);

    MOS_STATUS AddMfxEncodeVp8PicCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_PIC_STATE params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;;
    }

    MOS_STATUS InitMfxVp8EncoderCfgCmd(
        PMOS_RESOURCE cfgCmdBuffer,
        PMHW_VDBOX_VP8_ENCODER_CFG_PARAMS params)
    {
        MOS_UNUSED(cfgCmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }

    MOS_STATUS AddMfxVp8BspBufBaseAddrCmd(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMHW_VDBOX_VP8_BSP_BUF_BASE_ADDR_PARAMS params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);
        return MOS_STATUS_PLATFORM_NOT_SUPPORTED;
    }
};

#endif

