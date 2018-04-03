/*
* Copyright (c) 2011-2018, Intel Corporation
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
//! \file     vphal_render_vebox_g8_base.h
//! \brief    Interface and structure specific for BDW (GEN8) Vebox
//! \details  Interface and structure specific for BDW (GEN8) Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_G8_BASE_H__
#define __VPHAL_RENDER_VEBOX_G8_BASE_H__

#include "vphal_render_vebox_base.h"

//!
//! \brief Vebox Statistics Surface definition
//!
#define VPHAL_VEBOX_STATISTICS_SIZE_G8                 (288 * 4) // in DWORDs, twice per frame for 2 slices
#define VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G8       (288 * sizeof(uint32_t))
#define VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G8   0x400
#define VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G8   0x42C
#define VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G8   0x444

#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT      32
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT 32

struct VEBOX_STATE_UPDATE_STATIC_DATA_G8
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
            uint32_t       FirstFrameFlag              : 16;
            uint32_t       NoiseLevel                  : 16;
        };

        uint32_t   Value;
    } DW02;

    // DWORD 3 - GRF R1.3
    union
    {
        // Vebox Statistics Surface
        struct
        {
            uint32_t       VeboxStatisticsSurface;
        };

        uint32_t   Value;
    } DW03;

    // DWORD 4 - GRF R1.4
    union
    {
        // Vebox DnDi State Surface
        struct
        {
            uint32_t       VeboxDndiStateSurface;
        };

        uint32_t   Value;
    } DW04;

    // DWORD 5 - GRF R1.5
    union
    {
        // Vebox GNE surface
        struct
        {
            uint32_t       VeboxTempSurface;
        };

        uint32_t   Value;
    } DW05;
    uint32_t dwPad[2];
};

typedef class VPHAL_VEBOX_STATE_G8_BASE *PVPHAL_VEBOX_STATE_G8_BASE;
class VPHAL_VEBOX_STATE_G8_BASE :virtual public VPHAL_VEBOX_STATE
{
public:
    VPHAL_VEBOX_STATE_G8_BASE(
        PMOS_INTERFACE                  pOsInterface,
        PMHW_VEBOX_INTERFACE            pVeboxInterface,
        PRENDERHAL_INTERFACE            pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA           pPerfData,
        const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
        MOS_STATUS                      *peStatus);

    virtual                             ~VPHAL_VEBOX_STATE_G8_BASE() { }

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
    //!           [in/out] Pointer to VEBOX_DI_IECP command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupDiIecpStateForOutputSurf(
        bool                                    bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS           pVeboxDiIecpCmdParams);

    //!
    //! \brief    Get related surf parameters needed when allocate FFDI surface
    //! \details  Get related surf parameters needed when allocate FFDI surface
    //! \param    Format
    //!           [out] Format of FFDI surface
    //! \param    TileType
    //!           [out] Tile type of FFDI surface
    //! \param    ColorSpace
    //!           [out] Color space of FFDI surface
    //! \param    SampleType
    //!           [out] Sample type of FFDI surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if success, otherwise failed
    //!
    virtual MOS_STATUS GetFFDISurfParams(
        MOS_FORMAT          &Format,
        MOS_TILE_TYPE       &TileType,
        VPHAL_CSPACE        &ColorSpace,
        VPHAL_SAMPLE_TYPE   &SampleType);

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

    virtual MOS_STATUS VeboxQueryStatLayout(
        VEBOX_STAT_QUERY_TYPE       QueryType,
        uint32_t*                   pQuery);

    virtual void GetLumaDefaultValue(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    virtual VphalSfcState* CreateSfcState() { return nullptr; }
};

#endif // __VPHAL_RENDER_VEBOX_G8_BASE_H__
