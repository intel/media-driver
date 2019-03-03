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
//!
//! \file     mhw_vdbox_vdenc_g9_kbl.h
//! \brief    Defines functions for constructing Vdbox Vdenc commands on G9 KBL
//!

#ifndef __MHW_VDBOX_VDENC_G9_KBL_H__
#define __MHW_VDBOX_VDENC_G9_KBL_H__

#include "mhw_vdbox_vdenc_g9_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g9_kbl.h"

//!  MHW Vdbox Vdenc interface for Gen9 KBL platform
/*!
This class defines the Vdenc command construction functions for Gen9 KBL platform
*/
class MhwVdboxVdencInterfaceG9Kbl : public MhwVdboxVdencInterfaceG9<mhw_vdbox_vdenc_g9_kbl>
{
protected:

    enum VdencSurfaceFormat
    {
        vdencSurfaceFormatYuv422          = 0,    //!< YUYV/YUY2 (8:8:8:8 MSB V0 Y1 U0 Y0)
        vdencSurfaceFormatRgba4444        = 1,    //!< RGBA 32-bit 4:4:4:4 packed (8:8:8:8 MSB-X:B:G:R)
        vdencSurfaceFormatYuv444          = 2,    //!< YUV 32-bit 4:4:4 packed (8:8:8:8 MSB-Y:U:X:V)
        vdencSurfaceFormatY8Unorm         = 3,
        vdencSurfaceFormatPlanar420_8     = 4,    //!< (NV12, IMC1,2,3,4, YV12) 
        vdencSurfaceFormatYcrcbSwapy422   = 5,    //!< UYVY (8:8:8:8 MSB Y1 V0 Y0 U0)
        vdencSurfaceFormatYcrcbSwapuv422  = 6,    //!< YVYU (8:8:8:8 MSB U0 Y1 V0 Y0)
        vdencSurfaceFormatYcrcbSwapuvy422 = 7,    //!< VYUY (8:8:8:8 MSB Y1 U0 Y0 V0)
        vdencSurfaceFormatP010            = 8,    //!< CNL+
        vdencSurfaceFormatRgba_10_10_10_2 = 9,    //!< CNL+
        vdencSurfaceFormatY410            = 10,   //!< CNL+
        vdencSurfaceFormatNv21            = 11,   //!< CNL+
        vdencSurfaceFormatP010Variant     = 12    //!< CNL+
    };

public:
    //!
    //! \brief    Constructor
    //!
    MhwVdboxVdencInterfaceG9Kbl(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceG9(osInterface)
    {
        MHW_FUNCTION_ENTER;

        m_rhoDomainStatsEnabled = true;
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG9Kbl() { }

    //!
    //! \brief    Translate MOS type format to Mediastate VDEnc surface format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    MOS_FORMAT  format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceFormat(
        MOS_FORMAT format);

    MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    MOS_STATUS AddVdencImgStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_AVC_IMG_PARAMS            params);

    MOS_STATUS AddVdencWalkerStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_VDENC_WALKER_STATE_PARAMS params);

    MOS_STATUS AddVdencAvcWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_AVC_WEIGHTOFFSET_PARAMS   params);

};

#endif
