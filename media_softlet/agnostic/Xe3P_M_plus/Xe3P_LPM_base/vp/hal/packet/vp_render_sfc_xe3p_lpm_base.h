/* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     vp_render_sfc_xe3p_lpm_base.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_XE3P_LPM_BASE_H__
#define __VP_RENDER_SFC_XE3P_LPM_BASE_H__

#include "vp_render_sfc_base.h"

#define SIZE_OF_EOTF 4096
#define SIZE_OF_CCM  15
namespace vp
{
class SfcRenderXe3P_Lpm_Base : public SfcRenderBase
{
public:
    virtual ~SfcRenderXe3P_Lpm_Base();

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters
    //! \param    [in] targetSurface
    //!           Pointer to Output Surface
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSfcState(
        PVP_SURFACE targetSurface);

    virtual MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        mhw::sfc::SFC_LOCK_PAR *pSfcLockParams);

    virtual bool IsVdboxSfcInputFormatSupported(
        CODECHAL_STANDARD codecStandard,
        MOS_FORMAT        inputFormat);

    virtual bool IsVdboxSfcOutputFormatSupported(
        CODECHAL_STANDARD codecStandard,
        MOS_FORMAT        outputFormat,
        MOS_TILE_TYPE     tileType);

    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams);

    //!
    //! \brief    Set sfc pipe selected with vebox
    //! \details  Set sfc pipe selected with vebox
    //! \param    [in] dwSfcIndex
    //!           Sfc pipe selected with vebox
    //! \param    [in] dwSfcCount
    //!           Sfc pipe num in total
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcPipe(
        uint32_t dwSfcIndex,
        uint32_t dwSfcCount);

protected:
    SfcRenderXe3P_Lpm_Base(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);

    //!
    //! \brief    Initiazlize SFC State Parameters
    //! \details  Initiazlize SFC State Parameters
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitSfcStateParams();

    virtual MOS_STATUS SetSfcStateInputOrderingModeVdbox(
        mhw::sfc::SFC_STATE_PAR *sfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeAvp(
        mhw::sfc::SFC_STATE_PAR *sfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeHcp(
        mhw::sfc::SFC_STATE_PAR *sfcStateParams);

    virtual MOS_STATUS SetCodecPipeMode(CODECHAL_STANDARD codecStandard);

    virtual MOS_STATUS SetupScalabilityParams();
    virtual MOS_STATUS SetInterlacedScalingParams(PSFC_SCALING_PARAMS scalingParams);

    virtual MOS_STATUS AllocateResources();
    virtual MOS_STATUS FreeResources();

    virtual uint32_t GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput);

    virtual bool IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat);
    virtual bool IsCscNeeded(SFC_CSC_PARAMS &cscParams);

    virtual MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        mhw::sfc::SFC_STATE_PAR        *pSfcState,
        PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface);
    virtual double GammaSW_st2084(double input);
    virtual double GammaSW_G22ToG10(double input, double linearThresh, double linear, double power, double multiplier, double offset);
    virtual MOS_STATUS Gen2084EOTFLUT_1K(uint32_t *LUT_y, uint32_t num_pt);
    virtual MOS_STATUS GenG22EOTFLUT_1K(uint32_t *LUT_y, uint32_t num_pt);

    VP_SURFACE *m_tempFieldSurface = nullptr;
    VP_SURFACE *m_sfcIndirectState = nullptr;
    const uint32_t    m_dWsizeOfSfcIndirectState = (SIZE_OF_EOTF + SIZE_OF_CCM);
    uint32_t    m_sizeOfSfcIndirectState = (SIZE_OF_EOTF + SIZE_OF_CCM) * sizeof(uint32_t);
    uint32_t   *m_EOTF = nullptr;
    uint32_t   *m_IndirectStateLut = nullptr;

MEDIA_CLASS_DEFINE_END(vp__SfcRenderXe3P_Lpm_Base)
};

}
#endif // !__VP_RENDER_SFC_XE3P_LPM_BASE_H__
