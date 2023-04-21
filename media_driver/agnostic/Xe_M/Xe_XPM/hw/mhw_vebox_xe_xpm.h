/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

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
//! \file     mhw_vebox_xe_xpm.h
//! \brief    Defines functions for constructing vebox commands on Xe_XPM-based platforms
//!

#ifndef __MHW_VEBOX_XE_XPM_H__
#define __MHW_VEBOX_XE_XPM_H__

#include "mhw_vebox_g12_X.h"
#include "mhw_vebox_hwcmd_xe_xpm.h"

typedef enum _MHW_VEBOX_INDEX
{
    MHW_VEBOX_INDEX_0 = 0,
    MHW_VEBOX_INDEX_1 = 1,
    MHW_VEBOX_INDEX_2 = 2,
    MHW_VEBOX_INDEX_3 = 3
} MHW_VEBOX_INDEX;

//!
//! \brief Macro for Vebox Scalable
//!
#define MHW_VEBOX_MAX_PIPE_SIZE_G12                       4096
#define MHW_VEBOX_MAX_SEMAPHORE_NUM_G12                   4
#define MHW_VEBOX_TIMESTAMP_CNTS_PER_SEC_G12              12000048
#define MHW_VEBOX_4K_PIC_WIDTH_G12                        3840
#define MHW_VEBOX_4K_PIC_HEIGHT_G12                       2160

//!  MHW vebox  interface for Xe_XPM
/*!
This class defines the VEBOX command interface for Xe_XPM platforms
*/
class MhwVeboxInterfaceXe_Xpm: public MhwVeboxInterfaceG12
{
public:
    MhwVeboxInterfaceXe_Xpm(PMOS_INTERFACE pOsInterface);
    virtual ~MhwVeboxInterfaceXe_Xpm();

    MOS_STATUS AddVeboxDiIecp(
        PMOS_COMMAND_BUFFER           pCmdBuffer,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams) override;

    MOS_STATUS AddVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams) override;

    MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
        uint32_t                  *pdwSurfaceWidth,
        uint32_t                  *pdwSurfaceHeight,
        bool                      bDIEnable,
        bool                      b3DlutEnable);

    //!
    //! \brief    Set which vebox can be used by HW
    //! \details  VPHAL set which VEBOX can be use by HW
    //! \param    [in] dwVeboxIndex;
    //!           set which Vebox can be used by HW
    //! \param    [in] dwVeboxCount;
    //!           set Vebox Count
    //! \param    [in] dwUsingSFC;
    //!           set whether using SFC
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxIndex(
        uint32_t                    dwVeboxIndex,
        uint32_t                    dwVeboxCount,
        uint32_t                    dwUsingSFC) override;

    //!
    //! \brief    Create Gpu Context for Vebox
    //! \details  Create Gpu Context for Vebox
    //! \param    [in] pOsInterface
    //!           OS interface
    //! \param    [in] VeboxGpuContext
    //!           Vebox Gpu Context
    //! \param    [in] VeboxGpuNode
    //!           Vebox Gpu Node
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CreateGpuContext(
        PMOS_INTERFACE  pOsInterface,
        MOS_GPU_CONTEXT VeboxGpuContext,
        MOS_GPU_NODE    VeboxGpuNode) override;

    MOS_STATUS AddVeboxSurfaces(
        PMOS_COMMAND_BUFFER                 pCmdBuffer,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams) override;

    MOS_STATUS FindVeboxGpuNodeToUse(
        PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit) override;

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Validate the GPU Node to use for Vebox
    //! \details  Validate the GPU Node to create gpu context used by vebox
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateVeboxScalabilityConfig();
#endif

    //! \brief    get the status for vebox's Scalability.
    //! \details  VPHAL will check whether the veobx support Scalabiltiy.
    //! \return   bool
    //!           false or true
    bool IsScalabilitySupported();

protected:
    bool     m_veboxScalabilitySupported = false;
    bool     m_veboxScalabilityEnabled   = false;
    uint32_t m_indexofVebox              = 0;
    uint32_t m_numofVebox                = 1;
    uint32_t m_usingSfc                  = 0;
   
    using MhwVeboxInterfaceG12::SetVeboxSurfaces;

    void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS                  pSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                  pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                  pSkinScoreSurfaceParam,
        mhw_vebox_xe_xpm::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
        bool                                       bIsOutputSurface,
        bool                                       bDIEnable,
        bool                                       b3DlutEnable);

};

#endif // __MHW_VEBOX_XE_XPM_H__
