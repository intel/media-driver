/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     vp_vebox_cmd_packet.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#ifndef __VP_VEBOX_CMD_PACKET_H__
#define __VP_VEBOX_CMD_PACKET_H__

#include "vp_vebox_cmd_packet_base.h"
#include "vp_vebox_common.h"
#include "vp_filter.h"
#include "mhw_mi_itf.h"
#include "vp_render_sfc_base.h"
#include "hal_oca_interface_next.h"

#define VP_MAX_NUM_FFDI_SURFACES     4                                       //!< 2 for ADI plus additional 2 for parallel execution on HSW+
#define VP_NUM_FFDN_SURFACES         2                                       //!< Number of FFDN surfaces
#define VP_NUM_STMM_SURFACES         2                                       //!< Number of STMM statistics surfaces
#define VP_DNDI_BUFFERS_MAX          4                                       //!< Max DNDI buffers
#define VP_NUM_KERNEL_VEBOX          8                                       //!< Max kernels called at Adv stage

#define VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED          (3072 * 4)

#define VP_VEBOX_LACE_HISTOGRAM_256_BIN_PER_BLOCK          (256 * 2)
#define VP_NUM_RGB_CHANNEL                   3
#define VP_NUM_FRAME_PREVIOUS_CURRENT        2

#define VP_VEBOX_MAX_SLICES_G12                           4

#define VP_VEBOX_RGB_HISTOGRAM_SIZE_G12                   (VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                           VP_NUM_RGB_CHANNEL                    * \
                                                           VP_VEBOX_MAX_SLICES_G12)

#ifndef VEBOX_AUTO_DENOISE_SUPPORTED
#define VEBOX_AUTO_DENOISE_SUPPORTED    1
#endif

//!
//! \brief Denoise Range
//!
#define NOISEFACTOR_MAX                                 64                      //!< Max Slider value
#define NOISEFACTOR_MID                                 32                      //!< Mid Slider value, SKL+ only
#define NOISEFACTOR_MIN                                 0                       //!< Min Slider value

#define VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE                (256 * 4)
#define VP_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE      (256 * 4)

//No matter how many vebox running, the histogram slice number is always 4.
//When there is only one VEBOX, 0 is written to the other histogram slices by HW.
#define VP_VEBOX_HISTOGRAM_SLICES_COUNT                      4

#define VP_VEBOX_RGB_HISTOGRAM_SIZE                      (VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                          VP_NUM_RGB_CHANNEL                    * \
                                                          VP_VEBOX_HISTOGRAM_SLICES_COUNT)
#define VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED         (3072 * 4)

//!
//! \brief Vebox Statistics Surface definition for TGL
//!
#define VP_VEBOX_STATISTICS_SIZE                          (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE                (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET            0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET            0x2C
#define VP_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET           0x5C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET            0x44

//!
//! \brief Spatial Denoise Definitions
//!
#define NOSIE_GNE_CHROMA_THRESHOLD                        1850
#define NOSIE_GNE_LUMA_THRESHOLD                          32000
#define NOSIE_GNE_RESOLUTION_THRESHOLD                    2073600  // size of 1080P
//!
//! \brief Temporal Denoise Definitions
//!
//! 
#define NOISE_HISTORY_DELTA_DEFAULT                     8
#define NOISE_HISTORY_MAX_DEFAULT                       192
#define NOISE_NUMMOTIONPIXELS_THRESHOLD_DEFAULT         0
#define NOISE_LOWTEMPORALPIXELDIFF_THRESHOLD_DEFAULT    6
#define NOISE_TEMPORALPIXELDIFF_THRESHOLD_DEFAULT       12
#define NOISE_SUMABSTEMPORALDIFF_THRESHOLD_DEFAULT      128

// Pixel Range Threshold Array Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL          1
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL_MIN      65536
#define NOISE_BLF_RANGE_THRESHOLD_ADP_NLVL_MAX      393216
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI0          192
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI1          320
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI2          384
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI3          640
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI4          1024
#define NOISE_BLF_RANGE_THRESHOLD_NLVL_MI5          1280
#define NOISE_BLF_RANGE_THRADPDYNR_MIN              512
#define NOISE_BLF_RANGE_THRADPDYNR_MAX              2048
#define NOISE_BLF_RANGE_THRDYNR_MIN                 256
#define NOISE_BLF_RANGE_THRESHOLD_S0_MIN            32
#define NOISE_BLF_RANGE_THRESHOLD_S0_MID            192
#define NOISE_BLF_RANGE_THRESHOLD_S0_MAX            384
#define NOISE_BLF_RANGE_THRESHOLD_S1_MIN            64
#define NOISE_BLF_RANGE_THRESHOLD_S1_MID            256
#define NOISE_BLF_RANGE_THRESHOLD_S1_MAX            576
#define NOISE_BLF_RANGE_THRESHOLD_S2_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S2_MID            512
#define NOISE_BLF_RANGE_THRESHOLD_S2_MAX            896
#define NOISE_BLF_RANGE_THRESHOLD_S3_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S3_MID            640
#define NOISE_BLF_RANGE_THRESHOLD_S3_MAX            1280
#define NOISE_BLF_RANGE_THRESHOLD_S4_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S4_MID            896
#define NOISE_BLF_RANGE_THRESHOLD_S4_MAX            1920
#define NOISE_BLF_RANGE_THRESHOLD_S5_MIN            128
#define NOISE_BLF_RANGE_THRESHOLD_S5_MID            1280
#define NOISE_BLF_RANGE_THRESHOLD_S5_MAX            2560
#define NOISE_BLF_RANGE_THRESHOLD_S0_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S0_MID
#define NOISE_BLF_RANGE_THRESHOLD_S1_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S1_MID
#define NOISE_BLF_RANGE_THRESHOLD_S2_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S2_MID
#define NOISE_BLF_RANGE_THRESHOLD_S3_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S3_MID
#define NOISE_BLF_RANGE_THRESHOLD_S4_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S4_MID
#define NOISE_BLF_RANGE_THRESHOLD_S5_DEFAULT        NOISE_BLF_RANGE_THRESHOLD_S5_MID
#define NOISE_BLF_RANGE_THRESHOLD_S0_AUTO_DEFAULT   192
#define NOISE_BLF_RANGE_THRESHOLD_S1_AUTO_DEFAULT   320
#define NOISE_BLF_RANGE_THRESHOLD_S2_AUTO_DEFAULT   384
#define NOISE_BLF_RANGE_THRESHOLD_S3_AUTO_DEFAULT   640
#define NOISE_BLF_RANGE_THRESHOLD_S4_AUTO_DEFAULT   1024
#define NOISE_BLF_RANGE_THRESHOLD_S5_AUTO_DEFAULT   1280

// Pixel Range Weight Array Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_RANGE_WGTS0_MIN                   16
#define NOISE_BLF_RANGE_WGTS0_MID                   16
#define NOISE_BLF_RANGE_WGTS0_MAX                   16
#define NOISE_BLF_RANGE_WGTS1_MIN                   9
#define NOISE_BLF_RANGE_WGTS1_MID                   14
#define NOISE_BLF_RANGE_WGTS1_MAX                   15
#define NOISE_BLF_RANGE_WGTS2_MIN                   2
#define NOISE_BLF_RANGE_WGTS2_MID                   10
#define NOISE_BLF_RANGE_WGTS2_MAX                   13
#define NOISE_BLF_RANGE_WGTS3_MIN                   0
#define NOISE_BLF_RANGE_WGTS3_MID                   5
#define NOISE_BLF_RANGE_WGTS3_MAX                   10
#define NOISE_BLF_RANGE_WGTS4_MIN                   0
#define NOISE_BLF_RANGE_WGTS4_MID                   2
#define NOISE_BLF_RANGE_WGTS4_MAX                   7
#define NOISE_BLF_RANGE_WGTS5_MIN                   0
#define NOISE_BLF_RANGE_WGTS5_MID                   1
#define NOISE_BLF_RANGE_WGTS5_MAX                   4
#define NOISE_BLF_RANGE_WGTS0_DEFAULT               NOISE_BLF_RANGE_WGTS0_MID
#define NOISE_BLF_RANGE_WGTS1_DEFAULT               NOISE_BLF_RANGE_WGTS1_MID
#define NOISE_BLF_RANGE_WGTS2_DEFAULT               NOISE_BLF_RANGE_WGTS2_MID
#define NOISE_BLF_RANGE_WGTS3_DEFAULT               NOISE_BLF_RANGE_WGTS3_MID
#define NOISE_BLF_RANGE_WGTS4_DEFAULT               NOISE_BLF_RANGE_WGTS4_MID
#define NOISE_BLF_RANGE_WGTS5_DEFAULT               NOISE_BLF_RANGE_WGTS5_MID

// Distance Weight Matrix Denoise Definitions for SKL+ 5x5 Bilateral Filter
#define NOISE_BLF_DISTANCE_WGTS00_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS01_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS02_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS10_DEFAULT           12
#define NOISE_BLF_DISTANCE_WGTS11_DEFAULT           11
#define NOISE_BLF_DISTANCE_WGTS12_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS20_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS21_DEFAULT           10
#define NOISE_BLF_DISTANCE_WGTS22_DEFAULT           8

//!
//! \brief Improved Deinterlacing for CNL+
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
//! \brief STE factor
//!

#define MHW_STE_FACTOR_MAX                      9  // STE factor is 0 ~ 9

// \brief   VEBOX 3DLUT HDR
#define LUT65_SEG_SIZE                                              65
#define LUT65_MUL_SIZE                                              128
#define LUT33_SEG_SIZE                                              33
#define LUT33_MUL_SIZE                                              64
#define VP_VEBOX_HDR_3DLUT65                                        LUT65_SEG_SIZE *\
                                                                    LUT65_SEG_SIZE *\
                                                                    LUT65_MUL_SIZE * sizeof(int64_t)
#define VP_VEBOX_HDR_3DLUT33                                        LUT33_SEG_SIZE *             \
                                                                    LUT33_SEG_SIZE *     \
                                                                    LUT33_MUL_SIZE * \
                                                                    sizeof(int64_t)
#define SHAPE_1K_LOOKUP_SIZE                                        2048 * sizeof(uint32_t)
//!
//! \brief Vebox Statistics Surface definition
//!
#define VPHAL_VEBOX_STATISTICS_SIZE (32 * 8)
#define VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET 0x2C
#define VPHAL_VEBOX_STATISTICS_SURFACE_STD_OFFSET 0x44
#define VPHAL_VEBOX_STATISTICS_PER_FRAME_SIZE (32 * sizeof(uint32_t))
#define VPHAL_VEBOX_STATISTICS_SURFACE_FMD_OFFSET 0

//! \brief Number of LACE's PWLF surfaces
//!
#define VP_NUM_LACE_PWLF_SURFACES                                   2

//!
//! \brief  Chroma Denoise params
//!
typedef struct _VPHAL_DNUV_PARAMS
{
    uint32_t    dwHistoryInitUV;
    uint32_t    dwHistoryDeltaUV;
    uint32_t    dwHistoryMaxUV;
    uint32_t    dwSTADThresholdU;
    uint32_t    dwSTADThresholdV;
    uint32_t    dwLTDThresholdU;
    uint32_t    dwLTDThresholdV;
    uint32_t    dwTDThresholdU;
    uint32_t    dwTDThresholdV;
} VPHAL_DNUV_PARAMS, *PVPHAL_DNUV_PARAMS;

//!
//! \brief  Structure to handle DNDI sampler states
//!
typedef struct _VP_SAMPLER_STATE_DN_PARAM
{
    uint32_t  dwDenoiseASDThreshold;
    uint32_t  dwDenoiseHistoryDelta;
    uint32_t  dwDenoiseMaximumHistory;
    uint32_t  dwDenoiseSTADThreshold;
    uint32_t  dwDenoiseSCMThreshold;
    uint32_t  dwDenoiseMPThreshold;
    uint32_t  dwLTDThreshold;
    uint32_t  dwTDThreshold;
    uint32_t  dwGoodNeighborThreshold;
} VP_SAMPLER_STATE_DN_PARAM, *PVP_SAMPLER_STATE_DN_PARAM;

namespace vp
{
typedef struct _VEBOX_PACKET_SURFACE_PARAMS
{
    VP_SURFACE                      *pCurrInput;
    VP_SURFACE                      *pPrevInput;
    VP_SURFACE                      *pSTMMInput;
    VP_SURFACE                      *pSTMMOutput;
    VP_SURFACE                      *pDenoisedCurrOutput;
    VP_SURFACE                      *pCurrOutput;           //!< Current Vebox Output
    VP_SURFACE                      *pPrevOutput;
    VP_SURFACE                      *pStatisticsOutput;
    VP_SURFACE                      *pAlphaOrVignette;
    VP_SURFACE                      *pLaceOrAceOrRgbHistogram;
    VP_SURFACE                      *pSurfSkinScoreOutput;
    VP_SURFACE                      *pFMDHistorySurface;
    VP_SURFACE                      *pInnerTileConvertInput;
}VEBOX_PACKET_SURFACE_PARAMS, *PVEBOX_PACKET_SURFACE_PARAMS;
};

enum MEDIASTATE_DNDI_FIELDCOPY_SELECT
{
    MEDIASTATE_DNDI_DEINTERLACE     = 0,
    MEDIASTATE_DNDI_FIELDCOPY_PREV  = 1,
    MEDIASTATE_DNDI_FIELDCOPY_NEXT  = 2
};

//!
//! \brief VPHAL Query Type for Vebox Statistics Surface
//!
typedef enum _VEBOX_STAT_QUERY_TYPE
{
    VEBOX_STAT_QUERY_FMD_OFFEST = 0,
    VEBOX_STAT_QUERY_WB_OFFEST,
    VEBOX_STAT_QUERY_GNE_OFFEST,
    VEBOX_STAT_QUERY_STD_OFFEST,
    VEBOX_STAT_QUERY_GCC_OFFEST,
    VEBOX_STAT_QUERY_PER_FRAME_SIZE
} VEBOX_STAT_QUERY_TYPE;

//!
//! \brief  Feature specific cache control settings
//!
typedef struct _VP_VEBOX_CACHE_CNTL
{
    // Input
    bool                        bDnDi;
    bool                        bLace;

    // Output
    VPHAL_DNDI_CACHE_CNTL        DnDi;
    VPHAL_LACE_CACHE_CNTL        Lace;
} VP_VEBOX_CACHE_CNTL, *PVP_VEBOX_CACHE_CNTL;

namespace vp {

class VpVeboxCmdPacket : virtual public VpVeboxCmdPacketBase
{
public:
    VpVeboxCmdPacket(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc);

    virtual ~VpVeboxCmdPacket();

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Destory() { return MOS_STATUS_SUCCESS; };

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS PrepareState() override;

    virtual MOS_STATUS                  AllocateExecRenderData()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_lastExecRenderData)
        {
            m_lastExecRenderData = MOS_New(VpVeboxRenderData);
            if (!m_lastExecRenderData)
            {
                return MOS_STATUS_NO_SPACE;
            }

            eStatus = m_lastExecRenderData->Init();
            if (eStatus != MOS_STATUS_SUCCESS)
            {
                MOS_Delete(m_lastExecRenderData);
            }

        }
        return eStatus;
    }

    virtual VpVeboxRenderData *GetLastExecRenderData()
    {
        if (!m_lastExecRenderData)
        {
            AllocateExecRenderData();
        }
        return m_lastExecRenderData;
    }

    virtual bool IsIECPEnabled()
    {
        if (GetLastExecRenderData() != NULL)
        {
           return GetLastExecRenderData()->IECP.IsIecpEnabled();
        }
        return false;
    }

    virtual MOS_STATUS ValidateHDR3DLutParameters(bool is3DLutTableFilled);

    //!
    //! \brief    Setup surface states for Vebox
    //! \details  Setup surface states for use in the current Vebox Operation
    //! \param    [in] bDiVarianceEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxSurfaceStateCmdParams
    //!           Pointer to VEBOX_SURFACE_STATE command parameters
    //! \return   void
    //!
    virtual MOS_STATUS SetupSurfaceStates(
        PVP_VEBOX_SURFACE_STATE_CMD_PARAMS  pVeboxSurfaceStateCmdParams);

    //!
    //! \brief    Setup surface states for Vebox
    //! \details  Setup surface states for use in the current Vebox Operation
    //! \param    [in] pRenderHal
    //!           Pointer to Render Hal
    //! \param    [in] CmdBuffer
    //!           Pointer to command buffer
    //! \param    [in] pGenericPrologParams
    //!           pointer to Generic prolog params struct to send to cmd buffer header
    //! \return   void
    //!
    virtual MOS_STATUS InitCmdBufferWithVeParams(
        PRENDERHAL_INTERFACE                    pRenderHal,
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    //!
    //! \brief    Setup Scaling Params for Vebox/SFC
    //! \details  Setup surface Scaling Params for Vebox/SFC
    //! \param    [in] scalingParams
    //!           Scaling Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams) override;

    //!
    //! \brief    Setup CSC Params for Vebox/SFC
    //! \details  Setup surface CSC Params for Vebox/SFC
    //! \param    [in] cscParams
    //!           CSC/IEF Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetSfcCSCParams(PSFC_CSC_PARAMS cscParams) override;

    //!
    //! \brief    Setup CSC Params for Vebox back end
    //! \details  Setup surface CSC Params for Vebox
    //! \param    [in] cscParams
    //!           CSC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetVeboxCSCParams(PVEBOX_CSC_PARAMS cscParams) override;

    //!
    //! \brief    Setup CSC Params for Vebox front end
    //! \details  Setup surface CSC Params for Vebox
    //! \param    [in] cscParams
    //!           CSC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetVeboxFeCSCParams(PVEBOX_CSC_PARAMS cscParams) override;

     //!
    //! \brief    Setup CSC Params for Vebox
    //! \details  Setup surface CSC Params for Vebox
    //! \param    [in] cscParams
    //!           CSC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetVeboxBeCSCParams(PVEBOX_CSC_PARAMS cscParams) override;

    //!
    //! \brief    Setup Vebox Output Alpha Value
    //! \details  Setup Vebox Output Alpha Value
    //! \param    [in] cscParams
    //!           CSC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetVeboxOutputAlphaParams(PVEBOX_CSC_PARAMS cscParams) ;

    //!
    //! \brief    Setup Vebox Chroma sub sampling
    //! \details  Setup Vebox Chroma sub sampling
    //! \param    [in] cscParams
    //!           CSC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetVeboxChromasitingParams(PVEBOX_CSC_PARAMS cscParams);

    //!
    //! \brief    Setup Roattion/Mirror Params for Vebox/SFC
    //! \details  Setup surface Roattion/Mirror Params for Vebox/SFC
    //! \param    [in] rotMirParams
    //!           Rotation/Mirror Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetSfcRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams) override;

    //!
    //! \brief    Setup DN Params for Vebox
    //! \details  Setup surface DN Params for Vebox
    //! \param    [in] dnParams
    //!           DN Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDnParams(PVEBOX_DN_PARAMS dnParams) override;

    //!
    //! \brief    Setup STE Params for Vebox
    //! \details  Setup surface STE Params for Vebox
    //! \param    [in] steParams
    //!           STE Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetSteParams(PVEBOX_STE_PARAMS steParams) override;

    //!
    //! \brief    Setup HDR Params for Vebox
    //! \details  Setup surface HDR Params for Vebox
    //! \param    [in] HDRParams
    //!           HDR Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetHdrParams(PVEBOX_HDR_PARAMS hdrParams) override;

    //!
    //! \brief    Setup TCC Params for Vebox
    //! \details  Setup surface TCC Params for Vebox
    //! \param    [in] tccParams
    //!           TCC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetTccParams(PVEBOX_TCC_PARAMS tccParams) override;

    //!
    //! \brief    Setup Procamp Params for Vebox
    //! \details  Setup surface Procamp Params for Vebox
    //! \param    [in] procampParams
    //!           Procamp Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetProcampParams(PVEBOX_PROCAMP_PARAMS procampParams) override;

    //!
    //! \brief    Setup DI Params for Vebox
    //! \details  Setup surface DN Params for Vebox
    //! \param    [in] diParams
    //!           DI Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetDiParams(PVEBOX_DI_PARAMS diParams) override;
    virtual MOS_STATUS ConfigureSteParams(VpVeboxRenderData *renderData, bool bEnableSte, uint32_t dwSTEFactor, bool bEnableStd, uint32_t stdParaSizeInBytes, void *stdParams);

    //!
    //! \brief    Setup CGC Params for Vebox
    //! \details  Setup surface CGC Params for Vebox
    //! \param    [in] cgcParams
    //!           CGC Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetCgcParams(PVEBOX_CGC_PARAMS cgcParams) override;
    virtual MOS_STATUS ConfigureTccParams(VpVeboxRenderData *renderData, bool bEnableTcc, uint8_t magenta, uint8_t red, uint8_t yellow, uint8_t green, uint8_t cyan, uint8_t blue);
    virtual MOS_STATUS ConfigureProcampParams(VpVeboxRenderData *renderData, bool bEnableProcamp, float fBrightness, float fContrast, float fHue, float fSaturation);
    virtual MOS_STATUS ConfigureDenoiseParams(VpVeboxRenderData *renderData, float fDenoiseFactor);

    virtual MOS_STATUS UpdateCscParams(FeatureParamCsc &params) override;
    virtual MOS_STATUS UpdateDenoiseParams(FeatureParamDenoise &params) override;
    virtual MOS_STATUS UpdateTccParams(FeatureParamTcc &params) override;
    virtual MOS_STATUS UpdateSteParams(FeatureParamSte &params) override;
    virtual MOS_STATUS UpdateProcampParams(FeatureParamProcamp &params) override;
    
    virtual void AddCommonOcaMessage(PMOS_COMMAND_BUFFER pCmdBufferInUse, MOS_CONTEXT_HANDLE pOsContext, PMOS_INTERFACE pOsInterface, PRENDERHAL_INTERFACE pRenderHal, PMHW_MI_MMIOREGISTERS pMmioRegisters);
    //!
    //! \brief    Get DN luma parameters
    //! \details  Get DN luma parameters
    //! \param    [in] bDnEnabled
    //!           true if DN being enabled
    //! \param    [in] bAutoDetect
    //!           true if auto DN being enabled
    //! \param    [in] fDnFactor
    //!           DN factor
    //! \param    [in] bRefValid
    //!           true if reference surface available
    //! \param    [out] pLumaParams
    //!           DN luma parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetDnLumaParams(
        bool                        bDnEnabled,
        bool                        bAutoDetect,
        float                       fDnFactor,
        bool                        bRefValid,
        PVP_SAMPLER_STATE_DN_PARAM  pLumaParams) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Get DN chroma parameters
    //! \details  Get DN chroma parameters
    //! \param    [in] bChromaDenoise
    //!           true if chroma DN being enabled
    //! \param    [in] bAutoDetect
    //!           true if auto DN being enabled
    //! \param    [in] fDnFactor
    //!           DN factor
    //! \param    [out] pChromaParams
    //!           DN chroma parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetDnChromaParams(
        bool                        bChromaDenoise,
        bool                        bAutoDetect,
        float                       fDnFactor,
        PVPHAL_DNUV_PARAMS          pChromaParams) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Config DN luma pix range
    //! \details  Config DN luma pix range threshold and weight
    //! \param    [in] bDnEnabled
    //!           true if DN being enabled
    //! \param    [in] bAutoDetect
    //!           true if auto DN being enabled
    //! \param    [in] fDnFactor
    //!           DN factor
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ConfigLumaPixRange(
        bool                        bDnEnabled,
        bool                        bAutoDetect,
        float                       fDnFactor) { return MOS_STATUS_SUCCESS; }

    //!
    //! \brief    Config DN chroma pix range
    //! \details  Config DN chroma pix range threshold and weight
    //! \param    [in] bChromaDenoise
    //!           true if chroma DN being enabled
    //! \param    [in] bAutoDetect
    //!           true if auto DN being enabled
    //! \param    [in] fDnFactor
    //!           DN factor
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ConfigChromaPixRange(
        bool                        bChromaDenoise,
        bool                        bAutoDetect,
        float                       fDnFactor) { return MOS_STATUS_SUCCESS; }

    virtual MOS_STATUS InitSTMMHistory();

    //!
    //! \brief    Vebox Populate VEBOX parameters
    //! \details  Populate the Vebox VEBOX state parameters to VEBOX RenderData
    //! \param    [in] bDnEnabled
    //!           true if DN being enabled
    //! \param    [in] bChromaDenoise
    //!           true if chroma DN being enabled
    //! \param    [in] pLumaParams
    //!           Pointer to Luma DN and DI parameter
    //! \param    [in] pChromaParams
    //!           Pointer to Chroma DN parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ConfigDnLumaChromaParams(
        bool                            bDnEnabled,
        bool                            bChromaDenoise,
        PVP_SAMPLER_STATE_DN_PARAM      pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams
        );

    //!
    //! \brief    Calculate offsets of statistics surface address based on the
    //!           functions which were enabled in the previous call,
    //!           and store the width and height of the per-block statistics into DNDI_STATE
    //! \details
    //! Layout of Statistics surface when Temporal DI enabled
    //!     --------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
    //!     |-------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=4       | ...\n
    //!     |------------------------------\n
    //!     | ...\n
    //!     |------------------------------\n
    //!     | 16 bytes for x=0, Y=height-4| ...\n
    //!     |-----------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Previous)| 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Current) | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Previous)| 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Current) | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //!
    //! Layout of Statistics surface when DN or Spatial DI enabled (and Temporal DI disabled)
    //!     --------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=0       | 16 bytes for x=16, Y=0       | ...\n
    //!     |-------------------------------------------------------------\n
    //!     | 16 bytes for x=0, Y=4       | ...\n
    //!     |------------------------------\n
    //!     | ...\n
    //!     |------------------------------\n
    //!     | 16 bytes for x=0, Y=height-4| ...\n
    //!     |-----------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Input)   | 11 DW FMD0 | 6 DW GNE0 | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Input)   | 11 DW FMD1 | 6 DW GNE1 | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //!
    //! Layout of Statistics surface when both DN and DI are disabled
    //!     ------------------------------------------------Pitch----------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 0 (Input)   | 17 DW Reserved         | 2 DW STD0 | 2 DW GCC0 | 11 DW Reserved |\n
    //!     |--------------------------------------------------------------------------------------------------------------\n
    //!     | 256 DW of ACE histogram Slice 1 (Input)   | 17 DW Reserved         | 2 DW STD1 | 2 DW GCC1 | 11 DW Reserved |\n
    //!     ---------------------------------------------------------------------------------------------------------------\n
    //! \param    [out] pStatSlice0Offset
    //!           Statistics surface Slice 0 base pointer
    //! \param    [out] pStatSlice1Offset
    //!           Statistics surface Slice 1 base pointer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetStatisticsSurfaceOffsets(
        int32_t*                            pStatSlice0Offset,
        int32_t*                            pStatSlice1Offset);

    //!
    //! \brief    Configure FMD parameter
    //! \details  Configure FMD parameters for DNDI State
    //! \param    [in] bProgressive
    //!           true if sample being progressive
    //! \param    [in] bAutoDenoise
    //!           true if auto denoise being enabled
    //! \param    [out] pLumaParams
    //!           Pointer to DNDI Param for set FMD parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS ConfigFMDParams(bool bProgressive, bool bAutoDenoise, bool bFmdEnabled);

    //!
    //! \brief    Setup Vebox_State Command parameter
    //! \param    [in,out] pVeboxStateCmdParams
    //!           Pointer to VEBOX_STATE command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupVeboxState(mhw::vebox::VEBOX_STATE_PAR& veboxStateCmdParams);

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
        bool                        bDiScdEnable,
        mhw::vebox::VEB_DI_IECP_PAR &veboxDiIecpCmdParam);

    //!
    //! \brief    Check Vebox using kernel resource or not
    //! \details  Check Vebox using kernel resource or not
    //! \return   bool
    //!           Return true if use kernel resource
    //!
    virtual bool UseKernelResource();

    //!
    //! \brief    Vebox send Vebox ring HW commands
    //! \details  Send Vebox ring Commands.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SendVeboxCmd(MOS_COMMAND_BUFFER* commandBuffer);

    //!
    //! \brief    enable or disable SFC output path.
    //! \details  enable or disable SFC output path
    //! \return   void
    //!
    virtual void SetSfcOutputPath(bool bSfcUsed) { m_IsSfcUsed = bSfcUsed; };

    virtual SfcRenderBase* GetSfcRenderInstance() { return m_sfcRender; };

    virtual MOS_STATUS PacketInit(
        VP_SURFACE                          *inputSurface,
        VP_SURFACE                          *outputSurface,
        VP_SURFACE                          *previousSurface,
        VP_SURFACE_SETTING                  &surfSetting,
        VP_EXECUTE_CAPS                     packetCaps) override;

    virtual MOS_STATUS SetUpdatedExecuteResource(
        VP_SURFACE                          *inputSurface,
        VP_SURFACE                          *outputSurface,
        VP_SURFACE                          *previousSurface,
        VP_SURFACE_SETTING                  &surfSetting) override;

    //!
    //! \brief    Check whether the Vebox command parameters are correct
    //! \param    [in] VeboxStateCmdParams
    //!           MHW vebox state cmd params
    //! \param    [in] VeboxDiIecpCmdParams
    //!           DiIecpCmd params struct
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS IsCmdParamsValid(
        const mhw::vebox::VEBOX_STATE_PAR           &veboxStateCmdParams,
        const mhw::vebox::VEB_DI_IECP_PAR           &veboxDiIecpCmdParams,
        const VP_VEBOX_SURFACE_STATE_CMD_PARAMS  &VeboxSurfaceStateCmdParams);

    virtual MOS_STATUS QueryStatLayout(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t*             pQuery) {return MOS_STATUS_SUCCESS;};

    //!
    //! \brief    Determine if the Batch Buffer End is needed to add in the end
    //! \details  Detect platform OS and return the flag whether the Batch Buffer End is needed to add in the end
    //! \param    [in] pOsInterface
    //!           Pointer to MOS_INTERFACE
    //! \return   bool
    //!           The flag of adding Batch Buffer End
    //!
    virtual bool RndrCommonIsMiBBEndNeeded(
        PMOS_INTERFACE           pOsInterface);

    //!
    //! \brief    Vebox perftag
    //! \details  set vebox perftag
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetPerfTag();

    //!
    //! \brief    Vebox perftag for NV12 source
    //! \details  set vebox perftag for NV12 source
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetPerfTagNv12();

    //!
    //! \brief    Vebox perftag for Pa format source
    //! \details  set vebox perftag for Pa format source
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetPerfTagPaFormat();

    //!
    //! \brief    Vebox state heap update for auto mode features
    //! \details  Update Vebox indirect states for auto mode features
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateVeboxStates();

    //! \brief    Vebox get statistics surface base
    //! \details  Calculate address of statistics surface address based on the
    //!           functions which were enabled in the previous call.
    //! \param    uint8_t* pStat
    //!           [in] Pointer to Statistics surface
    //! \param    uint8_t* * pStatSlice0Base
    //!           [out] Statistics surface Slice 0 base pointer
    //! \param    uint8_t* * pStatSlice1Base
    //!           [out] Statistics surface Slice 1 base pointer
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetStatisticsSurfaceBase(
        uint8_t  *pStat,
        uint8_t **pStatSlice0Base,
        uint8_t **pStatSlice1Base);

    virtual MOS_STATUS QueryStatLayoutGNE(
        VEBOX_STAT_QUERY_TYPE QueryType,
        uint32_t             *pQuery,
        uint8_t              *pStatSlice0Base,
        uint8_t              *pStatSlice1Base);

    virtual MOS_STATUS CheckTGNEValid(
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr,
        uint32_t *pQuery);
    //!
    //! \brief    Vebox update HVS DN states
    //! \details  CPU update for VEBOX DN states
    //! \param    bDnEnabled
    //!           [in] true if DN enabled
    //! \param    bChromaDenoise
    //!           [in] true if chroma DN enabled
    //! \param    bAutoDenoise
    //!           [in] true if auto DN enabled
    //! \param    uint32_t* pStatSlice0GNEPtr
    //!           [out] Pointer to Vebox slice0 GNE data
    //! \param    uint32_t* pStatSlice1GNEPtr
    //!           [out] Pointer to Vebox slice1 GNE data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateDnHVSParameters(
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr);

    //!
    //! \brief    Vebox state adjust boundary for statistics surface
    //! \details  Adjust boundary for statistics surface block
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AdjustBlockStatistics();

    virtual MOS_STATUS GNELumaConsistentCheck(
        uint32_t &dwGNELuma,
        uint32_t *pStatSlice0GNEPtr,
        uint32_t *pStatSlice1GNEPtr);

    MOS_STATUS InitVeboxSurfaceStateCmdParamsForTileConvert(
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS mhwVeboxSurfaceStateCmdParams,
        PMOS_SURFACE                        inputSurface,
        PMOS_SURFACE                        outputSurface);

    MOS_STATUS AddTileConvertStates(
        MOS_COMMAND_BUFFER *CmdBuffer,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS &MhwVeboxSurfaceStateCmdParams);

    MOS_FORMAT AdjustFormatForTileConvert(MOS_FORMAT format);
    // TGNE
    uint32_t dwGlobalNoiseLevel_Temporal  = 0;  //!< Global Temporal Noise Level for Y
    uint32_t dwGlobalNoiseLevelU_Temporal = 0;  //!< Global Temporal Noise Level for U
    uint32_t dwGlobalNoiseLevelV_Temporal = 0;  //!< Global Temporal Noise Level for V
    uint32_t curNoiseLevel_Temporal       = 0;  //!< Temporal Noise Level for Y
    uint32_t curNoiseLevelU_Temporal      = 0;  //!< Temporal Noise Level for U
    uint32_t curNoiseLevelV_Temporal      = 0;  //!< Temporal Noise Level for V
    bool     m_bTgneEnable                = true;
    bool     m_bTgneValid                 = false;
    bool     m_bFallback                  = false;

    mhw::vebox::MHW_VEBOX_CHROMA_PARAMS veboxChromaParams = {};

#if (_DEBUG || _RELEASE_INTERNAL)
    virtual MOS_STATUS StallBatchBuffer(
        PMOS_COMMAND_BUFFER cmdBuffer)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS StoreCSEngineIdRegMem(
        MOS_COMMAND_BUFFER *cmdBuffer,
        const MHW_VEBOX_HEAP *veboxHeap)
    {
        return MOS_STATUS_SUCCESS;
    }

#endif

protected:

    //!
    //! \brief    Doing prepare stage tasks for SendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] GenericPrologParams
    //!           GpuStatusBuffer resource to be set
    //! \param    [out] iRemaining
    //!           integer showing initial cmd buffer usage
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
   virtual MOS_STATUS PrepareVeboxCmd(
      MOS_COMMAND_BUFFER*                      CmdBuffer,
      RENDERHAL_GENERIC_PROLOG_PARAMS&         GenericPrologParams,
      int32_t&                                 iRemaining);

    //!
    //! \brief    Render the Vebox Cmd buffer for SendVeboxCmd
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
    virtual MOS_STATUS RenderVeboxCmd(
        MOS_COMMAND_BUFFER                      *CmdBuffer,
        VP_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    //!
    //! \brief    handle Cmd buffer's offset when error occur
    //! \details  handle Cmd buffer's offset when error occur
    //! \param    [in,out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \return   void
    void CmdErrorHanlde(
        MOS_COMMAND_BUFFER  *CmdBuffer,
        int32_t             &iRemaining);

    //!
    //! \brief    Send Vecs Status Tag
    //! \details  Add MI Flush with write back into command buffer for GPU to write 
    //!           back GPU Tag. This should be the last command in 1st level batch.
    //!           This ensures sync tag will be written after rendering is complete.
    //! \param    [in] pOsInterface
    //!           Pointer to OS Interface
    //! \param    [out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SendVecsStatusTag(
      PMOS_INTERFACE                      pOsInterface,
      PMOS_COMMAND_BUFFER                 pCmdBuffer);

    virtual MOS_STATUS InitVeboxSurfaceStateCmdParams(
        PVP_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams);

    virtual MOS_STATUS InitVeboxSurfaceParams(
        PVP_SURFACE                     pVpHalVeboxSurface,
        PMHW_VEBOX_SURFACE_PARAMS       pMhwVeboxSurface);

    //!
    //! \brief    Copy Surface value
    //! \param    [in] pTargetSurface
    //!           Pointer to surface copy value to 
    //! \param    [in] pSourceSurface
    //!           Pointer to surface copy value from 
    //! \return   void
    //!
    virtual void CopySurfaceValue(
      PVP_SURFACE                 pTargetSurface,
      PVP_SURFACE                 pSourceSurface);

    //!
    //! \brief    Add vebox DNDI state
    //! \details  Add vebox DNDI state
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AddVeboxDndiState();

    //!
    //! \brief    Add vebox IECP state
    //! \details  Add vebox IECP state
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AddVeboxIECPState();

    //!
    //! \brief    Add vebox Hdr state
    //! \details  Add vebox Hdr state
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AddVeboxHdrState();

    virtual bool IsVeboxGamutStateNeeded();

    //!
    //! \brief    Add vebox Gamut state
    //! \details  Add vebox Gamut state
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AddVeboxGamutState();

    //!
    //! \brief    Vebox set up vebox state heap
    //! \details  Setup Vebox indirect states: DNDI and etc
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupIndirectStates();

    //!
    //! \brief    Vebox get the back-end colorspace conversion matrix
    //! \details  When the i/o is A8R8G8B8 or X8R8G8B8, the transfer matrix
    //!           needs to be updated accordingly
    //! \param    [in] inputColorSpace
    //!           color space of vebox input surface
    //! \param    [in] outputColorSpace
    //!           color space of vebox output surface
    //! \param    [in] inputFormat
    //!           format of vebox input surface
    //! \return   void
    //!
    virtual void VeboxGetBeCSCMatrix(
        VPHAL_CSPACE    inputColorSpace,
        VPHAL_CSPACE    outputColorSpace,
        MOS_FORMAT      inputFormat);

    virtual MOS_STATUS SetDiParams(
        bool                    bDiEnabled,
        bool                    bSCDEnabled,
        bool                    bHDContent,
        VPHAL_SAMPLE_TYPE       sampleTypeInput,
        MHW_VEBOX_DNDI_PARAMS   &param);

    bool IsTopField(VPHAL_SAMPLE_TYPE sampleType);
    bool IsTopFieldFirst(VPHAL_SAMPLE_TYPE sampleType);

    //!
    //! \brief    Get surface by type
    //! \details  Get surface by type
    //! \param    [in] type
    //!           surface type
    //! \return   VP_SURFACE*
    //!           Pointer to surface of specified type
    //!
    virtual VP_SURFACE* GetSurface(SurfaceType type);

    virtual MOS_STATUS InitSurfMemCacheControl(VP_EXECUTE_CAPS packetCaps);

    virtual MHW_CSPACE VpHalCspace2MhwCspace(VPHAL_CSPACE cspace);

    virtual MOS_STATUS SetupDNTableForHVS(
        mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams);

    virtual MOS_STATUS SetupHDRLuts(
        mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams);
    virtual MOS_STATUS Init3DLutTable(PVP_SURFACE surf3DLut);
    void    UpdateCpPrepareResources();
    virtual MOS_STATUS Add1DLutState(PVP_SURFACE &surface, PMHW_1DLUT_PARAMS p1DLutParams);
    virtual MOS_STATUS SetupVebox3DLutForHDR(
        mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams);
    virtual MOS_STATUS SetupVeboxFP16State(mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetupHDRUnifiedForHDR(
        mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams);

    virtual MOS_STATUS SetupVeboxExternal3DLutforHDR(
        mhw::vebox::VEBOX_STATE_PAR &veboxStateCmdParams);

private:

    //!
    //! \brief    IsFormatMMCSupported
    //! \details  Check if the format of vebox output surface is supported by MMC
    //! \param    [in] Format
    //! \return   bool  true if suported, otherwise not supported
    //!
    bool IsFormatMMCSupported(
        MOS_FORMAT                  Format);

    //!
    //! \brief    SetSfcMmcParams
    //! \details  set sfc state mmc related params
    //! \return   bool  success if succeeded, otherwise failure
    //!
    virtual MOS_STATUS SetSfcMmcParams();
    MOS_STATUS InitSfcRender();

    //!
    //! \brief    Dump Vebox State Heap
    //! \details  Dump Vebox State Heap
    //! \return   MOS_STATUS  MOS_STATUS_SUCCESS if succeeded, otherwise failure
    //!
    MOS_STATUS DumpVeboxStateHeap();

    MOS_STATUS SetVeboxSurfaceControlBits(
        MHW_VEBOX_SURFACE_CNTL_PARAMS       *pVeboxSurfCntlParams,
        uint32_t                            *pSurfCtrlBits);

    MOS_STATUS SetVeboxProCmd(
        MOS_COMMAND_BUFFER*   CmdBuffer);

    MOS_STATUS SetVeboxIndex(
        uint32_t                            dwVeboxIndex,
        uint32_t                            dwVeboxCount,
        uint32_t                            dwUsingSFC);

    MOS_STATUS SetVeboxState(
        PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

    MOS_STATUS SetVeboxSurfaces(
        PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pMhwVeboxSurfaceStateCmdParams);

    MOS_STATUS SetVeboxDiIecp(
        PMOS_COMMAND_BUFFER                pCmdBufferInUse);

protected:

    // Execution state
    VpVeboxRenderData           *m_lastExecRenderData     = nullptr;                             //!< Cache last render operation info

    VPHAL_CSPACE                m_CscOutputCspace = {};                            //!< Cspace of Output Frame
    VPHAL_CSPACE                m_CscInputCspace = {};                             //!< Cspace of Input frame
    float                       m_fCscCoeff[9];                                    //!< [3x3] Coeff matrix for CSC
    float                       m_fCscInOffset[3];                                 //!< [3x1] Input Offset matrix for CSC
    float                       m_fCscOutOffset[3];                                //!< [3x1] Output Offset matrix for CSC
    SfcRenderBase               *m_sfcRender             = nullptr;
    bool                        m_IsSfcUsed              = false;

    VEBOX_PACKET_SURFACE_PARAMS m_veboxPacketSurface = {};

    VP_SURFACE                  *m_currentSurface           = nullptr;              //!< Current frame
    VP_SURFACE                  *m_previousSurface          = nullptr;              //!< Previous frame
    VP_SURFACE                  *m_renderTarget             = nullptr;              //!< Render Target frame
    VP_SURFACE                  *m_originalOutput           = nullptr;              //!< Render Target frame

    uint32_t                    m_dwGlobalNoiseLevelU = 0;                        //!< Global Noise Level for U
    uint32_t                    m_dwGlobalNoiseLevelV = 0;                        //!< Global Noise Level for V
    uint32_t                    m_dwGlobalNoiseLevel = 0;                         //!< Global Noise Level
    PVP_VEBOX_CACHE_CNTL        m_surfMemCacheCtl = nullptr;                      //!< Surface memory cache control
    uint32_t                    m_DIOutputFrames = MEDIA_VEBOX_DI_OUTPUT_CURRENT; //!< default value is 2 for non-DI case.

    // Statistics
    uint32_t                    m_dwVeboxPerBlockStatisticsWidth = 0;             //!< Per block statistics width
    uint32_t                    m_dwVeboxPerBlockStatisticsHeight = 0;            //!< Per block statistics height

    // STE factor LUT
    static const uint32_t       m_satP1Table[MHW_STE_FACTOR_MAX + 1];
    static const uint32_t       m_satS0Table[MHW_STE_FACTOR_MAX + 1];
    static const uint32_t       m_satS1Table[MHW_STE_FACTOR_MAX + 1];

    MediaScalability           *m_scalability              = nullptr;            //!< scalability
    bool                        m_useKernelResource        = false;               //!< Use Vebox Kernel Resource 
    uint32_t                    m_inputDepth               = 0;
    std::shared_ptr<mhw::vebox::Itf> m_veboxItf            = nullptr;
    MediaFeatureManager        *m_featureManager           = nullptr;
    std::shared_ptr<mhw::mi::Itf> m_miItf                  = nullptr;
    vp::VpUserFeatureControl   *m_vpUserFeatureControl     = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCmdPacket)
};

}
#endif // !__VP_VEBOX_CMD_PACKET_H__
