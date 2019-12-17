/*
* Copyright (c) 2019, Intel Corporation
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
#ifndef __VP_VEBOX_CMD_PACKET_G12_H__
#define __VP_VEBOX_CMD_PACKET_G12_H__

#include "vp_vebox_cmd_packet.h"

//!
//! \brief Denoise Definitions
//!
#define NOISE_HISTORY_MAX_DEFAULT_G12                            208
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_G12              2
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G12      4
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G12     8
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_LOW_G12         10
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G12        14
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G12        128
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G12       144

//!
//! \brief added 4 LSB for ASD/STAD/SCM/LTDT/TDT, hence shifting 4 below.
//!
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_LOW_G12               (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_G12           (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_HIGH_G12              (40  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_LOW_G12          (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_G12      (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_HIGH_G12         (40  << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G12             (4   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G12         (8   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G12            (8   << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_LOW_G12                (8  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G12            (12  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G12               (12  << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G12               (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_G12           (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G12              (144 << 4)

//!
//! \brief Vebox Statistics Surface definition for TGL
//!
#define VPHAL_VEBOX_STATISTICS_SIZE_G12                          (32 * 8)
#define VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G12                (32 * sizeof(uint32_t))
#define VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G12            0
#define VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12            0x2C
#define VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G12            0x44

namespace vp {

class VpVeboxCmdPacketG12 : virtual public VpVeboxCmdPacket
{
public:
    VpVeboxCmdPacketG12(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc);

    virtual ~VpVeboxCmdPacketG12();

    virtual void GetLumaDefaultValue(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams);

    virtual MOS_STATUS SetDNParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    virtual MOS_STATUS SetDIParams(
            PVPHAL_SURFACE                  pSrcSurface);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    //!
    //! \brief    Setup Vebox_DI_IECP Command params
    //! \details  Setup Vebox_DI_IECP Command params
    //! \param    [in] bDiScdEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxDiIecpCmdParams
    //!           Pointer to VEBOX_DI_IECP command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupDiIecpState(
        bool                          bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams);

    virtual MOS_STATUS QueryStatLayout(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t*             pQuery);

};

}
#endif // !__VP_VEBOX_CMD_PACKET_G12_H__
