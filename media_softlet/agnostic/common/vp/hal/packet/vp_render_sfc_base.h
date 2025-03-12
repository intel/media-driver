/* Copyright (c) 2022-2024, Intel Corporation
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
//! \file     vp_render_sfc_base.h
//! \brief    The header file of the base class of SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!

#ifndef __VP_RENDER_SFC_BASE_H__
#define __VP_RENDER_SFC_BASE_H__

#include "vp_sfc_common.h"
#include "vp_vebox_common.h"
#include "vp_allocator.h"
#include "media_defs.h"
#include "mhw_sfc_itf.h"
#include "media_feature.h"

namespace vp {

class VpIef;

class SfcRenderBase
{

public:
    SfcRenderBase(VP_MHWINTERFACE &vpMhwinterface, PVpAllocator &allocator, bool disbaleSfcDithering);
    virtual ~SfcRenderBase();

    //!
    //! \brief    Initialize the object
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS Init();

    virtual MOS_STATUS Init(VIDEO_PARAMS& videoParams);

    //!
    //! \brief    Setup CSC parameters of the SFC State
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [out] iEFStateParams
    //!           MHW IEF state params
    //! \return   void
    //!
    virtual MOS_STATUS SetIefStateCscParams(
        mhw::sfc::SFC_STATE_PAR              *pSfcStateParams,
        mhw::sfc::SFC_IEF_STATE_PAR          *pIEFStateParams);

    //!
    //! \brief    Setup parameters related to SFC_IEF State
    //! \details  Setup the IEF and CSC params of the SFC_IEF State
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    virtual MOS_STATUS SetIefStateParams(
        mhw::sfc::SFC_STATE_PAR           *pSfcStateParams);

    //!
    //! \brief    Setup parameters related to SFC_AVS State
    //! \details  Setup the 8x8 table of SFC sampler
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetAvsStateParams();

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
        bool                            bOutputToMemory,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters including SFC State, AVS
    //!           and IEF parameters
    //! \param    [in] targetSurface
    //!           Pointer to Output Surface
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSfcState(PVP_SURFACE targetSurface);

    virtual MOS_STATUS UpdateCscParams(FeatureParamCsc &cscParams);

    //!
    //! \brief    check whether SFC Write have offset which may hit compresed write limitation
    //! \details  check whether SFC Write have offset which may hit compresed write limitation
    //! \param    [in] targetSurface
    //!           Pointer to targetSurface
    //! \return   the output pipe compression state
    //!
    virtual bool IsSFCUncompressedWriteNeeded(PVP_SURFACE targetSurface)
    {
        VP_FUNC_CALL();
        return false;
    }

    //!
    //! \brief    Set scaling parameters
    //! \details  Set scaling parameters
    //! \param    [in] scalingParams
    //!           Scaling parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams);

    //!
    //! \brief    Set csc parameters
    //! \details  Set csc parameters
    //! \param    [in] cscParams
    //!           Csc parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetCSCParams(PSFC_CSC_PARAMS cscParams);

    //!
    //! \brief    Set rotation and mirror parameters
    //! \details  Set rotation and mirror parameters
    //! \param    [in] rotMirParams
    //!           rotation and mirror parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams);

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
    virtual MOS_STATUS SetMmcParams(PMOS_SURFACE renderTarget, bool isFormatMmcSupported, bool isMmcEnabled);

    //!
    //! \brief    Check whether VDBOX-SFC Input Format Supported
    //! \details  Check whether VDBOX-SFC Input Format Supported.
    //! \param    codecStandard
    //!           [in] Codec Standard.
    //! \param    inputFormat
    //!           [in] Format of Input Frame
    //! \return   bool
    //!           Return true if supported, otherwise failed
    //!
    virtual bool IsVdboxSfcInputFormatSupported(
    CODECHAL_STANDARD           codecStandard,
    MOS_FORMAT                  inputFormat);

    //!
    //! \brief    Check whether VDBOX-SFC Output Format Supported
    //! \details  Check whether VDBOX-SFC Output Format Supported.
    //! \param    codecStandard
    //!           [in] Codec Standard.
    //! \param    outputFormat
    //!           [in] Format of Output Frame
    //! \return   bool
    //!           Return true if supported, otherwise failed
    //!
    virtual bool IsVdboxSfcOutputFormatSupported(
        CODECHAL_STANDARD           codecStandard,
        MOS_FORMAT                  outputFormat,
        MOS_TILE_TYPE               tileType);

    bool IsCSC() { return m_renderData.bCSC; }
    bool IsScaling() { return m_renderData.bScaling; }

    //!
    //! \brief    Get Sfc's input format
    //! \return   MOS_FORMAT
    //!
    MOS_FORMAT GetInputFormat()
    {
        return m_renderData.SfcInputFormat;
    }

    MOS_STATUS SetIefObj(VpIef *iefObj)
    {
        VP_PUBLIC_CHK_NULL_RETURN(iefObj);
        m_iefObj = iefObj;
        return MOS_STATUS_SUCCESS;
    }

    PVPHAL_IEF_PARAMS GetIefParams()
    {
        return m_renderData.pIefParams;
    }

    MOS_STATUS SetHistogramBuf(PMOS_BUFFER histogramBuf);

    //!
    //! \brief    Set sfc pipe selected with vebox
    //! \details  Set sfc pipe selected with vebox
    //! \param    [in] dwSfcPipe
    //!           Sfc pipe selected with vebox
    //! \param    [in] dwSfcNum
    //!           Sfc pipe num in total
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcPipe(uint32_t dwSfcPipe, uint32_t dwSfcNum)
    {
        MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
        MOS_UNUSED(dwSfcPipe);
        MOS_UNUSED(dwSfcNum);
        return eStatus;
    }

    virtual MOS_STATUS UpdateIefParams(PVPHAL_IEF_PARAMS iefParams);

protected:
    //!
    //! \brief    Initialize SfcState parameters
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS InitSfcStateParams() = 0;

    //!
    //! \brief    Set SFC input chroma subsampling
    //! \details  Set SFC input chroma subsampling according to
    //!           pipe mode
    //! \param    [out] sfcStateParams
    //!           Pointer to SFC state params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcStateInputChromaSubSampling(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Set SFC input ordering mode
    //! \details  SFC input ordering mode according to
    //!           pipe mode
    //! \param    [out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcStateInputOrderingMode(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeJpeg(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeVdbox(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);
    virtual MOS_STATUS SetSfcStateInputOrderingModeHcp(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Set codec pipe mode
    //! \details  Set codec pipe mode
    //! \param    [in] codecStandard
    //!           codec standard
    //! \return   MOS_STATUS
    virtual MOS_STATUS SetCodecPipeMode(CODECHAL_STANDARD codecStandard);
    //!
    //! \brief    Setup ColorFill parameters
    //! \details  Setup ColorFill parameters
    //! \param    [in] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    void SetColorFillParams(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Setup Rotation and Mirrow params
    //! \details  Setup Rotation and Mirrow params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetRotationAndMirrowParams(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Setup Chromasting params
    //! \details  Setup Chromasting params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetChromasitingParams(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Setup Bypass X & Y AdaptiveFilter params
    //! \details  Setup Bypass X & Y AdaptiveFilter params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetXYAdaptiveFilter(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Setup RGB Adaptive params
    //! \details  Setup RGB Adaptive params
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \return   void
    //!
    void SetRGBAdaptive(
        mhw::sfc::SFC_STATE_PAR       *pSfcStateParams);

    //!
    //! \brief    Initialize SFC Output Surface Command parameters
    //! \details  Initialize MHW SFC Output Surface Command parameters from SFC Pipe output Surface
    //! \param    [in] pSfcPipeOutSurface
    //!           pointer to SFC Pipe output Surface
    //! \param    [out] pMhwOutSurfParams
    //!           pointer to SFC Output Surface Command parameters
    //! \return   MOS_STATUS
    //!
    MOS_STATUS InitMhwOutSurfParams(
        PVP_SURFACE                     pSfcPipeOutSurface,
        PMHW_SFC_OUT_SURFACE_PARAMS     pMhwOutSurfParams);

    //!
    //! \brief    Initialize AVS parameters shared by Renderers
    //! \details  Initialize the members of the AVS parameter and allocate memory for its coefficient tables
    //! \param    [in,out] pAVS_Params
    //!           Pointer to MHW AVS parameter
    //! \param    [in] uiYCoeffTableSize
    //!           Size of the Y coefficient table
    //! \param    [in] uiUVCoeffTableSize
    //!           Size of the UV coefficient table
    //! \return   void
    //! 
    //!
    void InitAVSParams(
        PMHW_AVS_PARAMS     pAVS_Params,
        uint32_t            uiYCoeffTableSize,
        uint32_t            uiUVCoeffTableSize);

    //!
    //! \brief    Destroy AVS parameters shared by Renderers
    //! \details  Free the memory of AVS parameter's coefficient tables
    //! \param    [in,out] pAVS_Params
    //!           Pointer to VPHAL AVS parameter
    //! \return   void
    //!
    void DestroyAVSParams(
        PMHW_AVS_PARAMS   pAVS_Params);

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
    virtual uint32_t GetSfdLineBufferSize(bool lineTiledBuffer, MOS_FORMAT formatOutput, uint32_t widthOutput, uint32_t heightOutput);

    //!
    //! \brief    Allocate line buffer
    //! \details  Allocate line buffer
    //! \param    [in/out] lineBuffer
    //!           pointer to line buffer.
    //! \param    [in] size
    //!           size of line buffer.
    //! \param    [in] bufName
    //!           name of line buffer.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateLineBuffer(VP_SURFACE *&lineBuffer, uint32_t size, const char *bufName);

    //!
    //! \brief    Allocate line buffer array
    //! \details  Allocate line buffer
    //! \param    [in/out] lineBufferArray
    //!           pointer to line buffer.
    //! \param    [in] size
    //!           size of line buffer.
    //! \param    [in] bufName
    //!           name of line buffer.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS AllocateLineBufferArray(VP_SURFACE **&lineBufferArray, uint32_t size, const char *bufName);

    //!
    //! \brief    Destroy line buffer array
    //! \details  Allocate line buffer
    //! \param    [in/out] lineBufferArray
    //!           pointer to line buffer.
    //! \param    [in] count
    //!           count of line buffer.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS DestroyLineBufferArray(VP_SURFACE **&lineBufferArray, int32_t count);

    //!
    //! \brief    Allocate Resources for SFC Pipe
    //! \details  Allocate the AVS and IEF line buffer surfaces for SFC
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    SFC free resources
    //! \details  Free resources that are used in Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS FreeResources();

    virtual MOS_STATUS AddSfcLock(
        PMOS_COMMAND_BUFFER     pCmdBuffer,
        mhw::sfc::SFC_LOCK_PAR *pSfcLockParams);

    //!
    //! \brief    Set resource of line buffer
    //! \details  Set resource of line buffer
    //! \param    [out] osResLineBuffer
    //!           resource to be set
    //! \param    [in] lineBuffer
    //!           pointer to line buffer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetLineBuffer(PMOS_RESOURCE &osResLineBuffer, VP_SURFACE *lineBuffer);

    virtual MOS_STATUS SetupScalabilityParams() = 0;
    virtual bool IsInputChannelSwapNeeded(MOS_FORMAT inputFormat);
    virtual bool IsOutputChannelSwapNeeded(MOS_FORMAT outputFormat) = 0;
    virtual bool IsCscNeeded(SFC_CSC_PARAMS &cscParams) = 0;

    virtual MOS_STATUS AddSfcState(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        mhw::sfc::SFC_STATE_PAR     *pSfcStateParams,
        PMHW_SFC_OUT_SURFACE_PARAMS pOutSurface);

    virtual MOS_STATUS AddSfcAvsState(
        PMOS_COMMAND_BUFFER            pCmdBuffer);

    virtual MOS_STATUS AddSfcIefState(
        PMOS_COMMAND_BUFFER                 pCmdBuffer);

    virtual MOS_STATUS AddSfcAvsLumaTable(
        PMOS_COMMAND_BUFFER                    pCmdBuffer);

    virtual MOS_STATUS AddSfcAvsChromaTable(
        PMOS_COMMAND_BUFFER                      pCmdBuffer);

    virtual MOS_STATUS AddSfcFrameStart(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        uint8_t             sfcPipeMode);

    virtual MOS_STATUS SetSfcAVSScalingMode(
        MHW_SCALING_MODE  ScalingMode);

    virtual MOS_STATUS SetSfcSamplerTable(
        mhw::sfc::SFC_AVS_LUMA_Coeff_Table_PAR    *pLumaTable,
        mhw::sfc::SFC_AVS_CHROMA_Coeff_Table_PAR  *pChromaTable,
        PMHW_AVS_PARAMS                 pAvsParams,
        MOS_FORMAT                      SrcFormat,
        float                           fScaleX,
        float                           fScaleY,
        uint32_t                        dwChromaSiting,
        bool                            bUse8x8Filter,
        float                           fHPStrength,
        float                           fLanczosT);

protected:

    // HW intface to access MHW
    PMOS_INTERFACE                  m_osInterface  = nullptr;
    std::shared_ptr<mhw::sfc::Itf>  m_sfcItf       = nullptr;
    std::shared_ptr<mhw::mi::Itf>   m_miItf       = nullptr;
    MEDIA_FEATURE_TABLE             *m_skuTable = nullptr;
    MEDIA_WA_TABLE                  *m_waTable = nullptr;

    // AVS related params
    MHW_AVS_PARAMS                  m_AvsParameters = {};                     //!< AVS parameters
    VPHAL_SFC_AVS_STATE             m_avsState = {};                          //!< AVS State and Coeff. table
    static const uint32_t           k_YCoefficientTableSize = 256 * sizeof(int32_t);
    static const uint32_t           k_UVCoefficientTableSize = 128 * sizeof(int32_t);

    mhw::sfc::SFC_STATE_PAR         *m_sfcStateParams = nullptr;               //!< Pointer to sfc state parameters
    VP_SFC_RENDER_DATA              m_renderData = {};                        //!< Transient Render data populated for every BLT call
    //VP_SFC_RENDER_DATA              *m_renderData = nullptr;                        //!< Transient Render data populated for every BLT call

    VPHAL_CSPACE                    m_cscRTCspace = {};                       //!< Cspace of Render Target
    VPHAL_CSPACE                    m_cscInputCspace = {};                    //!< Cspace of input frame
    bool                            m_cscInputSwapNeeded  = false;

    mhw::sfc::SFC_IEF_STATE_PAR     m_IefStateParams = {};                    //!< IEF Params state
    float                           m_cscCoeff[9] = {};                       //!< [3x3] Coeff matrix
    float                           m_cscInOffset[3] = {};                    //!< [3x1] Input Offset matrix
    float                           m_cscOutOffset[3] = {};                   //!< [3x1] Output Offset matrix
    uint32_t                        m_currentChannel = 0;                     //!< 0=StereoLeft or nonStereo, 1=StereoRight. N/A in nonStereo

    VP_SURFACE                      **m_AVSLineBufferSurfaceArray = nullptr;  //!< AVS Line Buffer Surface for SFC
    VP_SURFACE                      **m_IEFLineBufferSurfaceArray = nullptr;  //!< IEF Line Buffer Surface for SFC
    VP_SURFACE                      **m_SFDLineBufferSurfaceArray = nullptr;  //!< SFD Line Buffer Surface for SFC

    VP_SURFACE                      *m_AVSLineTileBufferSurface = nullptr;    //!< AVS Line Tile Buffer Surface for SFC
    VP_SURFACE                      *m_IEFLineTileBufferSurface = nullptr;    //!< IEF Line Tile Buffer Surface for SFC
    VP_SURFACE                      *m_SFDLineTileBufferSurface = nullptr;    //!< SFD Line Tile Buffer Surface for SFC

    // Allocator interface
    PVpAllocator                    m_allocator = nullptr;                                //!< vp pipeline allocator
    VpIef                           *m_iefObj = nullptr;
    uint8_t                         m_pipeMode = MhwSfcInterface::SFC_PIPE_MODE_VEBOX; //!< which FE engine pipe used

    bool                            m_bVdboxToSfc = false;
    VIDEO_PARAMS                    m_videoConfig = {};

    int                             m_lineBufferAllocatedInArray    = 1;        //!< Line buffer allocated in array

    SCALABILITY_PARAMS              m_scalabilityParams             = {};

    MOS_SURFACE                     m_histogramSurf = {};   //!< Histogram buffer

    MHW_SFC_OUT_SURFACE_PARAMS      m_outSurfaceParam = {};

    bool                            m_disableSfcDithering = false;

    VP_SURFACE                      **m_AVSLineBufferSurfaceArrayfor1stPassofSfc2Pass = nullptr;  //!< AVS Line Buffer Surface for SFC 1st Pass of Sfc 2Pass
    VP_SURFACE                      **m_IEFLineBufferSurfaceArrayfor1stPassofSfc2Pass = nullptr;  //!< IEF Line Buffer Surface for SFC 1st Pass of Sfc 2Pass
    VP_SURFACE                      **m_SFDLineBufferSurfaceArrayfor1stPassofSfc2Pass = nullptr;  //!< SFD Line Buffer Surface for SFC 1st Pass of Sfc 2Pass
    int                             m_lineBufferAllocatedInArrayfor1stPassofSfc2Pass  = 1;        //!< Line buffer allocated in array for SFC 1st Pass of Sfc 2Pass

MEDIA_CLASS_DEFINE_END(vp__SfcRenderBase)
};


}
#endif // !__VP_RENDER_SFC_BASE_H__
