/*
* Copyright (c) 2018, Intel Corporation
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
#ifndef __VP_VEBOX_CMD_PACKET_H__
#define __VP_VEBOX_CMD_PACKET_H__

#include "mhw_vebox_g12_X.h"
#include "vp_cmd_packet.h"
#include "vp_vebox_common.h"
#include "vp_render_sfc_base.h"
#include "vp_filter.h"

#define MOVE_TO_HWFILTER // remove the macro after finished code refactor

#define VP_VEBOX_MAX_SLICES          4
#define VP_MAX_NUM_FFDI_SURFACES     4                                       //!< 2 for ADI plus additional 2 for parallel execution on HSW+
#define VP_NUM_FFDN_SURFACES         2                                       //!< Number of FFDN surfaces
#define VP_NUM_STMM_SURFACES         2                                       //!< Number of STMM statistics surfaces
#define VP_DNDI_BUFFERS_MAX          4                                       //!< Max DNDI buffers
#define VP_NUM_KERNEL_VEBOX          8                                       //!< Max kernels called at Adv stage

#define VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE                (256 * 4)
#define VP_VEBOX_ACE_HISTOGRAM_SIZE_PER_FRAME_PER_SLICE      (256 * 4)

#define VP_VEBOX_RGB_HISTOGRAM_SIZE                      (VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                          VP_NUM_RGB_CHANNEL                    * \
                                                          VP_VEBOX_MAX_SLICES)

#define VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED          (3072 * 4)

#define VP_VEBOX_LACE_HISTOGRAM_256_BIN_PER_BLOCK          (256 * 2)
#define VP_NUM_RGB_CHANNEL                   3
#define VP_NUM_FRAME_PREVIOUS_CURRENT        2

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

#define VP_VEBOX_MAX_SLICES                              4

#define VP_VEBOX_RGB_HISTOGRAM_SIZE                      (VP_VEBOX_RGB_HISTOGRAM_SIZE_PER_SLICE * \
                                                          VP_NUM_RGB_CHANNEL                    * \
                                                          VP_VEBOX_MAX_SLICES)
#define VP_VEBOX_RGB_ACE_HISTOGRAM_SIZE_RESERVED         (3072 * 4)

//!
//! \brief Vebox Statistics Surface definition for TGL
//!
#define VP_VEBOX_STATISTICS_SIZE                          (32 * 8)
#define VP_VEBOX_STATISTICS_PER_FRAME_SIZE                (32 * sizeof(uint32_t))
#define VP_VEBOX_STATISTICS_SURFACE_FMD_OFFSET            0
#define VP_VEBOX_STATISTICS_SURFACE_GNE_OFFSET            0x2C
#define VP_VEBOX_STATISTICS_SURFACE_STD_OFFSET            0x44

//!
//! \brief Temporal Denoise Definitions
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
typedef struct _VPHAL_SAMPLER_STATE_DNDI_PARAM
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
    bool      bDNEnable;
    bool      bDIEnable;
    bool      bDNDITopFirst;
    bool      bProgressiveDN;
    uint32_t  dwFMDFirstFieldCurrFrame;
    uint32_t  dwFMDSecondFieldPrevFrame;
} VPHAL_SAMPLER_STATE_DNDI_PARAM, *PVPHAL_SAMPLER_STATE_DNDI_PARAM;

typedef struct _VEBOX_PACKET_SURFACE_PARAMS
{
    PVPHAL_SURFACE                  pCurrInput;
    PVPHAL_SURFACE                  pPrevInput;
    PVPHAL_SURFACE                  pSTMMInput;
    PVPHAL_SURFACE                  pSTMMOutput;
    PVPHAL_SURFACE                  pDenoisedCurrOutput;
    PVPHAL_SURFACE                  pCurrOutput;
    PVPHAL_SURFACE                  pPrevOutput;
    PVPHAL_SURFACE                  pStatisticsOutput;
    PVPHAL_SURFACE                  pAlphaOrVignette;
    PVPHAL_SURFACE                  pLaceOrAceOrRgbHistogram;
    PVPHAL_SURFACE                  pSurfSkinScoreOutput;
}VEBOX_PACKET_SURFACE_PARAMS, *PVEBOX_PACKET_SURFACE_PARAMS;

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
//! \file     vp_vebox_cmd_packet.h
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

namespace vp {

class VpVeboxCmdPacket : public VpCmdPacket
{
public:
    VpVeboxCmdPacket(MediaTask * task, PVP_MHWINTERFACE hwInterface, PVpAllocator &allocator, VPMediaMemComp *mmc);

    virtual ~VpVeboxCmdPacket();

    virtual MOS_STATUS Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase = otherPacket) override;

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Destory() { return MOS_STATUS_SUCCESS; };

    virtual MOS_STATUS Prepare() override { return MOS_STATUS_SUCCESS; };

    virtual MOS_STATUS                  AllocateExecRenderData()
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        if (!m_lastExecRenderData)
        {
            m_lastExecRenderData = MOS_New(VP_VEBOX_RENDER_DATA);
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

    virtual PVPHAL_VEBOX_RENDER_DATA    GetLastExecRenderData()
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
           return GetLastExecRenderData()->bIECP;
        }
        return false;
    }

    //!
    //! \brief    Setup surface states for Vebox
    //! \details  Setup surface states for use in the current Vebox Operation
    //! \param    [in] bDiVarianceEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxSurfaceStateCmdParams
    //!           Pointer to VEBOX_SURFACE_STATE command parameters
    //! \return   void
    //!
    virtual void SetupSurfaceStates(
        bool                        bDiVarianceEnable,
        PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS  pVeboxSurfaceStateCmdParams);

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
    MOS_STATUS InitCmdBufferWithVeParams(
        PRENDERHAL_INTERFACE                    pRenderHal,
        MOS_COMMAND_BUFFER                      &CmdBuffer,
        PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams);

    MOS_STATUS InitSfcStateParams();

    //!
    //! \brief    Setup Scaling Params for Vebox/SFC
    //! \details  Setup surface Scaling Params for Vebox/SFC
    //! \param    [in] scalingParams
    //!           Scaling Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetScalingParams(PSFC_SCALING_PARAMS scalingParams);

    //!
    //! \brief    Setup CSC Params for Vebox/SFC
    //! \details  Setup surface CSC Params for Vebox/SFC
    //! \param    [in] cscParams
    //!           CSC/IEF Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetSfcCSCParams(PSFC_CSC_PARAMS cscParams);

    //!
    //! \brief    Setup Roattion/Mirror Params for Vebox/SFC
    //! \details  Setup surface Roattion/Mirror Params for Vebox/SFC
    //! \param    [in] rotMirParams
    //!           Rotation/Mirror Params
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetSfcRotMirParams(PSFC_ROT_MIR_PARAMS rotMirParams);

    MOS_STATUS SetDNPacketParams(
        PVEBOX_DN_PARAMS        pDNParams);

    MOS_STATUS InitSTMMHistory(int32_t iSurfaceIndex);

    //!
    //! \brief    Vebox Populate VEBOX parameters
    //! \details  Populate the Vebox VEBOX state parameters to VEBOX RenderData
    //! \param    [in] pLumaParams
    //!           Pointer to Luma DN and DI parameter
    //! \param    [in] pChromaParams
    //!           Pointer to Chroma DN parameter
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS PopulateDNDIParams(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams);

    //!
    //! \brief    Set DI output frame
    //! \details  Choose 2nd Field of Previous frame or 1st Field of Current frame
    //!           or both frames
    //! \param    [in] pRenderData
    //!           Pointer to Render data
    //! \param    [in] pVeboxState
    //!           Pointer to Vebox State
    //! \param    [in] pVeboxMode
    //!           Pointer to Vebox Mode
    //! \return   GFX_MEDIA_VEBOX_DI_OUTPUT_MODE
    //!           Return Previous/Current/Both frames
    //!
    virtual GFX_MEDIA_VEBOX_DI_OUTPUT_MODE SetDIOutputFrame(
        PVPHAL_VEBOX_RENDER_DATA pRenderData,
        PMHW_VEBOX_MODE          pVeboxMode);

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
    MOS_STATUS GetStatisticsSurfaceOffsets(
        int32_t*                            pStatSlice0Offset,
        int32_t*                            pStatSlice1Offset);

    //!
    //! \brief    Vebox Set FMD parameter
    //! \details  Set up the FMD parameters for DNDI State
    //! \param    [out] pLumaParams
    //!           Pointer to DNDI Param for set FMD parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetFMDParams(
        PVPHAL_SAMPLER_STATE_DNDI_PARAM     pLumaParams);

    //!
    //! \brief    Vebox Set VEBOX parameter
    //! \details  Set up the VEBOX parameter value
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetVeboxDNDIParams(PVPHAL_SURFACE pSrcSurface);

    //!
    //! \brief    Setup Vebox_State Command parameter
    //! \param    [in] bDiVarianceEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxStateCmdParams
    //!           Pointer to VEBOX_STATE command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupVeboxState(
        bool                        bDiVarianceEnable,
        PMHW_VEBOX_STATE_CMD_PARAMS pVeboxStateCmdParams);

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
        PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams);

    //!
    //! \brief    Setup Vebox_DI_IECP Command params for VEBOX final output surface on G9
    //! \details  Setup Vebox_DI_IECP Command params for VEBOX final output surface on G9
    //! \param    [in] bDiScdEnable
    //!           Is DI/Variances report enabled
    //! \param    [in,out] pVeboxDiIecpCmdParams
    //!           Pointer to VEBOX_DI_IECP command parameters
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful
    //!                  MOS_STATUS_UNIMPLEMENTED, if condition not implemented,
    //!                  otherwise failed
    //!
    MOS_STATUS SetupDiIecpStateForOutputSurf(
        bool                            bDiScdEnable,
        PMHW_VEBOX_DI_IECP_CMD_PARAMS   pVeboxDiIecpCmdParams);

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
    void SetSfcOutputPath(bool bSfcUsed) { m_IsSfcUsed = bSfcUsed; };

    virtual SfcRenderBase* GetSfcRenderInstance() { return m_sfcRender; };

    virtual MOS_STATUS SetupVeboxRenderMode0(
        PVPHAL_SURFACE           pSrcSurface,
        PVPHAL_SURFACE           pOutputSurface);

    // Need to remove vphal surface dependence from VpCmdPacket later.
    virtual MOS_STATUS PacketInit(
        PVPHAL_SURFACE      pSrcSurface,
        PVPHAL_SURFACE      pOutputSurface,
        VP_EXECUTE_CAPS     packetCaps) override;

    //!
    //! \brief    Copy and update vebox state
    //! \details  Copy and update vebox state for input frame.
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyAndUpdateVeboxState(
        PVPHAL_SURFACE           pSrcSurface);

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
        const MHW_VEBOX_STATE_CMD_PARAMS        &VeboxStateCmdParams,
        const MHW_VEBOX_DI_IECP_CMD_PARAMS      &VeboxDiIecpCmdParams);

    virtual MOS_STATUS SetDNDIParams(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SAMPLER_STATE_DNDI_PARAM pLumaParams,
        PVPHAL_DNUV_PARAMS              pChromaParams) { return MOS_STATUS_SUCCESS;}

    //!
    //! \brief    Copy Vebox state heap
    //! \details  Call HW interface function,
    //!           use Secure_Block_Copy kernel,
    //!           copy Vebox state heap between different memory
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyVeboxStates();

    //!
    //! \brief    Vebox state heap update for auto mode features
    //! \details  Update Vebox indirect states for auto mode features
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UpdateVeboxStates(
        PVPHAL_SURFACE              pSrcSurface);

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
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS VeboxSetPerfTag(
       MOS_FORMAT            srcFmt);

protected:

    //!
    //! \brief    Doing prepare stage tasks for SendVeboxCmd
    //!           Parameters might remain unchanged in case
    //! \param    [out] CmdBuffer
    //!           reference to Cmd buffer control struct
    //! \param    [out] GenericPrologParams
    //!           Generic prolog params struct to be set
    //! \param    [out] GpuStatusBuffer
    //!           GpuStatusBuffer resource to be set
    //! \param    [out] iRemaining
    //!           integer showing initial cmd buffer usage
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
   virtual MOS_STATUS PrepareVeboxCmd(
      MOS_COMMAND_BUFFER*                      CmdBuffer,
      RENDERHAL_GENERIC_PROLOG_PARAMS&         GenericPrologParams,
      MOS_RESOURCE&                            GpuStatusBuffer,
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
        MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
        VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
        MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
        MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
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
    //! \param    [in] pMhwMiInterface
    //!           MHW MI interface
    //! \param    [in] pOsInterface
    //!           Pointer to OS Interface
    //! \param    [out] pCmdBuffer
    //!           Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    MOS_STATUS SendVecsStatusTag(
      PMHW_MI_INTERFACE                   pMhwMiInterface,
      PMOS_INTERFACE                      pOsInterface,
      PMOS_COMMAND_BUFFER                 pCmdBuffer);

    MOS_STATUS InitVeboxSurfaceStateCmdParams(
        PVPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
        PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams);

    MOS_STATUS InitVeboxSurfaceParams(
        PVPHAL_SURFACE                 pVpHalVeboxSurface,
        PMHW_VEBOX_SURFACE_PARAMS      pMhwVeboxSurface);

    //!
    //! \brief    Copy Surface value
    //! \param    [in] pTargetSurface
    //!           Pointer to surface copy value to 
    //! \param    [in] pSourceSurface
    //!           Pointer to surface copy value from 
    //! \return   void
    //!
    virtual void CopySurfaceValue(
      PVPHAL_SURFACE              pTargetSurface,
      PVPHAL_SURFACE              pSourceSurface);

    //!
    //! \brief    Vebox allocate resources
    //! \details  Allocate resources that will be used in Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS AllocateResources();

    //!
    //! \brief    Vebox free resources
    //! \details  Free resources that are used in Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    virtual MOS_STATUS FreeResources();

    //!
    //! \brief    Get Output surface params needed when allocate surfaces
    //! \details  Get Output surface params needed when allocate surfaces
    //! \param    [out] Format
    //!           Format of output surface
    //! \param    [out] TileType
    //!           Tile type of output surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if success, otherwise failed
    //!
    MOS_STATUS GetOutputSurfParams(
        MOS_FORMAT    &Format,
        MOS_TILE_TYPE &TileType);

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
    //! \brief    Vebox set up vebox state heap
    //! \details  Setup Vebox indirect states: DNDI and etc
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutSurface
    //!           Pointer to output surface of Vebox
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupIndirectStates(
        PVPHAL_SURFACE                  pSrcSurface,
        PVPHAL_SURFACE                  pOutSurface);

    //!
    //! \brief    Vebox get the back-end colorspace conversion matrix
    //! \details  When the i/o is A8R8G8B8 or X8R8G8B8, the transfer matrix
    //!           needs to be updated accordingly
    //! \param    [in] pSrcSurface
    //!           Pointer to input surface of Vebox
    //! \param    [in] pOutSurface
    //!           Pointer to output surface of Vebox
    //! \return   void
    //!
    virtual void VeboxGetBeCSCMatrix(
        PVPHAL_SURFACE pSrcSurface,
        PVPHAL_SURFACE pOutSurface);

private:

    //!
    //! \brief    Get output surface of Vebox
    //! \details  Get output surface of Vebox in current operation
    //! \param    [in] bDiVarianceEnable
    //!           Is DI/Variances report enabled
    //! \return   PVPHAL_SURFACE
    //!           Corresponding output surface pointer
    //!
    PVPHAL_SURFACE GetSurfOutput(bool   bDiVarianceEnable);

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

protected:

    // Execution state
    PVPHAL_VEBOX_RENDER_DATA    m_lastExecRenderData     = nullptr;                             //!< Cache last render operation info

    VPHAL_CSPACE                m_CscOutputCspace = {};                            //!< Cspace of Output Frame
    VPHAL_CSPACE                m_CscInputCspace = {};                             //!< Cspace of Input frame
    float                       m_fCscCoeff[9];                               //!< [3x3] Coeff matrix for CSC
    float                       m_fCscInOffset[3];                            //!< [3x1] Input Offset matrix for CSC
    float                       m_fCscOutOffset[3];                           //!< [3x1] Output Offset matrix for CSC
    SfcRenderBase               *m_sfcRender             = nullptr;
    VPHAL_SFC_RENDER_DATA       m_sfcRenderData          = {};
    bool                        m_IsSfcUsed              = false;
    uint32_t                    m_histogramSurfaceOffset = 0;                                 //!< Vebox Histogram Surface Offset

    VEBOX_PACKET_SURFACE_PARAMS m_veboxPacketSurface = {};

#ifdef MOVE_TO_HWFILTER
    // Resources
    VPHAL_SURFACE               m_veboxStatisticsSurface = {};                                 //!< Statistics Surface for VEBOX
    VPHAL_SURFACE               *m_currentSurface        = nullptr;                            //!< Current frame
    VPHAL_SURFACE               *m_previousSurface       = nullptr;                            //!< Previous frame
    VPHAL_SURFACE               *m_renderTarget          = nullptr;                            //!< Render Target frame
    VPHAL_SURFACE               *FFDISurfaces[VP_MAX_NUM_FFDI_SURFACES] = {};  //!< FFDI output surface structure
    VPHAL_SURFACE               *FFDNSurfaces[VP_NUM_FFDN_SURFACES] = {};      //!< Denoise output surface.
    VPHAL_SURFACE               STMMSurfaces[VP_NUM_STMM_SURFACES] = {};        //!< Motion history (DI)
    int32_t                     m_iCurDNIndex = 0;                                //!< Current index of Denoise Output
    // DNDI
    struct
    {
        int32_t                     m_iNumFFDISurfaces = 2;                       //!< Actual number of FFDISurfaces. Is <= VPHAL_NUM_FFDI_SURFACES PE on: 4 used. PE off: 2 used
        int32_t                     m_iCurStmmIndex = 0;                          //!< Current index of Motion History Buffer
    };
    // Chroma DN
    struct
    {
        int32_t                     m_iCurHistIndex = 0;                          //!< Current index of Chroma Denoise History Buffer
    };
    // timestamps for DI output control
    int32_t                         m_iCurFrameID = 0;                            //!< Current Frame ID
    int32_t                         m_iPrvFrameID = 0;                            //!< Previous Frame ID
    // S3D channel
    uint32_t                    m_uiCurrentChannel = 0;                           //!< 0=StereoLeft or nonStereo, 1=StereoRight. N/A in nonStereo
#endif
    bool                        m_bRefValid = false;
    uint32_t                    m_dwGlobalNoiseLevelU = 0;                        //!< Global Noise Level for U
    uint32_t                    m_dwGlobalNoiseLevelV = 0;                        //!< Global Noise Level for V
    bool                        m_bFirstFrame = false;                            //!< First frame case for Chroma DN
    uint32_t                    m_dwGlobalNoiseLevel = 0;                         //!< Global Noise Level
    VPHAL_DNDI_CACHE_CNTL       m_DnDiSurfMemObjCtl = {};                         //!< Surface memory object control
    uint32_t                    m_DIOutputFrames = MEDIA_VEBOX_DI_OUTPUT_CURRENT; //!< default value is 2 for non-DI case.

    // Statistics
    uint32_t                    m_dwVeboxPerBlockStatisticsWidth = 0;             //!< Per block statistics width
    uint32_t                    m_dwVeboxPerBlockStatisticsHeight = 0;            //!< Per block statistics height

};

}
#endif // !__VP_VEBOX_CMD_PACKET_H__
