/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2022, Intel Corporation

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
//! \file     vphal_xe_xpm.h
//! \brief    vphal interface clarification for GEN12 TGL HP
//! \details  vphal interface clarification for GEN12 TGL HP inlcuding:
//!           some marcro, enum, structure, function
//!
#ifndef __VPHAL_XE_XPM_H__
#define __VPHAL_XE_XPM_H__

#include "vphal.h"

//!
//! Class VphalStateXe_Xpm
//! \brief VPHAL class definition for GEN12 TGL HP
//!
class VphalStateXe_Xpm : public VphalState
{
public:
    //!
    //! \brief    VphalState Constructor
    //! \details  Creates instance of VphalState
    //!           - Caller must call Allocate to allocate all VPHAL states and objects.
    //! \param    [in] pOsInterface
    //!           OS interface, if provided externally - may be NULL
    //! \param    [in] pOsDriverContext
    //!           OS driver context (UMD context, pShared, ...)
    //! \param    [in,out] pStatus
    //!           Pointer to the MOS_STATUS flag.
    //!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    VphalStateXe_Xpm(
        PMOS_INTERFACE          pOsInterface,
        MOS_STATUS              *peStatus) :
        VphalState(pOsInterface, peStatus)
    {
        // check the peStatus returned from VphalState
        MOS_STATUS eStatus = peStatus ? (*peStatus) : MOS_STATUS_SUCCESS;

        if (MOS_FAILED(eStatus))
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("VphalStateXe_Xpm construct failed due to VphalState() returned failure: eStatus = %d", eStatus);
            return;
        }

        bool                        bComputeContextEnabled;

        // bComputeContextEnabled is true only if Gen12+.
        // Gen12+, compute context(MOS_GPU_NODE_COMPUTE, MOS_GPU_CONTEXT_COMPUTE) can be used for render engine.
        // Before Gen12, we only use MOS_GPU_NODE_3D and MOS_GPU_CONTEXT_RENDER.
        bComputeContextEnabled      = true;

#if (_DEBUG || _RELEASE_INTERNAL)
        bool       computeContextEnabled = false;
        MOS_STATUS                  eRegKeyReadStatus = MOS_STATUS_SUCCESS;
        eRegKeyReadStatus = ReadUserSettingForDebug(
            m_userSettingPtr,
            computeContextEnabled,
            __VPHAL_ENABLE_COMPUTE_CONTEXT,
            MediaUserSetting::Group::Sequence);
        if (eRegKeyReadStatus == MOS_STATUS_SUCCESS)
        {
            bComputeContextEnabled = computeContextEnabled ? true : false;
        }
#endif

        if (m_skuTable && !MEDIA_IS_SKU(m_skuTable, FtrCCSNode))
        {
            bComputeContextEnabled = false;
        }

        if (bComputeContextEnabled)
        {
            m_renderGpuContext    = MOS_GPU_CONTEXT_COMPUTE;
            m_renderGpuNode       = MOS_GPU_NODE_COMPUTE;
        }
    }

    //!
    //! \brief    VphalState Destuctor
    //! \details  Destroys VPHAL and all internal states and objects
    //! \return   VOID
    //!
    ~VphalStateXe_Xpm()
    {
    }

    //!
    //! \brief    Allocate VPHAL Resources
    //! \details  Allocate VPHAL Resources
    //!           - Allocate and initialize HW states
    //!           - Allocate and initialize renderer states
    //! \param    [in] pVpHalSettings
    //!           Pointer to VPHAL Settings
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Allocate(
        const VphalSettings     *pVpHalSettings) override;

protected:

    virtual bool IsRenderContextBasedSchedulingNeeded() override
    {
        return true;
    }

    //!
    //! \brief    Create instance of VphalRenderer
    //! \details  Create instance of VphalRenderer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS CreateRenderer() override;
};

#endif  // __VPHAL_XE_XPM_H__
