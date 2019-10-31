/*
* Copyright (c) 2011-2019, Intel Corporation
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
//! \file     vphal_render_vebox_g12_base.h
//! \brief    Interface and structure specific for GEN12 Vebox
//! \details  Interface and structure specific for GEN12 Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_G12_BASE_H__
#define __VPHAL_RENDER_VEBOX_G12_BASE_H__

#include "vphal_render_vebox_base.h"
#include "vphal_render_hdr_g11.h"
#include "vphal_common_hdr.h"

#define VPHAL_VEBOX_MAX_SLICES_G12                              4

#define VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_G12                      (VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                                 VPHAL_NUM_RGB_CHANNEL                    * \
                                                                 VPHAL_VEBOX_MAX_SLICES_G12)
#define VPHAL_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED_G12         (3072 * 4)
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

struct VEBOX_STATE_UPDATE_STATIC_DATA_G12
{
    // uint32_t 0 - GRF R1.0
    union
    {
        // DN State Update
        struct
        {
            uint32_t    OffsetToSlice0;
        };

        uint32_t    Value;
    } DW00;

    // uint32_t 1 - GRF R1.1
    union
    {
        // DN State Update
        struct
        {
            uint32_t    OffsetToSlice1;
        };

        uint32_t    Value;
    } DW01;

    // uint32_t 2 - GRF R1.2
    union
    {
        // DN State Update
        struct
        {
            uint32_t    OffsetToSlice2;
        };

        uint32_t    Value;
    } DW02;

    // uint32_t 3 - GRF R1.3
    union
    {
        // DN State Update
        struct
        {
            uint32_t    OffsetToSlice3;
        };

        uint32_t   Value;
    } DW03;

    // uint32_t 4 - GRF R1.4
    union
    {
        // DN State Update
        struct
        {
            uint32_t    FirstFrameFlag : 16;
            uint32_t    NoiseLevel : 16;
        };

        uint32_t    Value;
    } DW04;

    // uint32_t 5 - GRF R1.5
    union
    {
        // RangeThr Adp2NLvl: 1 if enabled, 0 otherwise
        struct
        {
            uint32_t    RangeThrAdp2NLvl : 16;
            uint32_t    Reserved : 16;
        };

        uint32_t    Value;
    } DW05;

    // uint32_t 6 - GRF R1.6
    union
    {
        // Vebox Statistics Surface
        struct
        {
            uint32_t    VeboxStatisticsSurface;
        };

        uint32_t     Value;
    } DW06;

    // uint32_t 7 - GRF R1.7
    union
    {
        // Vebox DnDi State Surface
        struct
        {
            uint32_t    VeboxDndiStateSurface;
        };

        uint32_t Value;
    } DW07;

    // uint32_t 8 - GRF R2.0
    union
    {
        // Vebox GNE surface
        struct
        {
            uint32_t    VeboxTempSurface;
        };

        uint32_t Value;
    } DW08;

    // uint32_t 9 - GRF R2.1
    union
    {
        // Vebox Spatial Attributes Configuration Surface
        struct
        {
            uint32_t    VeboxSpatialAttributesConfigurationSurface;
        };

        uint32_t Value;
    } DW09;
};

typedef class VPHAL_VEBOX_STATE_G12_BASE *PVPHAL_VEBOX_STATE_G12_BASE;
class VPHAL_VEBOX_STATE_G12_BASE:virtual public VPHAL_VEBOX_STATE
{
public:
    VPHAL_VEBOX_STATE_G12_BASE(
        PMOS_INTERFACE                 pOsInterface,
        PMHW_VEBOX_INTERFACE           pVeboxInterface,
        PMHW_SFC_INTERFACE             pSfcInterface,
        PRENDERHAL_INTERFACE           pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE        pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA          pPerfData,
        const VPHAL_DNDI_CACHE_CNTL    &dndiCacheCntl,
        MOS_STATUS                     *peStatus);

    virtual                             ~VPHAL_VEBOX_STATE_G12_BASE() { }

    virtual MOS_STATUS Initialize(
        const VphalSettings         *pSettings,
        Kdll_State                  *pKernelDllState);

    virtual MOS_STATUS AllocateResources();

    virtual void FreeResources();

    virtual MOS_STATUS SetupVeboxKernel(
        int32_t     iKDTIndex);

    virtual MOS_STATUS SetupDiIecpState(
        bool                            bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams);

    virtual void SetupSurfaceStates(
        bool                                    bDiVarianceEnable,
        PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS   pVeboxSurfaceStateCmdParams);

    virtual MOS_STATUS SetupVeboxState(
        bool                           bDiVarianceEnable,
        PMHW_VEBOX_STATE_CMD_PARAMS    pVeboxStateCmdParams);

    virtual bool IsNeeded(
        PCVPHAL_RENDER_PARAMS   pcRenderParams,
        RenderpassData         *pRenderPassData);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    virtual MOS_STATUS LoadUpdateDenoiseKernelStaticData(
        int32_t     *iCurbeOffsetOutDN);

    virtual MOS_STATUS SetupSurfaceStatesForDenoise();
#endif

    virtual bool IsFormatSupported(
        PVPHAL_SURFACE    pSrcSurface);

    virtual bool IsRTFormatSupported(
        PVPHAL_SURFACE    pSrcSurface,
        PVPHAL_SURFACE    pRTSurface);

    virtual bool IsDnFormatSupported(
        PVPHAL_SURFACE    pSrcSurface);

    virtual bool IsDiFormatSupported(
        PVPHAL_SURFACE    pSrcSurface);

    virtual bool UseKernelResource();

    virtual void VeboxGetBeCSCMatrix(
        PVPHAL_SURFACE    pSrcSurface,
        PVPHAL_SURFACE    pOutSurface);

protected:
    //!
    //! \brief    IsMMCEnabledForCurrOutputSurf
    //! \details  Check if MMC can be enabled for current output surface.
    //! \return   bool  true if suported, otherwise not supported
    //!
    virtual bool IsMMCEnabledForCurrOutputSurf();

    //!
    //! \brief    Setup Vebox_DI_IECP Command params for VEBOX final output surface on G75
    //! \details  Setup Vebox_DI_IECP Command params for VEBOX final output surface on G75
    //! \param    bDiScdEnable
    //!           [in] Is DI/Variances report enabled
    //! \param    pVeboxDiIecpCmdParams
    //!           [in,out] Pointer to VEBOX_DI_IECP command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupDiIecpStateForOutputSurf(
        bool                             bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS    pVeboxDiIecpCmdParams);

    //!
    //! \brief    IsFormatMMCSupported
    //! \details  Check if the format of vebox output surface is supported by MMC
    //! \param    [in] Format
    //! \return   bool  true if suported, otherwise not supported
    //!
    virtual bool IsFormatMMCSupported(
        MOS_FORMAT    Format);

    //!
    //! \brief    Get Output surface params needed when allocate surfaces
    //! \details  Get Output surface params needed when allocate surfaces
    //! \param    Format
    //!           [out] Format of output surface
    //! \param    TileType
    //!           [out] Tile type of output surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if success, otherwise failed
    //!
    virtual MOS_STATUS GetOutputSurfParams(
        MOS_FORMAT       &Format,
        MOS_TILE_TYPE    &TileType);

    //!
    //! \brief    Get related surf parameters needed when allocate FFDI surface
    //! \details  Get related surf parameters needed when allocate FFDI surface
    //! \param    ColorSpace
    //!           [out] Color space of FFDI surface
    //! \param    SampleType
    //!           [out] Sample type of FFDI surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if success, otherwise failed
    //!
    virtual MOS_STATUS GetFFDISurfParams(
        VPHAL_CSPACE         &ColorSpace,
        VPHAL_SAMPLE_TYPE    &SampleType);

    //!
    //! \brief    Check for DN only case
    //! \details  Check for DN only case
    //! \return   bool
    //!           Return true if DN only case, otherwise not
    //!
    virtual bool IsDNOnly();

    //!
    //! \brief    Check whether FFDI Surf is needed
    //! \details  For decide whether FFDI surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool IsFFDISurfNeeded();

    //!
    //! \brief    Check whether FFDN Surf is needed
    //! \details  For decide whether FFDN surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool IsFFDNSurfNeeded();

    //!
    //! \brief    Check whether STMM Surf is needed
    //! \details  For decide whether STMM surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool IsSTMMSurfNeeded();

    //!
    //! \brief    Get output surface of Vebox
    //! \details  Get output surface of Vebox in current operation
    //! \param    bDiVarianceEnable
    //!           [in] Is DI/Variances report enabled
    //! \return   PVPHAL_SURFACE
    //!           Corresponding output surface pointer
    //!
    virtual PVPHAL_SURFACE GetSurfOutput(
        bool    bDiVarianceEnable);

    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE    QueryType,
        uint32_t*                pQuery);

    //!
    //! \brief    Check if 2 passes CSC are supported on the platform
    //!
    virtual bool Is2PassesCscPlatformSupported()
    {
        return true;
    }

    virtual void GetLumaDefaultValue(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM     pLumaParams);

    //!
    //! \brief    Vebox set DN parameter
    //! \details  Set denoise paramters for luma and chroma.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pLumaParams
    //!           Pointer to Luma DN parameter
    //! \param    [in] pChromaParams
    //!           Pointer to Chroma DN parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDNParams(
        PVPHAL_SURFACE                     pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM    pLumaParams,
        PVPHAL_DNUV_PARAMS                 pChromaParams);

    //!
    //! \brief    Vebox set DI parameter
    //! \details  Set deinterlace paramters
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDIParams(
        PVPHAL_SURFACE    pSrcSurface);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                     pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM    pLumaParams,
        PVPHAL_DNUV_PARAMS                 pChromaParams);

    //!
    //! \brief    Setup Chroma Sampling for Vebox
    //! \details  Setup Chroma Sampling for use in the current Vebox Operation
    //! \param    [in] pChromaSampling
    //!           Pointer to chroma sampling params of Vebox
    //! \return   void
    //!
    void SetupChromaSampling(
        PMHW_VEBOX_CHROMA_SAMPLING    pChromaSampling);

    virtual VphalSfcState* CreateSfcState();

    //!
    //! \brief    Get Output Pipe
    //! \details  Get Output Pipe
    //! \param    [in] pcRenderParams
    //!           Pointer to Render parmas
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in/out] pRenderData
    //!           Pointer to Render data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual VPHAL_OUTPUT_PIPE_MODE  GetOutputPipe(
        PCVPHAL_RENDER_PARAMS           pcRenderParams,
        PVPHAL_SURFACE                  pSrcSurface,
        RenderpassData*                 pRenderData);

    //!
    //! \brief    Vebox Set Rendering Flags
    //! \details  Vebox Set Rendering Flags
    //! \param    [in] pSrc
    //!           Pointer to Input Surface
    //! \param    [in] pRenderTarget
    //!           Pointer to Output Surface
    //! \return   void
    //!
    void VeboxSetRenderingFlags(
        PVPHAL_SURFACE              pSrc,
        PVPHAL_SURFACE              pRenderTarget);

    //!
    //! \brief    Render the Vebox Cmd buffer for VeboxSendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [in,out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] VeboxDiIecpCmdParams
    //!           DiIecpCmd params struct to be set
    //! \param    [out] VeboxSurfaceStateCmdParams
    //!           VPHAL surface state cmd to be set
    //! \param    [out] MhwVeboxSurfaceStateCmdParams
    //!           MHW surface state cmd to be set
    //! \param    [out] VeboxStateCmdParams
    //!           MHW vebox state cmd to be set
    //! \param    [out] FlushDwParams
    //!           MHW MI_FLUSH_DW cmd to be set
    //! \param    [in] pGenericPrologParams
    //!           pointer to Generic prolog params struct to send to cmd buffer header
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxRenderVeboxCmd(
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
        VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
        MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

protected:
    VPHAL_SURFACE      Vebox3DLutOutputSurface   = {};
    Hdr3DLutGenerator  *m_hdr3DLutGenerator      = nullptr;
};

#endif // __VPHAL_RENDER_VEBOX_G12_BASE_H__
