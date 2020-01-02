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

//! \file     mhw_vdbox_vdenc_g10_X.h
//! \details  Defines functions for constructing Vdbox Vdenc commands on Gen10-based platforms
//!

#ifndef __MHW_VDBOX_VDENC_G10_X_H__
#define __MHW_VDBOX_VDENC_G10_X_H__

#include "mhw_vdbox_vdenc_generic.h"
#include "mhw_vdbox_vdenc_hwcmd_g10_X.h"

//!  MHW Vdbox Vdenc interface for Gen10
/*!
This class defines the Vdenc command construction functions for Gen10 platforms as template
*/
class MhwVdboxVdencInterfaceG10 : public MhwVdboxVdencInterfaceGeneric<mhw_vdbox_vdenc_g10_X>
{
protected:
    inline uint32_t GetVdencCmd1Size()
    {
        return 0;
    }

    inline uint32_t GetVdencCmd2Size()
    {
        return 0;
    }

    enum CommandsNumberOfAddresses
    {
        MI_FLUSH_DW_CMD_NUMBER_OF_ADDRESSES = 1,
        MI_BATCH_BUFFER_START_CMD_NUMBER_OF_ADDRESSES = 1,
        VDENC_PIPE_BUF_ADDR_STATE_CMD_NUMBER_OF_ADDRESSES = 21
    };

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

    MOS_STATUS InitRowstoreUserFeatureSettings();

public:
    //!
    //! \brief  Constructor
    //!
    MhwVdboxVdencInterfaceG10(PMOS_INTERFACE osInterface) : MhwVdboxVdencInterfaceGeneric(osInterface)
    {
        MHW_FUNCTION_ENTER;

        m_rhoDomainStatsEnabled = true;
        InitRowstoreUserFeatureSettings();
    }

    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxVdencInterfaceG10() { }

    MOS_STATUS GetRowstoreCachingAddrs(
        PMHW_VDBOX_ROWSTORE_PARAMS rowstoreParams);

    MOS_STATUS GetVdencStateCommandsDataSize(
        uint32_t                        mode,
        uint32_t                        waAddDelayInVDEncDynamicSlice,
        uint32_t                        *commandsSize,
        uint32_t                        *patchListSize);

    //!
    //! \brief    Translate MOS type format to Mediastate VDEnc surface format
    //! \details  VDBOX protected function to translate mos format to media state format
    //! \param    MOS_FORMAT  Format
    //!           [in] MOS type format
    //! \return   VdencSurfaceFormat
    //!           media state surface format
    //!
    VdencSurfaceFormat MosFormatToVdencSurfaceFormat(
        MOS_FORMAT format);

    MOS_STATUS AddVdencPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS   params);

    MOS_STATUS AddVdencPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS      params);

    MOS_STATUS AddVdencSrcSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    MOS_STATUS AddVdencRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params);

    MOS_STATUS AddVdencDsRefSurfaceStateCmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS            params,
        uint8_t                              numSurfaces);

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

    MOS_STATUS AddVdencWeightsOffsetsStateCmd(
        PMOS_COMMAND_BUFFER                   cmdBuffer,
        PMHW_BATCH_BUFFER                     batchBuffer,
        PMHW_VDBOX_VDENC_WEIGHT_OFFSET_PARAMS params);

    MOS_STATUS AddVdencCmd1Cmd(
        PMOS_COMMAND_BUFFER                  cmdBuffer,
        PMHW_BATCH_BUFFER                    batchBuffer,
        PMHW_VDBOX_VDENC_CMD1_PARAMS  params)
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS AddVdencCmd2Cmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_BATCH_BUFFER                   batchBuffer,
        PMHW_VDBOX_VDENC_CMD2_STATE params)
    {
        return MOS_STATUS_SUCCESS;
    }

    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS CreateMhwVdboxPipeModeSelectParams()
    {
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams = MOS_New(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS);

        return pipeModeSelectParams;
    }

    void ReleaseMhwVdboxPipeModeSelectParams(PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS pipeModeSelectParams) 
    {
        MOS_Delete(pipeModeSelectParams);
    }
};

#endif
