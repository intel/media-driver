/*
* Copyright (c) 2010-2019, Intel Corporation
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
//! \file     vphal_render_sfc_base.h
//! \brief    The header file of the base class of VPHAL SFC rendering component
//! \details  The SFC renderer supports Scaling, IEF, CSC/ColorFill and Rotation.
//!           It's responsible for setting up HW states and generating the SFC
//!           commands.
//!
#ifndef __VPHAL_RENDER_SFC_BASE_H__
#define __VPHAL_RENDER_SFC_BASE_H__

#include "mhw_sfc.h"
#include "renderhal.h"
#include "vphal.h"
#include "vphal_render_common.h"
#include "vphal_render_vebox_iecp.h"

#ifndef __VPHAL_SFC_SUPPORTED
#define __VPHAL_SFC_SUPPORTED 1
#endif

//!
//! \brief  Structure to hold AVS Coeff tables
//!
struct VPHAL_SFC_AVS_STATE
{
    MHW_SFC_AVS_LUMA_TABLE      LumaCoeffs;
    MHW_SFC_AVS_CHROMA_TABLE    ChromaCoeffs;
    MHW_SFC_AVS_STATE           AvsStateParams;
};

//!
//! \brief Transient Render data populated for every BLT call
//!
typedef struct _VPHAL_SFC_RENDER_DATA
{
    bool                                bColorFill;                             //!< Enable ColorFill
    bool                                bScaling;                               //!< Enable Scaling
    bool                                bIEF;                                   //!< Enable IEF filter
    bool                                bCSC;                                   //!< Enable CSC filter

    float                               fScaleX;                                //!< X Scaling ratio
    float                               fScaleY;                                //!< Y Scaling ratio
    uint16_t                            wIEFFactor;                             //!< IEF factor
    MHW_SFC_STATE_PARAMS                *SfcStateParams;                        //!< Pointer to SFC state params
    MHW_SFC_IEF_STATE_PARAMS            IEFStateParams;                         //!< Pointer to IEF state params
    PMHW_AVS_PARAMS                     pAvsParams;                             //!< Pointer to AVS params
    PVPHAL_COLORFILL_PARAMS             pColorFillParams;                       //!< Pointer to ColorFill params
    PVPHAL_ALPHA_PARAMS                 pAlphaParams;                           //!< Pointer to Alpha params
    VPHAL_CSPACE                        SfcInputCspace;                         //!< SFC Input Color Space
    MOS_FORMAT                          SfcInputFormat;                         //!< SFC Input Format
    VPHAL_ROTATION                      SfcRotation;                            //!< SFC Rotation Mode
    VPHAL_SCALING_MODE                  SfcScalingMode;                         //!< SFC Scaling Mode
    uint32_t                            SfcSrcChromaSiting;                     //!< SFC Source Surface Chroma Siting

    PVPHAL_SURFACE                      pSfcPipeOutSurface;                     //!< SFC Pipe output surface

    bool                                bForcePolyPhaseCoefs;                   //!< SFC AVS force polyphase coef
} VPHAL_SFC_RENDER_DATA, *PVPHAL_SFC_RENDER_DATA;

#if __VPHAL_SFC_SUPPORTED
class VphalSfcState
{
public:
    //!
    //! \brief    VphalSfcState constructor
    //! \param    [in] osInterface
    //!           Pointer to os interface
    //! \param    [in] renderHal
    //!           pointer to renderhal structure
    //! \param    [in] sfcInterface
    //!           pointer to mhw sfc interface
    //! \return   void
    //!
    VphalSfcState(PMOS_INTERFACE       osInterface,
                  PRENDERHAL_INTERFACE renderHal,
                  PMHW_SFC_INTERFACE   sfcInterface);
    //!
    //! \brief    VphalSfcState destructor
    //! \return   void
    //!
    virtual ~VphalSfcState();

    //!
    //! \brief    Disable the SFC functionality
    //! \param    [in] disable
    //!           true to disable, false to enable SFC
    //! \return   void
    //!
    void SetDisable(bool disable)
    {
        disableSFC = disable;
    }

    //!
    //! \brief    Set SFC MMC status
    //! \param    [in] enable
    //!           true to disable, false to enable SFC MMC
    //! \return   void
    //!
    void SetSfcOutputMmcStatus(bool enable)
    {
        enableSfcMMC = enable;
    }

    //!
    //! \brief    Check if SFC functionality is disabled
    //! \return   true if disabled, false if enabled
    //!
    bool IsDisabled()
    {
        return disableSFC;
    }

    //!
    //! \brief    Check if SFC MMC is enabled
    //! \return   true if enabled, false if disabled
    //!
    bool IsSfcMmcEnabled()
    {
        return enableSfcMMC;
    }

    //!
    //! \brief    Set stero channel of input
    //! \param    [in] currentChannel
    //!           Current stero channel
    //! \return   void
    //!
    void SetStereoChannel(uint32_t currentChannel)
    {
        m_currentChannel = currentChannel;
    }

    //!
    //! \brief    Get Sfc's input format
    //! \return   MOS_FORMAT
    //!
    MOS_FORMAT GetInputFormat()
    {
        return m_renderData.SfcInputFormat;
    }

    //!
    //! \brief    Get Sfc's input color space
    //! \return   VPHAL_CSPACE
    //!
    VPHAL_CSPACE GetInputColorSpace()
    {
        return m_renderData.SfcInputCspace;
    }

    //!
    //! \brief    Initialize sfc render data
    //! \return   void
    //!
    virtual void InitRenderData()
    {
        MOS_FreeMemory(m_renderData.SfcStateParams);
        m_renderData = { };
    }

    //!
    //! \brief    Get the output pipe considering the SFC restrictions
    //! \details  Check which output pipe can be applied, SFC or Comp
    //! \param    [in] pSrc
    //!           Pointer to input surface of SFC
    //! \param    [in] pRenderTarget
    //!           Pointer to RenderTarget
    //! \param    [in] pcRenderParams
    //!           Pointer to VpHal render parameters
    //! \return   the output pipe mode
    //!
    virtual VPHAL_OUTPUT_PIPE_MODE GetOutputPipe(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget,
        PCVPHAL_RENDER_PARAMS       pcRenderParams);

    //!
    //! \brief    Check if SFC is feasible to generate output
    //! \param    [in] pcRenderParams
    //!           Pointer to VpHal render parameters
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of SFC
    //! \param    [in] pRenderTarget
    //!           Pointer to RenderTarget
    //! \return   true if SFC is feasible
    //!
    virtual bool IsOutputPipeSfcFeasible(
        PCVPHAL_RENDER_PARAMS       pcRenderParams,
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Setup SFC Rendering Flags
    //! \details  Set up the SFC rendering flag based the input and render target
    //!           surface, SFC state and the ColorFill/Alpha parameters
    //! \param    [in] pColorFillParams
    //!           Pointer to ColorFill params
    //! \param    [in] pAlphaParams
    //!           Pointer to Alpha params
    //! \param    [in] pSrc
    //!           Pointer to Source Surface
    //! \param    [in] pRenderTarget
    //!           Pointer to Render Target Surface
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox render Data
    //! \return   void
    //!
    virtual void SetRenderingFlags(
        PVPHAL_COLORFILL_PARAMS         pColorFillParams,
        PVPHAL_ALPHA_PARAMS             pAlphaParams,
        PVPHAL_SURFACE                  pSrc,
        PVPHAL_SURFACE                  pRenderTarget,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData);

    //!
    //! \brief    Free resources used by SFC Pipe
    //! \details  Free the AVS and IEF line buffer surfaces for SFC
    //! \return   void
    //!
    virtual void FreeResources();

    //!
    //! \brief    Check to see if the i/o surface is supported by SFC
    //! \details  Check the format and the size of i/o surface and determine
    //!           whether it can go through SFC pipe
    //! \param    [in] pSrcSurface
    //!           Pointer to Src surface
    //! \param    [in] pOutSurface
    //!           Pointer to Output surface
    //! \param    [in] pAlphaParams
    //!           Pointer to Alpha params
    //! \return   true if SFC Pipe is needed else false
    //!
    virtual bool IsFormatSupported(
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface,
        PVPHAL_ALPHA_PARAMS         pAlphaParams);

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters including SFC State, AVS
    //!           and IEF parameters
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \param    [in] pOutSurface
    //!           Pointer to Output Surface
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render data
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupSfcState(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData);

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
        PVPHAL_VEBOX_RENDER_DATA        pRenderData,
        PMOS_COMMAND_BUFFER             pCmdBuffer);

    //!
    //! \brief    Set Sfc index used by HW
    //! \details  VPHAL set Sfc index used by HW
    //! \param    [in] dwSfcIndex;
    //!           set which Sfc can be used by HW
    //! \param    [in] dwSfcCount;
    //!           set Sfc Count
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS SetSfcIndex(
        uint32_t                    dwSfcIndex,
        uint32_t                    dwSfcCount)
    {
        MOS_UNUSED(dwSfcIndex);
        MOS_UNUSED(dwSfcCount);
        return MOS_STATUS_SUCCESS;
    }

protected:
    //!
    //! \brief      Gen specific function for SFC adjust boundary
    //! \details    Adjust the width and height of the input surface for SFC
    //! \param      [in] pSurface
    //!             Pointer to input Surface
    //! \param      [out] pdwSurfaceWidth
    //!             Pointer to adjusted surface width
    //! \param      [out] pdwSurfaceHeight
    //!             Pointer to adjusted surface height
    //! \return     void
    //!
    virtual void AdjustBoundary(
        PVPHAL_SURFACE              pSurface,
        uint32_t*                   pdwSurfaceWidth,
        uint32_t*                   pdwSurfaceHeight);

    //!
    //! \brief    Allocate Resources for SFC Pipe
    //! \details  Allocate the AVS and IEF line buffer surfaces for SFC
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Is the Format MMC Supported in SFC
    //! \details  Check whether input format is supported by memory compression
    //! \param    [in] Format
    //!           Surface format
    //! \return   true if supported, false if not
    //!
    virtual bool IsFormatMMCSupported(
        MOS_FORMAT                  Format);

    //!
    //! \brief    Check if SFC is capable to output
    //! \details  Check all conditions to see if it meets SFC's capablitiy
    //! \param    [in] isColorFill
    //!           indicates whether color fill is required
    //! \param    [in] src
    //!           Pointer to source surface
    //! \param    [in] renderTarget
    //!           Pointer to renderTarget surface
    //! \return   true if capable, false if not
    //!
    virtual bool IsOutputCapable(
        bool            isColorFill,
        PVPHAL_SURFACE  src,
        PVPHAL_SURFACE  renderTarget);

    //!
    //! \brief    Check if the input format is supported
    //! \param    [in] srcSurface
    //!           input surface
    //! \return   true if supported, false if not
    //!
    virtual bool IsInputFormatSupported(
        PVPHAL_SURFACE              srcSurface) = 0;

    //!
    //! \brief    Check if the output format is supported
    //! \param    [in] outSurface
    //!           output surface
    //! \return   true if supported, false if not
    //!
    virtual bool IsOutputFormatSupported(
        PVPHAL_SURFACE              outSurface) = 0;

    //!
    //! \brief    Get width and height align unit of input format
    //! \param    [in] inputFormat
    //!           input format
    //! \param    [in] outputFormat
    //!           output format
    //! \param    [out] widthAlignUnit
    //!           width align unit
    //! \param    [out] heightAlignUnit
    //!           height align unit
    //! \return   void
    //!
    virtual void GetInputWidthHeightAlignUnit(
        MOS_FORMAT              inputFormat,
        MOS_FORMAT              outputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit) = 0;

    //!
    //! \brief    Get width and height align unit of output format
    //! \param    [in] outputFormat
    //!           output format
    //! \param    [out] widthAlignUnit
    //!           width align unit
    //! \param    [out] heightAlignUnit
    //!           height align unit
    //! \return   void
    //!
    virtual void GetOutputWidthHeightAlignUnit(
        MOS_FORMAT              outputFormat,
        uint16_t                &widthAlignUnit,
        uint16_t                &heightAlignUnit);

    //!
    //! \brief    Setup SFC State related parameters
    //! \details  Setup the SFC State related parameters based on the i/o surface
    //!           and Vebox and SFC rendering data
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render data
    //! \param    [in] pSrcSurface
    //!           Pointer to Input Source Surface
    //! \param    [in] pOutSurface
    //!           Pointer to Output Source Surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetSfcStateParams(
        PVPHAL_VEBOX_RENDER_DATA        pRenderData,
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface);

    //!
    //! \brief    Setup parameters related to SFC_IEF State
    //! \details  Setup the IEF and CSC params of the SFC_IEF State
    //! \param    [in] veboxRenderData
    //!           Pointer to Vebox Render Data
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [in] inputSurface
    //!           Pointer to Input Surface
    //! \return   void
    //!
    virtual void SetIefStateParams(
        PVPHAL_VEBOX_RENDER_DATA        veboxRenderData,
        PMHW_SFC_STATE_PARAMS           sfcStateParams,
        PVPHAL_SURFACE                  inputSurface);

    //!
    //! \brief    Setup CSC parameters of the SFC State
    //! \param    [in,out] pSfcStateParams
    //!           Pointer to SFC_STATE params
    //! \param    [out] pIEFStateParams
    //!           Pointer to MHW IEF state params
    //! \return   void
    //!
    virtual void SetIefStateCscParams(
        PMHW_SFC_STATE_PARAMS           pSfcStateParams,
        PMHW_SFC_IEF_STATE_PARAMS       pIEFStateParams);

    //!
    //! \brief    Update SFC Rendering Flags
    //! \details  Update the SFC rendering flag based the input and render target
    //!           surface, SFC state and the ColorFill/Alpha parameters
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \param    [in] pOutSurface
    //!           Pointer to Render Target Surface
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render Data
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS UpdateRenderingFlags(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData);

    //!
    //! \brief    Determine if CSC is required in SFC pipe and set related params
    //! \param    [in] src
    //!           Pointer to Source Surface
    //! \param    [in] renderTarget
    //!           Pointer to Target Surface
    //! \return   void
    //!
    virtual void DetermineCscParams(
        PVPHAL_SURFACE                  src,
        PVPHAL_SURFACE                  renderTarget);

    //!
    //! \brief    Determine SFC input surface format
    //! \details  Determine SFC input surface format according to
    //!           surface info and vebox flags
    //! \param    [in] src
    //!           Pointer to Source Surface
    //! \param    [in] veboxRenderData
    //!           Pointer to vebox Render Data
    //! \return   void
    //!
    virtual void DetermineInputFormat(
        PVPHAL_SURFACE                  src,
        PVPHAL_VEBOX_RENDER_DATA        veboxRenderData);

    //!
    //! \brief    Determine SFC input ordering mode
    //! \details  Determine SFC input ordering mode according to
    //!           vebox flags
    //! \param    [in] veboxRenderData
    //!           Pointer to vebox Render Data
    //! \param    [out] sfcStateParams
    //!           Pointer to SFC state params
    //! \return   void
    //!
    virtual void SetSfcStateInputOrderingMode(
        PVPHAL_VEBOX_RENDER_DATA    veboxRenderData,
        PMHW_SFC_STATE_PARAMS       sfcStateParams);

    //!
    //! \brief    Send SFC pipe commands
    //! \details  Register the surfaces and send the commands needed by SFC pipe
    //! \param    [in] pSfcInterface
    //!           Pointer to SFC interface
    //! \param    [in] pMhwMiInterface
    //!           Pointer to MI interface
    //! \param    [in] pOsInterface
    //!           Pointer to OS interface
    //! \param    [in] pSfcPipeOutSurface
    //!           Pointer to SFC Output surface
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS RenderSfcMmcCMD(
        PMHW_SFC_INTERFACE          pSfcInterface,
        MhwMiInterface            * pMhwMiInterface,
        PMOS_INTERFACE              pOsInterface,
        PMHW_SFC_OUT_SURFACE_PARAMS pSfcPipeOutSurface,
        PMOS_COMMAND_BUFFER         pCmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Set SFC MMC States
    //! \details  Update the SFC output MMC status
    //! \param    [in] renderData
    //!           Pointer to Render Data
    //! \param    [in] outSurface
    //!           Pointer to Sfc Output Surface
    //! \param    [in,out] sfcStateParams
    //!           Pointer to SFC State Params
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetSfcMmcStatus(
        PVPHAL_VEBOX_RENDER_DATA    renderData,
        PVPHAL_SURFACE              outSurface,
        PMHW_SFC_STATE_PARAMS       sfcStateParams);

private:

    //!
    //! \brief    Setup parameters related to SFC_AVS State
    //! \details  Setup the 8x8 table of SFC sampler
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SetAvsStateParams();

public:
    bool                            m_bSFC2Pass = false;

protected:
    bool                            disableSFC    = false;                      //!< Disable SFC for validation purposes
    bool                            enableSfcMMC  = false;                      //!< Memory compression enbale flag - read from User feature keys

    PMOS_INTERFACE                  m_osInterface;                              //!< Os Interface
    PRENDERHAL_INTERFACE            m_renderHal;                                //!< Pointer to RenderHal Structure Interface
    PMHW_SFC_INTERFACE              m_sfcInterface;                             //!< Pointer to SFC Structure Interface

                                                                                // AVS related params
    MHW_AVS_PARAMS                  m_AvsParameters       = {};                 //!< AVS parameters
    VPHAL_SFC_AVS_STATE             m_avsState            = {};                 //!< AVS State and Coeff. table

                                                                                // ColorFill CSC related params
    VPHAL_COLOR_SAMPLE_8            m_colorFillColorSrc   = {};                 //!< ColorFill Sample from DDI
    VPHAL_COLOR_SAMPLE_8            m_colorFillColorDst   = {};                 //!< ColorFill Sample programmed in Sfc State
    VPHAL_CSPACE                    m_colorFillSrcCspace  = {};                 //!< Cspace of the source ColorFill Color
    VPHAL_CSPACE                    m_colorFillRTCspace   = {};                 //!< Cspace of the Render Target

                                                                                // CSC related params
    VPHAL_CSPACE                    m_cscRTCspace      = {};                    //!< Cspace of Render Target
    VPHAL_CSPACE                    m_cscInputCspace   = {};                    //!< Cspace of input frame
    float                           m_cscCoeff[9]     = {};                     //!< [3x3] Coeff matrix
    float                           m_cscInOffset[3]  = {};                     //!< [3x1] Input Offset matrix
    float                           m_cscOutOffset[3] = {};                     //!< [3x1] Output Offset matrix

                                                                                // Surfaces used by SFC
    VPHAL_SURFACE                   m_AVSLineBufferSurface = {};                //!< AVS Line Buffer Surface for SFC
    VPHAL_SURFACE                   m_IEFLineBufferSurface = {};                //!< IEF Line Buffer Surface for SFC

                                                                                // Stereo state.
    uint32_t                        m_currentChannel = 0;                       //!< 0=StereoLeft or nonStereo, 1=StereoRight. N/A in nonStereo

    VPHAL_SFC_RENDER_DATA           m_renderData = {};                          //!< Transient Render data populated for every BLT call

};
#else
// A dummy class that does nothing when SFC is not supported

class VphalSfcState
{
public:
    VphalSfcState(
        PMOS_INTERFACE       osInterface,
        PRENDERHAL_INTERFACE renderHal,
        PMHW_SFC_INTERFACE   sfcInterface) {}
    virtual ~VphalSfcState() {}

    //!
    //! \brief    Disable the SFC functionality
    //! \param    [in] disable
    //!           true to disable, false to enable SFC
    //! \return   void
    void SetDisable(bool disable)
    {

    }

    //!
    //! \brief    Set SFC MMC status
    //! \param    [in] enable
    //!           true to disable, false to enable SFC MMC
    //! \return   void
    void SetSfcOutputMmcStatus(bool enable)
    {

    }
    //!
    //! \brief    Check if SFC functionality is disabled
    //! \return   true if disabled, false if enabled
    bool IsDisabled()
    {
        return true;
    }

    //!
    //! \brief    Set stero channel of input
    //! \param    [in] currentChannel
    //!           Current stero channel
    //! \return   void
    void SetStereoChannel(uint32_t currentChannel)
    {

    }

    MOS_FORMAT GetInputFormat()
    {
        return Format_Invalid;
    }

    VPHAL_CSPACE GetInputColorSpace()
    {
        return CSpace_None;
    }

    void InitRenderData()
    {
        {};
    }

    //!
    //! \brief    Get the output pipe considering the SFC restrictions
    //! \details  Check which output pipe can be applied, SFC or Comp
    //! \param    [in] pSrc
    //!           Pointer to input surface of SFC
    //! \param    [in] pRenderTarget
    //!           Pointer to RenderTarget
    //! \param    [in] pcRenderParams
    //!           Pointer to VpHal render parameters
    //! \return   the output pipe mode
    //!
    VPHAL_OUTPUT_PIPE_MODE GetOutputPipe(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget,
        PCVPHAL_RENDER_PARAMS       pcRenderParams)
    {
        return VPHAL_OUTPUT_PIPE_MODE_COMP;
    }

    //!
    //! \brief    Setup SFC Rendering Flags
    //! \details  Set up the SFC rendering flag based the input and render target
    //!           surface, SFC state and the ColorFill/Alpha parameters
    //! \param    [in] pColorFillParams
    //!           Pointer to ColorFill params
    //! \param    [in] pAlphaParams
    //!           Pointer to Alpha params
    //! \param    [in] pSrc
    //!           Pointer to Source Surface
    //! \param    [in] pRenderTarget
    //!           Pointer to Render Target Surface
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox render Data
    //! \return   void
    //!
    void SetRenderingFlags(
        PVPHAL_COLORFILL_PARAMS         pColorFillParams,
        PVPHAL_ALPHA_PARAMS             pAlphaParams,
        PVPHAL_SURFACE                  pSrc,
        PVPHAL_SURFACE                  pRenderTarget,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData)    {  }

    //!
    //! \brief    Free resources used by SFC Pipe
    //! \details  Free the AVS and IEF line buffer surfaces for SFC
    //! \return   void
    //!
    void FreeResources() { }

    //!
    //! \brief    Check to see if the i/o surface is supported by SFC
    //! \details  Check the format and the size of i/o surface and determine
    //!           whether it can go through SFC pipe
    //! \param    [in] pSrcSurface
    //!           Pointer to Src surface
    //! \param    [in] pOutSurface
    //!           Pointer to Output surface
    //! \param    [in] pAlphaParams
    //!           Pointer to Alpha params
    //! \return   true if SFC Pipe is needed else false
    //!
    bool IsFormatSupported(
        PVPHAL_SURFACE              pSrcSurface,
        PVPHAL_SURFACE              pOutSurface,
        PVPHAL_ALPHA_PARAMS         pAlphaParams) { return false;}

    //!
    //! \brief    Setup SFC states and parameters
    //! \details  Setup SFC states and parameters including SFC State, AVS
    //!           and IEF parameters
    //! \param    [in] pSrcSurface
    //!           Pointer to Source Surface
    //! \param    [in] pOutSurface
    //!           Pointer to Output Surface
    //! \param    [in,out] pRenderData
    //!           Pointer to Vebox Render data
    //! \return   Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetupSfcState(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface,
        PVPHAL_VEBOX_RENDER_DATA        pRenderData)    {return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Send SFC pipe commands
    //! \details  Register the surfaces and send the commands needed by SFC pipe
    //! \param    [in] pRenderData
    //!           Pointer to Vebox Render data
    //! \param    [in,out] pCmdBuffer
    //!           Pointer to command buffer
    //! \return   MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SendSfcCmd(
        PVPHAL_VEBOX_RENDER_DATA        pRenderData,
        PMOS_COMMAND_BUFFER             pCmdBuffer)   { return MOS_STATUS_SUCCESS;}
};
#endif

#endif // __VPHAL_RENDER_SFC_BASE_H__
