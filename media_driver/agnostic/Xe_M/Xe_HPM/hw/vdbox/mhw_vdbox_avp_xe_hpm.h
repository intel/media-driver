/*===================== begin_copyright_notice ==================================

# Copyright (c) 2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_avp_xe_hpm.h
//! \brief    Defines functions for constructing Vdbox AVP commands on DG2 platform
//!

#ifndef __MHW_VDBOX_AVP_XE_HPM_H__
#define __MHW_VDBOX_AVP_XE_HPM_H__

#include "mhw_vdbox_avp_g12_X.h"
#include "mhw_vdbox_avp_impl_xe_hpm.h"

class MhwVdboxAvpPipeBufAddrParamsXe_Hpm : public MhwVdboxAvpPipeBufAddrParams
{
public:
    MOS_RESOURCE *m_originalUncompressedPictureSourceBuffer   = nullptr;
    MOS_RESOURCE *m_downscaledUncompressedPictureSourceBuffer = nullptr;
    MOS_RESOURCE *m_tileSizeStreamoutBuffer                   = nullptr;
    uint32_t      m_tileSizeStreamoutBufferSize               = 0;
    uint32_t      m_tileSizeStreamoutBufferOffset             = 0;
    MOS_RESOURCE *m_tileStatisticsPakStreamoutBuffer          = nullptr;
    MOS_RESOURCE *m_cuStreamoutBuffer                         = nullptr;
    MOS_RESOURCE *m_sseLineReadWriteBuffer                    = nullptr;
    MOS_RESOURCE *m_sseTileLineReadWriteBuffer                = nullptr;
    MOS_SURFACE  *m_postCDEFpixelsBuffer                      = nullptr;

    MOS_MEMCOMP_STATE m_postCdefSurfMmcState                  = {};
    virtual void Initialize();
    //!
    //! \brief    Destructor
    //!
    virtual ~MhwVdboxAvpPipeBufAddrParamsXe_Hpm() {}
};

//!  MHW Vdbox Avp interface for Xe_HPM
/*!
This class defines the Avp command interface for Xe_HPM platforms
*/
class MhwVdboxAvpInterfaceXe_Hpm : public MhwVdboxAvpInterfaceG12
{
public:
    MhwVdboxAvpInterfaceXe_Hpm(
        PMOS_INTERFACE  osInterface,
        MhwMiInterface *miInterface,
        MhwCpInterface *cpInterface,
        bool            decodeInUse);
    ~MhwVdboxAvpInterfaceXe_Hpm();

    MOS_STATUS GetAvpStateCommandSize(
        uint32_t *                      commandsSize,
        uint32_t *                      patchListSize,
        PMHW_VDBOX_STATE_CMDSIZE_PARAMS params) override;

    MOS_STATUS GetAvpPrimitiveCommandSize(
        uint32_t *commandsSize,
        uint32_t *patchListSize) override;

    MOS_STATUS AddAvpPipeModeSelectCmd(
        PMOS_COMMAND_BUFFER                cmdBuffer,
        PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params) override;

    MOS_STATUS AddAvpDecodeSurfaceStateCmd(
        PMOS_COMMAND_BUFFER       cmdBuffer,
        PMHW_VDBOX_SURFACE_PARAMS params) override;

    MOS_STATUS AddAvpPipeBufAddrCmd(
        PMOS_COMMAND_BUFFER           cmdBuffer,
        MhwVdboxAvpPipeBufAddrParams *params) override;

    //!
    //! \brief    Adds AVP Ind Obj Base Address command
    //! \details  function to add AVP Ind Obj Base Address command in command buffer
    //!
    //! \param    [in] cmdBuffer
    //!           Command buffer to which HW command is added
    //! \param    [in] params
    //!           Params structure used to populate the HW command
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddAvpIndObjBaseAddrCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS params) override;

    std::shared_ptr<void> GetNewAvpInterface() override
    {
        if (!m_avpItfNew)
        {
            auto ptr = std::make_shared<mhw::vdbox::avp::xe_hpm::Impl>(m_osInterface);
            ptr->SetCacheabilitySettings(m_cacheabilitySettings);
            m_avpItfNew = ptr;
        }

        return m_avpItfNew;
    }
};

#endif
