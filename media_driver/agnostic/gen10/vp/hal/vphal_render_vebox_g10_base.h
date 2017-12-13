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
//! \file     vphal_render_vebox_g10_base.h
//! \brief    Interface and structure specific for GEN10 Vebox
//! \details  Interface and structure specific for GEN10 Vebox
//!
#ifndef __VPHAL_RENDER_VEBOX_G10_BASE_H__
#define __VPHAL_RENDER_VEBOX_G10_BASE_H__

#include "vphal_render_vebox_base.h"

#define VPHAL_VEBOX_MAX_SLICES_G10                              2

#define VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_G10                      (VPHAL_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                                 VPHAL_NUM_RGB_CHANNEL                    * \
                                                                 VPHAL_VEBOX_MAX_SLICES_G10)
//!
//! \brief Noise Detection Definitions for CNL
//!
#define NOISE_HIGH_NOISE_LEVEL_G10                               450

//!
//! \brief Temporal Denoise Definitions for CNL
//!
#define NOISE_HISTORY_MAX_DEFAULT_G10                            208
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_LOW_G10                  0 
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_HIGH_G10                 2
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT_G10              2
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G10      4
#define NOISE_CHROMA_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G10     8
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_LOW_G10         10
#define NOISE_CHROMA_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G10        14
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G10        128
#define NOISE_CHROMA_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G10       144

//!
//! \brief CNL added 4 LSB for ASD/STAD/SCM/LTDT/TDT, hence shifting 4 below.
//!
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_LOW_G10               (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_DEFAULT_G10           (32  << 4)
#define NOISE_ABSSUMTEMPORALDIFF_THRESHOLD_HIGH_G10              (40  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_LOW_G10          (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_DEFAULT_G10      (32  << 4)
#define NOISE_SPATIALCOMPLEXITYMATRIX_THRESHOLD_HIGH_G10         (40  << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_LOW_G10             (4   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G10         (8   << 4)
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_HIGH_G10            (8   << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_LOW_G10                (8  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT_G10            (12  << 4)
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_HIGH_G10               (12  << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_LOW_G10               (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT_G10           (128 << 4)
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_HIGH_G10              (144 << 4)

//!
//! \brief Improved Deinterlacing
//!
#define VPHAL_VEBOX_DI_CHROMA_TDM_WEIGHT_NATUAL                     0
#define VPHAL_VEBOX_DI_LUMA_TDM_WEIGHT_NATUAL                       4
#define VPHAL_VEBOX_DI_SHCM_DELTA_NATUAL                            5
#define VPHAL_VEBOX_DI_SHCM_THRESHOLD_NATUAL                        255
#define VPHAL_VEBOX_DI_SVCM_DELTA_NATUAL                            5
#define VPHAL_VEBOX_DI_SVCM_THRESHOLD_NATUAL                        255
#define VPHAL_VEBOX_DI_LUMA_TDM_CORING_THRESHOLD_NATUAL             0
#define VPHAL_VEBOX_DI_CHROMA_TDM_CORING_THRESHOLD_NATUAL           0
#define VPHAL_VEBOX_DI_DIRECTION_CHECK_THRESHOLD_NATUAL             3
#define VPHAL_VEBOX_DI_TEARING_LOW_THRESHOLD_NATUAL                 20
#define VPHAL_VEBOX_DI_TEARING_HIGH_THRESHOLD_NATUAL                100
#define VPHAL_VEBOX_DI_DIFF_CHECK_SLACK_THRESHOLD_NATUAL            15
#define VPHAL_VEBOX_DI_SAD_WT0_NATUAL                               0
#define VPHAL_VEBOX_DI_SAD_WT1_NATUAL                               63
#define VPHAL_VEBOX_DI_SAD_WT2_NATUAL                               76
#define VPHAL_VEBOX_DI_SAD_WT3_NATUAL                               89
#define VPHAL_VEBOX_DI_SAD_WT4_NATUAL                               114
#define VPHAL_VEBOX_DI_SAD_WT6_NATUAL                               217
#define VPHAL_VEBOX_DI_LPFWTLUT0_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT0_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT1_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT1_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT2_SD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT2_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT3_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT3_HD_NATUAL                          0
#define VPHAL_VEBOX_DI_LPFWTLUT4_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT4_HD_NATUAL                          32
#define VPHAL_VEBOX_DI_LPFWTLUT5_SD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT5_HD_NATUAL                          64
#define VPHAL_VEBOX_DI_LPFWTLUT6_SD_NATUAL                          255
#define VPHAL_VEBOX_DI_LPFWTLUT6_HD_NATUAL                          128
#define VPHAL_VEBOX_DI_LPFWTLUT7_SD_NATUAL                          255
#define VPHAL_VEBOX_DI_LPFWTLUT7_HD_NATUAL                          255

//!
//! \brief Vebox Statistics Surface definition for CNL
//!
#define VPHAL_VEBOX_STATISTICS_SIZE_G10                          (32 * 4)
#define VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE_G10                (32 * sizeof(uint32_t))
#define VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET_G10            0
#define VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G10            0x2C
#define VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET_G10            0x44

//!
//! \brief Vebox Histogram Surface definition for CNL
//!
#define VPHAL_VEBOX_ACE_HISTOGRAM_SLICE0_OFFSET_G10              0x1C00
#define VPHAL_VEBOX_ACE_HISTOGRAM_SLICE1_OFFSET_G10              0x2400
#define VPHAL_VEBOX_ACE_HISTOGRAM_PREVIOUS_SLICE0_OFFSET_G10     0x1800
#define VPHAL_VEBOX_ACE_HISTOGRAM_PREVIOUS_SLICE1_OFFSET_G10     0x2000

//!
//! \brief Chroma Downsampling and Upsampling
//!
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_HORZ_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_HORZ_OFFSET     1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE0_VERT_OFFSET     2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE1_VERT_OFFSET     2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE2_VERT_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE3_VERT_OFFSET     0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE4_VERT_OFFSET     4
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITH_DI_TYPE5_VERT_OFFSET     4
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_HORZ_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_HORZ_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE0_VERT_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE1_VERT_OFFSET  1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE2_VERT_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE3_VERT_OFFSET  0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE4_VERT_OFFSET  2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_420_WITHOUT_DI_TYPE5_VERT_OFFSET  2
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_HORZ_OFFSET             0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_HORZ_OFFSET             1
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE2_VERT_OFFSET             0
#define VPHAL_VEBOX_CHROMA_UPSAMPLING_422_TYPE3_VERT_OFFSET             0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE0_VERT_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE1_VERT_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE2_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE3_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE4_VERT_OFFSET           2
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_420_TYPE5_VERT_OFFSET           2
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_HORZ_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_HORZ_OFFSET           1
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE2_VERT_OFFSET           0
#define VPHAL_VEBOX_CHROMA_DOWNSAMPLING_422_TYPE3_VERT_OFFSET           0


struct VEBOX_STATE_UPDATE_STATIC_DATA_G10
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

typedef class VPHAL_VEBOX_STATE_G10_BASE *PVPHAL_VEBOX_STATE_G10_BASE;
class VPHAL_VEBOX_STATE_G10_BASE:virtual public VPHAL_VEBOX_STATE
{
public:
    VPHAL_VEBOX_STATE_G10_BASE(
        PMOS_INTERFACE                  pOsInterface,
        PMHW_VEBOX_INTERFACE            pVeboxInterface,
        PMHW_SFC_INTERFACE              pSfcInterface,
        PRENDERHAL_INTERFACE            pRenderHal,
        PVPHAL_VEBOX_EXEC_STATE         pVeboxExecState,
        PVPHAL_RNDR_PERF_DATA           pPerfData,
        const VPHAL_DNDI_CACHE_CNTL     &dndiCacheCntl,
        MOS_STATUS                      *peStatus);

    virtual                             ~VPHAL_VEBOX_STATE_G10_BASE() { }

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
        PVPHAL_SURFACE                   pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM  pLumaParams,
        PVPHAL_DNUV_PARAMS               pChromaParams);

    //!
    //! \brief    Vebox set DI parameter
    //! \details  Set deinterlace paramters
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDIParams(
        PVPHAL_SURFACE                   pSrcSurface);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    //!
    //! \brief    Setup Chroma Sampling for Vebox
    //! \details  Setup Chroma Sampling for use in the current Vebox Operation
    //! \param    [in] pChromaSampling
    //!           Pointer to chroma sampling params of Vebox
    //! \return   void
    //!
    void SetupChromaSampling(
        PMHW_VEBOX_CHROMA_SAMPLING      pChromaSampling);

    virtual VphalSfcState* CreateSfcState();
};

#endif // __VPHAL_RENDER_VEBOX_G10_BASE_H__
