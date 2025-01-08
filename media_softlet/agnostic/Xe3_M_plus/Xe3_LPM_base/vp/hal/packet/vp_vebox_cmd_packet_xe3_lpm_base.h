/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_vebox_cmd_packet_xe3_lpm_base.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#ifndef __VP_VEBOX_CMD_PACKET_XE3_LPM_BASE_H__
#define __VP_VEBOX_CMD_PACKET_XE3_LPM_BASE_H__

#include "vp_vebox_cmd_packet.h"

//!
//! \brief Vebox Statistics Surface definition for Xe3_LPM_BASE
//!
#define VP_VEBOX_STATISTICS_SIZE_XE3_LPM_BASE                                (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE_XE3_LPM_BASE                      (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_XE3_LPM_BASE                   0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_XE3_LPM_BASE                   0x2C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET_XE3_LPM_BASE                   0x44
#define VP_VEBOX_STATISTICS_SURFACE_SW_WA_OFFSET_XE3_LPM_BASE                 -12

//!
//! \brief Denoise Definitions
//!
#define NOISE_HISTORY_MAX_DEFAULT_XE3_LPM_BASE                            208
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_XE3_LPM_BASE              2
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_XE3_LPM_BASE      4
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_XE3_LPM_BASE     8
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_LOW_XE3_LPM_BASE         10
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_HIGH_XE3_LPM_BASE        14
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_LOW_XE3_LPM_BASE        128
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_XE3_LPM_BASE       144

//!
//! \brief added 4 LSB for ASD/STAD/SCM/LTDT/TDT, hence shifting 4 below.
//!
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_LOW_XE3_LPM_BASE               (32 << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_XE3_LPM_BASE           (32 << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_HIGH_XE3_LPM_BASE              (40 << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_LOW_XE3_LPM_BASE          (32 << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_XE3_LPM_BASE      (32 << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_HIGH_XE3_LPM_BASE         (40 << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_XE3_LPM_BASE             (4 << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_XE3_LPM_BASE         (8 << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_XE3_LPM_BASE            (8 << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_LOW_XE3_LPM_BASE                (8 << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_XE3_LPM_BASE            (12 << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_HIGH_XE3_LPM_BASE               (12 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_LOW_XE3_LPM_BASE               (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_XE3_LPM_BASE           (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_XE3_LPM_BASE              (144 << 4)

namespace vp
{
class VpVeboxCmdPacketXe3_Lpm_Base : virtual public VpVeboxCmdPacket
{
public:
    virtual ~VpVeboxCmdPacketXe3_Lpm_Base();

    virtual void GetLumaDefaultValue(
        PVP_SAMPLER_STATE_DN_PARAM pLumaParams);

    virtual MOS_STATUS GetDnLumaParams(
        bool                       bDnEnabled,
        bool                       bAutoDetect,
        float                      fDnFactor,
        bool                       bRefValid,
        PVP_SAMPLER_STATE_DN_PARAM pLumaParams) override;

    virtual MOS_STATUS GetDnChromaParams(
        bool               bChromaDenoise,
        bool               bAutoDetect,
        float              fDnFactor,
        PVPHAL_DNUV_PARAMS pChromaParams) override;

    virtual MOS_STATUS ConfigLumaPixRange(
        bool  bDnEnabled,
        bool  bAutoDetect,
        float fDnFactor) override;

    virtual MOS_STATUS ConfigChromaPixRange(
        bool  bChromaDenoise,
        bool  bAutoDetect,
        float fDnFactor) override;

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
        mhw::vebox::VEB_DI_IECP_PAR   &veboxDiIecpCmdParam) override;

    virtual MOS_STATUS QueryStatLayout(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t *            pQuery) override;

    virtual MOS_STATUS InitSurfMemCacheControl(VP_EXECUTE_CAPS packetCaps) override;

protected:
    VpVeboxCmdPacketXe3_Lpm_Base(MediaTask *task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc);

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCmdPacketXe3_Lpm_Base)
};

}  // namespace vp
#endif  // !__VP_VEBOX_CMD_PACKET_XE3_LPM_BASE_H__
