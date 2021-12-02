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
//! \file     mhw_sfc_xe_xpm.h
//! \brief    Defines functions for constructing sfc commands on Gen12-based platforms
//!

#ifndef __MHW_SFC_XE_XPM_H__
#define __MHW_SFC_XE_XPM_H__

#include "mhw_sfc_g12_X.h"
#include "mhw_sfc_hwcmd_xe_xpm.h"

typedef enum _MHW_SFC_INDEX
{
    MHW_SFC_INDEX_0          = 0,
    MHW_SFC_INDEX_1          = 1,
    MHW_SFC_INDEX_2          = 2,
    MHW_SFC_INDEX_3          = 3
} MHW_SFC_INDEX;

#define MHW_SFC_MAX_PIPE_NUM_XE_XPM                        4

struct MHW_SFC_STATE_PARAMS_XE_XPM: public MHW_SFC_STATE_PARAMS_G12
{
    uint32_t    ditheringEn; //!< 0 - disable, 1 - enable.

    // Interlaced Scaling parameters
    uint32_t                        iScalingType;
    uint32_t                        inputFrameDataFormat;                       // Input frame data format -- Progressive, Interleaved, Field mode
    uint32_t                        outputFrameDataFormat;                      // Output frame data format -- Progressive, Interleaved, Field mode
    uint32_t                        topBottomField;                             // Top/Bottom field -- Top field, Bottom field
    uint32_t                        topBottomFieldFirst;                        // Top/Bottom field first
    uint32_t                        outputSampleType;                           // Output sample type
    uint32_t                        bottomFieldVerticalScalingOffset;           // Bottom field vertical scaling offset
    PMOS_RESOURCE                   tempFieldResource;                          // Temp filed surface

    PMOS_RESOURCE                   pOsResAVSLineBufferSplit[MHW_SFC_MAX_PIPE_NUM_XE_XPM]; //!< AVS Line buffer used by SFC
    PMOS_RESOURCE                   pOsResIEFLineBufferSplit[MHW_SFC_MAX_PIPE_NUM_XE_XPM]; //!< IEF Line buffer used by SFC
};
using PMHW_SFC_STATE_PARAMS_XE_XPM = MHW_SFC_STATE_PARAMS_XE_XPM*;

//!  MHW SFC interface for Xe_XPM
/*!
This class defines the SFC command interface for Gen12 common platforms
*/
class MhwSfcInterfaceXe_Xpm: public MhwSfcInterfaceG12
{
public:
    MhwSfcInterfaceXe_Xpm(PMOS_INTERFACE pOsInterface);
    ~MhwSfcInterfaceXe_Xpm();

    MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_SFC_STATE_PARAMS       pSfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS pOutSurface);

    //!
    //! \brief    Set which Sfc can be used by HW
    //! \details  VPHAL set which Sfc can be use by HW
    //! \param    [in] dwSfcIndex;
    //!           set which Sfc can be used by HW
    //! \param    [in] dwSfcCount;
    //!           set Sfc Count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetSfcIndex(
        uint32_t                    dwSfcIndex,
        uint32_t                    dwSfcCount);

private:
    bool     m_sfcScalabilitySupported;
    bool     m_sfcScalabilityEnabled;
    uint32_t m_indexofSfc;
    uint32_t m_numofSfc;
};
#endif // __MHW_SFC_XE_XPM_H__
