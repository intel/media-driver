/*
* Copyright (c) 2015-2019, Intel Corporation
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
//! \file      mhw_vebox_g12_X.h
//! \brief     Defines functions for constructing vebox commands on Gen12-based platforms

#ifndef __MHW_VEBOX_G12_X_H__
#define __MHW_VEBOX_G12_X_H__

#include "mhw_vebox_generic.h"
#include "mhw_vebox_hwcmd_g12_X.h"
#include "renderhal.h"

#define MHW_VEBOX_TIMESTAMP_PER_TICK_IN_NS_G12              83.333f

#define MHW_LACE_COLOR_COMPENSATION_LUT_POINT_NUMBER            16

typedef struct _MHW_LACE_COLOR_WEIGHT_LUT
{
    int32_t iPoint[MHW_LACE_COLOR_COMPENSATION_LUT_POINT_NUMBER];        // U5.3
    int32_t iBias[MHW_LACE_COLOR_COMPENSATION_LUT_POINT_NUMBER];        // U5.8
    int32_t iSlope[MHW_LACE_COLOR_COMPENSATION_LUT_POINT_NUMBER];        // U1.10
} MHW_LACE_COLOR_WEIGHT_LUT;

typedef struct _MHW_LACE_COLOR_CORRECTION
{
    bool                        bColorCorrectionEnable;         // Color correction enable from Gen12;
    bool                        bYUVFullRange;                  // Color correction need YUV offset from Gen12
    float                       fColorCompensationPowerFactor;  // Color Compensation Power Factor from Gen12
    MHW_LACE_COLOR_WEIGHT_LUT   colorWeightLut;
} MHW_LACE_COLOR_CORRECTION;

#if (_DEBUG || _RELEASE_INTERNAL)
#define MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(keyval, VDId, shift, mask, bUseVD) \
do\
{\
    int32_t TmpVal = keyval;\
    while (TmpVal != 0) \
    {\
        if (((TmpVal) & (mask)) == (VDId))\
        {\
            bUseVD = true;\
            break;\
        }\
        TmpVal >>= (shift);\
    };\
}while(0)
#endif

const MHW_VEBOX_SETTINGS g_Vebox_Settings_g12 =
{
    MHW_MAX_VEBOX_STATES,                                                     //!< uiNumInstances
    MHW_SYNC_SIZE,                                                            //!< uiSyncSize
    MHW_PAGE_SIZE,                                                            //!< uiDndiStateSize
    MHW_PAGE_SIZE,                                                            //!< uiIecpStateSize
    MHW_PAGE_SIZE * 2,                                                        //!< uiGamutStateSize
    MHW_PAGE_SIZE,                                                            //!< uiVertexTableSize
    MHW_PAGE_SIZE,                                                            //!< uiCapturePipeStateSize
    MHW_PAGE_SIZE * 2,                                                        //!< uiGammaCorrectionStateSize
    0,                                                                        //!< ui3DLUTSize
    0                                                                         //!< uiHdrStateSize
};

class MhwVeboxInterfaceG12 : public MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>
{
public:
    // Chroma parameters
    typedef struct _MHW_VEBOX_CHROMA_PARAMS
    {
        uint32_t  dwPixRangeThresholdChromaU[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeWeightChromaU[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeThresholdChromaV[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwPixRangeWeightChromaV[MHW_PIXRANGETHRES_NUM];
        uint32_t  dwHotPixelThresholdChromaU;
        uint32_t  dwHotPixelCountChromaU;
        uint32_t  dwHotPixelThresholdChromaV;
        uint32_t  dwHotPixelCountChromaV;
    }MHW_VEBOX_CHROMA_PARAMS;

    MhwVeboxInterfaceG12(
        PMOS_INTERFACE pInputInterface);

    virtual ~MhwVeboxInterfaceG12() { MHW_FUNCTION_ENTER; }

    MOS_STATUS VeboxAdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
        uint32_t                  *pdwSurfaceWidth,
        uint32_t                  *pdwSurfaceHeight,
        bool                      bDIEnable);

    MOS_STATUS AddVeboxState(
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams,
        bool                        bUseCmBuffer);

    MOS_STATUS AddVeboxDiIecp(
        PMOS_COMMAND_BUFFER           pCmdBuffer,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams);

    MOS_STATUS AddVeboxDndiState(
        PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams);

    MOS_STATUS AddVeboxIecpState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

    MOS_STATUS AddVeboxGamutState(
        PMHW_VEBOX_IECP_PARAMS  pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS pVeboxGamutParams);

    MOS_STATUS FindVeboxGpuNodeToUse(
        PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit);

    MOS_STATUS AddVeboxIecpAceState(
        PMHW_VEBOX_IECP_PARAMS pVeboxIecpParams);

    MOS_STATUS SetVeboxIecpStateSTE(
        mhw_vebox_g12_X::VEBOX_STD_STE_STATE_CMD *pVeboxStdSteState,
        PMHW_COLORPIPE_PARAMS                     pColorPipeParams)
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL(pVeboxStdSteState);
        MHW_CHK_NULL(pColorPipeParams);

        MhwVeboxInterfaceGeneric<mhw_vebox_g12_X>::SetVeboxIecpStateSTE(pVeboxStdSteState, pColorPipeParams);
        // Enable Skin Score Output surface to be written by Vebox
        pVeboxStdSteState->DW1.StdScoreOutput = pColorPipeParams->bEnableLACE && pColorPipeParams->LaceParams.bSTD;

    finish:
        return eStatus;
    }

    //!
    //! \brief    Set which vebox can be used by HW
    //! \details  VPHAL set whehter the VEBOX can be use by HW
    //! \param    [in] inputVebox0;
    //!           set whether Vebox0 can be used by HW
    //! \param    [in] inputVebox01;
    //!           set whether vebox1 can be use by HW
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxInUse(
        bool inputVebox0,
        bool inputVebox1);

    //!
    //! \brief    Set which vebox can be used by DNDI
    //! \details  VPHAL set Chroma parmaters can be use by DNDI
    //! \param    [in] chromaParams
    //!           Chroma parameter which will be used by DNDI
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxChromaParams(
        MHW_VEBOX_CHROMA_PARAMS *chromaParams);

    //! \brief    get the status for vebox's Scalability.
    //! \details  VPHAL will check whether the veobx support Scalabiltiy. 
    //! \return   bool
    //!           false or true
    bool IsScalabilitySupported();

    //!
    //! \brief    Add Vebox Surface Control Bits
    //! \details  Add Vebox Surface Control Bits
    //! \param    [in] pVeboxSurfCntlParams
    //!           Vebox Surface Control Parameters
    //! \param    [in] pSurfCtrlBits
    //!           Vebox Surface Control Control Bits
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS AddVeboxSurfaceControlBits(
        PMHW_VEBOX_SURFACE_CNTL_PARAMS pVeboxSurfCntlParams,
        uint32_t                       *pSurfCtrlBits);

    //!
    //! \brief    Add Vebox Tiling Convert Control Bits
    //! \details  Add Vebox Tiling Convert Control Bits
    //! \param    [in] cmdBuffer
    //!           Pointers to the HW Cmd buffer
    //! \param    [in] inSurParams
    //!           Pointer to input vebox surface params
    //! \param    [in] outSurParams
    //!           Pointer to output vebox surface params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    virtual MOS_STATUS AddVeboxTilingConvert(
        PMOS_COMMAND_BUFFER cmdBuffer, 
        PMHW_VEBOX_SURFACE_PARAMS        inSurParams,
        PMHW_VEBOX_SURFACE_PARAMS        outSurParams);

    //!
    //! \brief    Set Vebox Lace Color Parameters
    //! \details  VPHAL Set Vebox Lace Color Parameters
    //! \param    [in] laceParams
    //!           Lace parameter which will be used by Lace color correction
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS SetVeboxLaceColorParams(
        MHW_LACE_COLOR_CORRECTION *pLaceParams);

    //!
    //! \brief    Set Vebox Prolog CMD
    //! \details  VPHAL Set Vebox Prolog Cmd for MMC
    //! \param    [in] mhwMiInterface
    //!           Pointers to the HW Mi interface
    //! \param    [in] cmdBuffer
    //!           Pointers to the HW Cmd buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    MOS_STATUS setVeboxPrologCmd(
        PMHW_MI_INTERFACE            mhwMiInterface,
        PMOS_COMMAND_BUFFER          cmdBuffer);

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
        MOS_GPU_NODE    VeboxGpuNode);

private:
    uint32_t m_BT2020InvPixelValue[256];
    uint32_t m_BT2020FwdPixelValue[256];
    uint32_t m_BT2020InvGammaLUT[256];
    uint32_t m_BT2020FwdGammaLUT[256];
    bool     m_vebox0InUse;
    bool     m_vebox1InUse;
    bool     m_veboxScalabilitySupported;

    uint32_t                        m_veboxSplitRatio;
    MHW_VEBOX_CHROMA_PARAMS         m_chromaParams;
    MHW_LACE_COLOR_CORRECTION       m_laceColorCorrection;

    //!
    //! \brief      Set Vebox Iecp State Back-End CSC
    //! \details    Set Back-End CSC part of the VEBOX IECP States
    //! \param      [in] pVeboxIecpState
    //!             Pointer to VEBOX IECP States
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \param      [in] bEnableFECSC
    //!             Flag to enable FECSC
    //! \return     void
    //!
    void SetVeboxIecpStateBecsc(
        mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD *pVeboxIecpState,
        PMHW_VEBOX_IECP_PARAMS                 pVeboxIecpParams,
        bool                                   bEnableFECSC);
    //!
    //! \brief      Manual mode H2S
    //! \details    Manual mode H2S 
    //              Based on WW23'16 algorithm design.
    //! \param      [in] pVeboxIecpParams
    //!             Pointer to VEBOX IECP State Params
    //! \param      [in] pVeboxGamutParams
    //!             Pointer to VEBOX Gamut State Params
    //! \return     MOS_STATUS 
    //!
    MOS_STATUS VeboxInterface_H2SManualMode(
        PMHW_VEBOX_HEAP             pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams);

    //!
    //! \brief    Back end CSC setting, BT2020 YUV Full/Limited to BT2020 RGB Full (G10 +)
    //! \details  Back end CSC setting, BT2020 YUV Full/Limited to BT2020 RGB Full
    //! \param    [in] pVeboxIecpParams
    //!           Pointer to VEBOX IECP State Params
    //! \param    [in] pVeboxGamutParams
    //!           Pointer to VEBOX Gamut State Params
    //! \return   MOS_STATUS
    //!
    MOS_STATUS VeboxInterface_BT2020YUVToRGB(
        PMHW_VEBOX_HEAP             pVeboxHeapInput,
        PMHW_VEBOX_IECP_PARAMS      pVeboxIecpParams,
        PMHW_VEBOX_GAMUT_PARAMS     pVeboxGamutParams);

    //!
    //! \brief      init VEBOX IECP States
    //! \details    init Vebox IECP states STD/E, ACE, TCC, Gamut 
    //! \param      [in] pVeboxIecpStateCmd
    //!             Pointer to Vebox Interface Structure
    //! \return     viod
    //!
    void IecpStateInitialization(
        mhw_vebox_g12_X::VEBOX_IECP_STATE_CMD    *pVeboxIecpState);

    MOS_STATUS AdjustBoundary(
        PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
        uint32_t                  *pdwSurfaceWidth,
        uint32_t                  *pdwSurfaceHeight,
        bool                      bDIEnable);

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Validate the GPU Node to use for Vebox
    //! \details  Validate the GPU Node to create gpu context used by vebox
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ValidateVeboxScalabilityConfig();
#endif

    void SetVeboxSurfaces(
        PMHW_VEBOX_SURFACE_PARAMS                 pSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                 pDerivedSurfaceParam,
        PMHW_VEBOX_SURFACE_PARAMS                 pSkinScoreSurfaceParam,
        mhw_vebox_g12_X::VEBOX_SURFACE_STATE_CMD *pVeboxSurfaceState,
        bool                                      bIsOutputSurface,
        bool                                      bDIEnable);
};

#endif
