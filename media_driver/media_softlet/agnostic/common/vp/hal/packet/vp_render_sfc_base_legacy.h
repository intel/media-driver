/* Copyright (c) 2022, Intel Corporation
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
//! \file     vp_render_sfc_base_legacy.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_BASE_LEGACY_H__
#define __VP_RENDER_SFC_BASE_LEGACY_H__

#include "vp_render_sfc_base.h"
#include "mhw_sfc_g12_X.h"

namespace vp {

class VpIef;

class SfcRenderBaseLegacy : public SfcRenderBase
{

public:
    SfcRenderBaseLegacy(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);
    virtual ~SfcRenderBaseLegacy();

    //!
    //! \brief    Initialize the object
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Init(VIDEO_PARAMS &videoParams) override;

    //!
    //! \brief    Setup CSC parameters of the SFC State
    //! \param    [in,out] pSfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [out] pIEFStateParams
    //!           Pointer to MHW IEF state params
    //! \return   void
    //!
    virtual MOS_STATUS SetIefStateCscParams(
        PMHW_SFC_STATE_PARAMS     sfcStateParams,
        PMHW_SFC_IEF_STATE_PARAMS pIEFStateParams);

    //!
    //! \brief    Setup parameters related to SFC_IEF State
    //! \details  Setup the IEF and CSC params of the SFC_IEF State
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [in] inputSurface
    //!           Pointer to Input Surface
    //! \return   void
    //!
    virtual MOS_STATUS SetIefStateParams(
        PMHW_SFC_STATE_PARAMS sfcStateParams);

    //!
    //! \brief    Setup parameters related to SFC_AVS State
    //! \details  Setup the 8x8 table of SFC sampler
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetAvsStateParams() override;

    //!
    //! \brief    Send SFC pipe commands
    //! \details  Register the surfaces and send the commands needed by SFC pipe
    //! \param    [in] pRenderData
    //!           Pointer to Vebox Render data
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SendSfcCmd(
        bool                bOutputToMemory,
        PMOS_COMMAND_BUFFER pCmdBuffer) override;

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters including SFC State, AVS
    //!           and IEF parameters
    //! \param    [in] targetSurface
    //!           Pointer to Output Surface
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSfcState(PVP_SURFACE targetSurface) override;

    //!
    //! \brief    check whether SFC Write have offset which may hit compresed write limitation
    //! \details  check whether SFC Write have offset which may hit compresed write limitation
    //! \param    [in] targetSurface
    //!           Pointer to targetSurface
    //! \return   the output pipe compression state
    //!
    virtual bool IsSFCUncompressedWriteNeeded(PVP_SURFACE targetSurface) override;

    //!
    //! \brief    Set scaling parameters
    //! \details  Set scaling parameters
    //! \param    [in] scalingParams
    //!           Scaling parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams) override;

    //!
    //! \brief    Set csc parameters
    //! \details  Set csc parameters
    //! \param    [in] cscParams
    //!           Csc parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetCSCParams(PSFC_CSC_PARAMS cscParams) override;

    //!
    //! \brief    Set rotation and mirror parameters
    //! \details  Set rotation and mirror parameters
    //! \param    [in] rotMirParams
    //!           rotation and mirror parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams) override;

    //!
    //! \brief    Set mmc parameters
    //! \details  Set mmc parameters
    //! \param    [in] renderTarget
    //!           render target surface
    //! \param    [in] isFormatMmcSupported
    //!           Is format supported by mmc
    //! \param    [in] isMmcEnabled
    //!           Is mmc enabled
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled) override;

    virtual MOS_STATUS UpdateIefParams(PVPHAL_IEF_PARAMS iefParams) override;
    virtual MOS_STATUS UpdateCscParams(FeatureParamCsc &cscParams) override;

    bool IsCSC() { return m_renderDataLegacy.bCSC; }
    bool IsScaling() { return m_renderDataLegacy.bScaling; }

    //!
    //! \brief    Get Sfc's input format
    //! \return   MOS_FORMAT
    //!
    MOS_FORMAT GetInputFormat()
    {
        return m_renderDataLegacy.SfcInputFormat;
    }

    PVPHAL_IEF_PARAMS GetIefParams()
    {
        return m_renderDataLegacy.pIefParams;
    }

protected:

    //!
    //! \brief    Set SFC input chroma subsampling
    //! \details  Set SFC input chroma subsampling according to
    //!           pipe mode
    //! \param    [out] sfcStateParams
    //!           Pointer to SFC state params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcStateInputChromaSubSampling(
        PMHW_SFC_STATE_PARAMS sfcStateParams);

    //!
    //! \brief    Set SFC input ordering mode
    //! \details  SFC input ordering mode according to
    //!           pipe mode
    //! \param    [out] sfcStateParams
    //!           Pointer to SFC state params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcStateInputOrderingMode(
        PMHW_SFC_STATE_PARAMS sfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeJpeg(
        PMHW_SFC_STATE_PARAMS sfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeVdbox(
        PMHW_SFC_STATE_PARAMS sfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeHcp(
        PMHW_SFC_STATE_PARAMS sfcStateParams) = 0;

    //!
    //! \brief    Setup ColorFill parameters
    //! \details  Setup ColorFill parameters
    //! \param    [in] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    void SetColorFillParams(
        PMHW_SFC_STATE_PARAMS pSfcStateParams);

    //!
    //! \brief    Setup Rotation and Mirrow params
    //! \details  Setup Rotation and Mirrow params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetRotationAndMirrowParams(
        PMHW_SFC_STATE_PARAMS pSfcStateParams);

    //!
    //! \brief    Setup Chromasting params
    //! \details  Setup Chromasting params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetChromasitingParams(
        PMHW_SFC_STATE_PARAMS pSfcStateParams);

    //!
    //! \brief    Setup Bypass X & Y AdaptiveFilter params
    //! \details  Setup Bypass X & Y AdaptiveFilter params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetXYAdaptiveFilter(
        PMHW_SFC_STATE_PARAMS pSfcStateParams);

    //!
    //! \brief    Setup RGB Adaptive params
    //! \details  Setup RGB Adaptive params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetRGBAdaptive(
        PMHW_SFC_STATE_PARAMS pSfcStateParams);

    //!
    //! \brief    Get Avs line buffer size
    //! \details  Get Avs line buffer size according to height of input surface
    //! \param    [in] lineTiledBuffer
    //!           ture if avs line tile buffer, otherwise, avs line buffer.
    //! \param    [in] b8tapChromafiltering
    //!           ture if 8-tap UV, otherwise, 4-tap UV.
    //! \param    [in] width
    //!           The width of input surface
    //! \param    [in] height
    //!           The height of input surface
    //! \return   uint32_t
    //!
    uint32_t GetAvsLineBufferSize(bool lineTiledBuffer, bool b8tapChromafiltering, uint32_t width, uint32_t height);

    //!
    //! \brief    Get Ief line buffer size
    //! \details  Get Ief line buffer size according to height of scaled surface
    //! \param    [in] lineTiledBuffer
    //!           ture if ief line tile buffer, otherwise, ief line buffer.
    //! \param    [in] heightOutput
    //!           The height of output surface
    //! \return   uint32_t
    //!
    uint32_t GetIefLineBufferSize(bool lineTiledBuffer, uint32_t heightOutput);

    //!
    //! \brief    Get Sfd line buffer size
    //! \details  Get Sfd line buffer size according to height of scaled surface
    //! \param    [in] lineTiledBuffer
    //!           ture if sdf line tile buffer, otherwise, sdf line buffer.
    //! \param    [in] formatOutput
    //!           format of output surface.
    //! \param    [in] widthOutput
    //!           The width of input surface
    //! \param    [in] heightOutput
    //!           The height of input surface
    //! \return   uint32_t
    //!
    virtual uint32_t GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput) override;

    //!
    //! \brief    Allocate Resources for SFC Pipe
    //! \details  Allocate the AVS and IEF line buffer surfaces for SFC
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateResources() override;

    virtual MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        PMHW_SFC_LOCK_PARAMS    pSfcLockParams);

    virtual MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER            pCmdBuffer,
        PMHW_SFC_STATE_PARAMS          pSfcState,
        PMHW_SFC_OUT_SURFACE_PARAMS    pOutSurface);

    virtual MOS_STATUS AddSfcAvsState(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        PMHW_SFC_AVS_STATE  pSfcAvsStateParams);

    virtual MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER       pCmdBuffer,
        PMHW_SFC_IEF_STATE_PARAMS pSfcIefStateParams);

    virtual MOS_STATUS AddSfcAvsLumaTable(
            PMOS_COMMAND_BUFFER     pCmdBuffer,
            PMHW_SFC_AVS_LUMA_TABLE pLumaTable);

    virtual MOS_STATUS AddSfcAvsChromaTable(
            PMOS_COMMAND_BUFFER       pCmdBuffer,
            PMHW_SFC_AVS_CHROMA_TABLE pChromaTable);

    virtual MOS_STATUS AddSfcFrameStart(
            PMOS_COMMAND_BUFFER pCmdBuffer,
            uint8_t             sfcPipeMode) override;

    virtual MOS_STATUS SetSfcAVSScalingMode(
        MHW_SCALING_MODE  ScalingMode) override;

    virtual MOS_STATUS SetSfcSamplerTable(
        PMHW_SFC_AVS_LUMA_TABLE         pLumaTable,
        PMHW_SFC_AVS_CHROMA_TABLE       pChromaTable,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting,
        bool                            bUse8x8Filter,
        float                           fHPStrength,
        float                           fLanczosT);

protected:

    PMHW_SFC_STATE_PARAMS           m_sfcStateParamsLegacy = nullptr;  //!< Pointer to sfc state parameters

    MHW_SFC_IEF_STATE_PARAMS        m_IefStateParamsLegacy = {};  //!< IEF Params state
    // HW intface to access MHW
    PMHW_SFC_INTERFACE              m_sfcInterface         = nullptr;
    PMHW_MI_INTERFACE               m_miInterface          = nullptr;

    VP_SFC_RENDER_DATA_LEGACY       m_renderDataLegacy     = {};       //!< Transient Render data populated for every BLT call
    VPHAL_SFC_AVS_STATE_LEGACY      m_avsStateLegacy       = {};                          //!< AVS State and Coeff. table

MEDIA_CLASS_DEFINE_END(vp__SfcRenderBaseLegacy)
};

}
#endif // !__VP_RENDER_SFC_BASE_LEGACY_H__
