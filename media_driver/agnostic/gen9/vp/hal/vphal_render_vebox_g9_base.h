/*
* Copyright (c) 2011-2017, Intel Corporation
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
//! \file     vphal_render_vebox_g9_base.h
//! \brief    Interface and structure specific for SKL (GEN9) Vebox
//! \details  Interface and structure specific for SKL (GEN9) Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_G9_BASE_H__
#define __VPHAL_RENDER_VEBOX_G9_BASE_H__

#include "vphal_render_vebox_base.h"

#define VPHAL_VEBOX_MAX_SLICES_G9                               2

#define VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_G9                       (VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                                 VPHAL_NUM_RGB_CHANNEL                    * \
                                                                 VPHAL_VEBOX_MAX_SLICES_G9)
//!
//! \brief Noise Detection Definitions for SKL+
//!
#define NOISE_HIGH_NOISE_LEVEL_G9                               450

//!
//! \brief Temporal Denoise Definitions for SKL+
//!
#define NOISE_HISTORY_MAX_DEFAULT_G9                            208
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_LOW_G9                  0
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_HIGH_G9                 2
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_G9              2
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G9      4
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G9     8
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_LOW_G9         10
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G9        14
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G9        128
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G9       144

//!
//! \brief SKL+ added 4 LSB for ASD/STAD/SCM/LTDT/TDT, hence shifting 4 below.
//!
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_LOW_G9               (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_G9           (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_HIGH_G9              (40  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_LOW_G9          (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_G9      (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_HIGH_G9         (40  << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G9             (4   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G9         (8   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G9            (8   << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_LOW_G9                (8  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G9            (12  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G9               (12  << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G9               (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_G9           (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G9              (144 << 4)

//!
//! \brief Vebox Statistics Surface definition for SKL+
//!
#define VPHAL_VEBOX_STATISTICS_SIZE_G9                          (32 * 4)
#define VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G9                (32 * sizeof(uint32_t))
#define VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G9            0
#define VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G9            0x2C
#define VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G9            0x44

//!
//! \brief Vebox Histogram Surface definition for SKL
//!
#define VPHAL_VEBOX_ACE_HISTOGRAM_SLICE0_OFFSET_G9              0x1C00
#define VPHAL_VEBOX_ACE_HISTOGRAM_SLICE1_OFFSET_G9              0x2400

struct VEBOX_STATE_UPDATE_STATIC_DATA_G9
{
    // DWORD 0 - GRF R1.0
    union
    {
        // DN State Update
        struct
        {
            uint32_t       OffsetToSlice0;
        };

        uint32_t       Value;
    } DW00;

    // DWORD 1 - GRF R1.1
    union
    {
        // DN State Update
        struct
        {
            uint32_t       OffsetToSlice1;
        };

        uint32_t       Value;
    } DW01;

    // DWORD 2 - GRF R1.2
    union
    {
        // DN State Update
        struct
        {
            uint32_t       FirstFrameFlag : 16;
            uint32_t       NoiseLevel : 16;
        };

        uint32_t   Value;
    } DW02;

    // DWORD 3 - GRF R1.3
    union
    {
        // RangeThr Adp2NLvl: 1 ifenabled, 0 otherwise
        struct
        {
            uint32_t       RangeThrAdp2NLvl : 16;
            uint32_t       reserved : 16;
        };

        uint32_t   Value;
    } DW03;

    // DWORD 4 - GRF R1.4
    union
    {
        // Vebox Statistics Surface
        struct
        {
            uint32_t       VeboxStatisticsSurface;
        };

        uint32_t   Value;
    } DW04;

    // DWORD 5 - GRF R1.5
    union
    {
        // Vebox DnDi State Surface
        struct
        {
            uint32_t       VeboxDndiStateSurface;
        };

        uint32_t   Value;
    } DW05;

    // DWORD 6 - GRF R1.6
    union
    {
        // Vebox GNE surface
        struct
        {
            uint32_t       VeboxTempSurface;
        };

        uint32_t   Value;
    } DW06;

    // DWORD 7 - GRF R1.7
    union
    {
        // Vebox Spatial Attributes Configuration Surface
        struct
        {
            uint32_t       VeboxSpatialAttributesConfigurationSurface;
        };

        uint32_t   Value;
    } DW07;
};

typedef class VPHAL_VEBOX_STATE_G9_BASE *PVPHAL_VEBOX_STATE_G9_BASE;
class VPHAL_VEBOX_STATE_G9_BASE:virtual public VPHAL_VEBOX_STATE
{
public:
    VPHAL_VEBOX_STATE_G9_BASE(
        PMOS_INTERFACE                  pOsInterface,
        PMHW_VEBOX_INTERFACE            pVeboxInterface,
        PMHW_SFC_INTERFACE              pSfcInterface,
        PRENDERHAL_INTERFACE            pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA           pPerfData,
        const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
        MOS_STATUS                      *peStatus);

    virtual                             ~VPHAL_VEBOX_STATE_G9_BASE() { }

    virtual MOS_STATUS AllocateResources();

    virtual void FreeResources();

    virtual MOS_STATUS SetupVeboxKernel(
        int32_t                     iKDTIndex);

    virtual MOS_STATUS SetupDiIecpState(
        bool                        bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS
        pVeboxDiIecpCmdParams);

    virtual void SetupSurfaceStates(
        bool                        bDiVarianceEnable,
        PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS
        pVeboxSurfaceStateCmdParams);

    virtual MOS_STATUS SetupVeboxState(
        bool                        bDiVarianceEnable,
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams);

    virtual bool IsNeeded(
        PCVPHAL_RENDER_PARAMS       pcRenderParams,
        RenderpassData              *pRenderPassData);

#if VEBOX_AUTO_DENOISE_SUPPORTED
    virtual MOS_STATUS LoadUpdateDenoiseKernelStaticData(
        int32_t*                        iCurbeOffsetOutDN);

    virtual MOS_STATUS SetupSurfaceStatesForDenoise();
#endif

    virtual bool IsFormatSupported(
        PVPHAL_SURFACE              pSrcSurface);

    virtual bool IsRTFormatSupported(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pRTSurface);

    virtual bool IsDnFormatSupported(
        PVPHAL_SURFACE                  pSrcSurface);

    virtual bool IsDiFormatSupported(
        PVPHAL_SURFACE              pSrcSurface);

    virtual VPHAL_OUTPUT_PIPE_MODE  GetOutputPipe(
        PCVPHAL_RENDER_PARAMS               pcRenderParams,
        PVPHAL_SURFACE                      pSrcSurface,
        bool*                               pbCompNeeded);

    virtual bool UseKernelResource();

    virtual void VeboxGetBeCSCMatrix(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface);

protected:
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
        bool                                    bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams);

    //!
    //! \brief    IsFormatMMCSupported
    //! \details  Check if the format of vebox output surface is supported by MMC
    //! \param    [in] Format
    //! \return   bool  true if suported, otherwise not supported
    //!
    virtual bool IsFormatMMCSupported(
        MOS_FORMAT                  Format);

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
    virtual MOS_STATUS      GetOutputSurfParams(
        MOS_FORMAT          &Format,
        MOS_TILE_TYPE       &TileType);

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
    virtual MOS_STATUS      GetFFDISurfParams(
        VPHAL_CSPACE        &ColorSpace,
        VPHAL_SAMPLE_TYPE   &SampleType);

    //!
    //! \brief    Check for DN only case
    //! \details  Check for DN only case
    //! \return   bool
    //!           Return true if DN only case, otherwise not
    //!
    virtual bool            IsDNOnly();

    //!
    //! \brief    Check whether FFDI Surf is needed
    //! \details  For decide whether FFDI surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool            IsFFDISurfNeeded();

    //!
    //! \brief    Check whether FFDN Surf is needed
    //! \details  For decide whether FFDN surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool            IsFFDNSurfNeeded();

    //!
    //! \brief    Check whether STMM Surf is needed
    //! \details  For decide whether STMM surf should be allocated or will be used
    //! \return   bool
    //!           Return true is needed, otherwise false
    //!
    virtual bool            IsSTMMSurfNeeded();

    //!
    //! \brief    Get output surface of Vebox
    //! \details  Get output surface of Vebox in current operation
    //! \param    bDiVarianceEnable
    //!           [in] Is DI/Variances report enabled
    //! \return   PVPHAL_SURFACE
    //!           Corresponding output surface pointer
    //!
    virtual PVPHAL_SURFACE GetSurfOutput(
        bool                                    bDiVarianceEnable);

    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE       QueryType,
        uint32_t*                   pQuery);

    //!
    //! \brief    Check if 2 passes CSC are supported on the platform
    //!
    virtual bool Is2PassesCscPlatformSupported()
    {
        return true;
    }

    virtual void GetLumaDefaultValue(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    virtual VphalSfcState* CreateSfcState();
};

#endif // __VPHAL_RENDER_VEBOX_G9_BASE_H__
