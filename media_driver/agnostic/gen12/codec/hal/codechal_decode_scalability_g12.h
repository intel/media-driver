/*
// Copyright (C) 2018-2019, Intel Corporation
//
// Licensed under the Apache License,Version 2.0(the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/
//!
//! \file     codechal_decode_scalability_g12.h
//! \brief    Impelements the public interface for Gen12 Scalability Decode 
//!
#ifndef __CODECHAL_DECODE_SCALABILITY_G12_H__
#define __CODECHAL_DECODE_SCALABILITY_G12_H__


#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_hw_g12_X.h"
#include "codechal_decoder.h"
#include "mos_os_virtualengine_scalability.h"
#include "codechal_decode_scalability.h"

#define CODECHAL_HCP_DECODE_PHASE_REAL_TILE  0xFF

#define CODECHAL_SCALABILITY_SLICE_STATE_CACHELINES_PER_SLICE_TGL  9

inline static uint8_t CodecHalDecodeMaxNumPipesInUseG12(uint8_t vdboxNum)
{
    uint8_t maxNumPipesInUs = 1;
    if (vdboxNum == 2)
    {
        maxNumPipesInUs = 2;
    }
    else 
    {
        maxNumPipesInUs = vdboxNum - 1;
    }
    return maxNumPipesInUs;
}

//!
//! \enum   HCP_RT_PHASE_INDICATOR
//! \brief  Phase indicator for HEVC multiple phase
//!
typedef enum
{
    HCP_RT_FIRST_PHASE = 0,                                //!< First phase
    HCP_RT_MIDDLE_PHASE = 1,                                //!< Middle phase
    HCP_RT_LAST_PHASE = 2                                 //!< Last phase
} HCP_RT_PHASE_INDICATOR;

typedef struct _CODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12 : public _CODECHAL_DECODE_SCALABILITY_INIT_PARAMS
{
    bool             bIsTileEnabled;                 //!< The picture can be partitioned into tiles
    bool             bIsSccDecoding;                 //!< Codec is HEVC SCC decoding
}CODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12, *PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS_G12;

typedef struct _CODECHAL_DECODE_SFC_SCALABILITY_PARAMS
{
    // HCP-SFC pipe only for scalability and more input/output color format
    uint32_t                        engineMode;                                 //!< 0 - single, 1 - left most column, 2 - right most column, 3 - middle column
    uint32_t                        tileType;                                   //!< Real tile = 0, virtual tile = 1
    uint32_t                        srcStartX;                                  //!< Source surface column horizontal start position in pixel
    uint32_t                        srcEndX;                                    //!< Source surface column horizontal end position in pixel
    uint32_t                        dstStartX;                                  //!< Output surface column horizontal start position in pixel
    uint32_t                        dstEndX;                                    //!< Output surface column horizontal end position in pixel
}CODECHAL_DECODE_SFC_SCALABILITY_PARAMS, *PCODECHAL_DECODE_SFC_SCALABILITY_PARAMS;

typedef struct _CODECHAL_DECODE_SCALABILITY_STATE_G12 : public _CODECHAL_DECODE_SCALABILITY_STATE
{
    // For hevc real tile decoding
    bool                            bIsRtMode;
    uint8_t                         u8RtCurPipe;
    uint8_t                         u8RtCurPhase;
    uint8_t                         u8RtPhaseNum;
    uint8_t                         u8RtPipeInLastPhase;

#if (_DEBUG || _RELEASE_INTERNAL)
    bool                            bDisableRtMode;
    bool                            bEnableRtMultiPhase;
    uint8_t                         dbgOverUserPipeNum;
#endif
}CODECHAL_DECODE_SCALABILITY_STATE_G12, *PCODECHAL_DECODE_SCALABILITY_STATE_G12;

#define CodecHalDecodeScalablity_SetPhaseIndicator(PhaseIndicator)\
{                                                                                                                      \
    if (m_scalabilityState->u8RtCurPhase == 0)                                                                         \
    {                                                                                                                  \
        PhaseIndicator = HCP_RT_FIRST_PHASE;                                                                           \
    }                                                                                                                  \
    else if (m_scalabilityState->u8RtCurPhase == m_scalabilityState->u8RtPhaseNum - 1)                                 \
    {                                                                                                                  \
        PhaseIndicator = HCP_RT_LAST_PHASE;                                                                            \
    }                                                                                                                  \
    else                                                                                                               \
    {                                                                                                                  \
        PhaseIndicator = HCP_RT_MIDDLE_PHASE;                                                                          \
    }                                                                                                                  \
}while (0)

#define CodecHalDecodeScalabilityIsRealTileMode(pScalabilityState) \
    (pScalabilityState ? (pScalabilityState->bScalableDecodeMode && pScalabilityState->bIsRtMode): false)

#define CodecHalDecodeScalabilityIsVirtualTileMode(pScalabilityState) \
    (pScalabilityState ? (pScalabilityState->bScalableDecodeMode && !pScalabilityState->bIsRtMode): false)

#define CodecHalDecodeScalabilityIsRealTilePhase(pScalabilityState)                 \
    (pScalabilityState && pScalabilityState->bScalableDecodeMode &&                 \
    (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE))

#define CodecHalDecodeScalabilityIsFirstRealTilePhase(pScalabilityState)            \
    (CodecHalDecodeScalabilityIsRealTilePhase(pScalabilityState) &&                 \
    (pScalabilityState->u8RtCurPhase == 0))

#define CodecHalDecodeScalabilityIsLastRealTilePhase(pScalabilityState)             \
    (CodecHalDecodeScalabilityIsRealTilePhase(pScalabilityState) &&                 \
    ((pScalabilityState->u8RtCurPhase == pScalabilityState->u8RtPhaseNum - 1) ||    \
    ((pScalabilityState->u8RtCurPipe >= pScalabilityState->u8RtPipeInLastPhase) &&  \
    (pScalabilityState->u8RtCurPhase == pScalabilityState->u8RtPhaseNum - 2))))

#define CodecHalDecodeScalabilityIsLastRealTilePass(pScalabilityState)              \
    (CodecHalDecodeScalabilityIsRealTilePhase(pScalabilityState) &&                 \
    (pScalabilityState->u8RtCurPhase == pScalabilityState->u8RtPhaseNum - 1) &&     \
    (pScalabilityState->u8RtCurPipe == pScalabilityState->u8RtPipeInLastPhase - 1))

#define CodecHalDecodeScalabilityIsBEPhaseG12(pScalabilityState)               \
    (pScalabilityState && pScalabilityState->bScalableDecodeMode &&            \
     ((pScalabilityState->HcpDecPhase >= CODECHAL_HCP_DECODE_PHASE_BE0) &&     \
     (pScalabilityState->HcpDecPhase != CODECHAL_HCP_DECODE_PHASE_REAL_TILE)))

#define CodecHalDecodeScalabilityIsFinalBEPhaseG12(pScalabilityState)                                                       \
     (pScalabilityState && pScalabilityState->bScalableDecodeMode &&                                                        \
       ((pScalabilityState->HcpDecPhase >= CODECHAL_HCP_DECODE_PHASE_BE0) &&                                                \
        (pScalabilityState->HcpDecPhase != CODECHAL_HCP_DECODE_PHASE_REAL_TILE)) &&                                         \
       (pScalabilityState->ucScalablePipeNum == pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE))

#define CodecHalDecodeScalablity_DecPhaseToHwWorkMode_G12(EngineMode, PipeWorkMode)\
do                                                                                                                      \
{                                                                                                                       \
    if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_REAL_TILE)                                                           \
    {                                                                                                                   \
        PipeWorkMode = MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_REAL_TILE;                                                    \
        if (m_scalabilityState->u8RtCurPipe == 0)                                                                       \
        {                                                                                                               \
            if ((m_scalabilityState->u8RtCurPhase == m_scalabilityState->u8RtPhaseNum - 1) &&                           \
                (m_scalabilityState->u8RtPipeInLastPhase == 1))                                                         \
                EngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;                                                 \
            else                                                                                                        \
                EngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;                                                      \
        }                                                                                                               \
        else if (m_scalabilityState->u8RtCurPipe == m_scalabilityState->ucScalablePipeNum - 1)                          \
            EngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;                                                         \
        else                                                                                                            \
            EngineMode = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;                                                        \
    }                                                                                                                   \
    else if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE)                                                             \
    {                                                                                                                   \
        EngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;                                                     \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_FE;                                                         \
    }                                                                                                                   \
    else if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0)                                                            \
    {                                                                                                                   \
        EngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;                                                          \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;                                                         \
    }                                                                                                                   \
    else                                                                                                                \
    {                                                                                                                   \
        if(((m_hcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE) <= m_scalabilityState->ucScalablePipeNum) &&                 \
           (m_hcpDecPhase > CODECHAL_HCP_DECODE_PHASE_BE0))                                                             \
        {                                                                                                               \
            CODECHAL_DECODE_ASSERT(m_scalabilityState->ucScalablePipeNum > 2);                                          \
            EngineMode    = (m_scalabilityState->ucScalablePipeNum == (m_hcpDecPhase - CODECHAL_HCP_DECODE_PHASE_FE)) ? \
                              MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT : MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;           \
            PipeWorkMode  = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;                                                      \
        }                                                                                                               \
    }                                                                                                                   \
}while (0)

//!
//! \brief    Get secondary cmd buffer
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pSdryCmdBuf
//!                secondary cmd buffer address
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_GetVESecondaryCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState,
    PMOS_COMMAND_BUFFER                pSdryCmdBuf);

//!
//! \brief    get command buffer to use
//! \details  decide and get command buffer to add cmds. it is for decoder which can support both scalability and single pipe
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in]  pScdryCmdBuf
//!                pointer to secondary cmd buffer
//! \param    [in]  ppCmdBufToUse
//!                pointer to cmd buffer to use
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_GetCmdBufferToUse_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pScdryCmdBuf,
    PMOS_COMMAND_BUFFER                 *ppCmdBufToUse);

//!
//! \brief    return secondary cmd buffer
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in]  pScdryCmdBuf
//!                pointer to secondary cmd buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_ReturnSdryCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pSdryCmdBuf);

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief    dump command buffer in scalability mode
//! \param    [in]  pDecoder
//!                Decoder device
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_DbgDumpCmdBuffer_G12(
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalDebugInterface              *debugInterface,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf);
#endif

//!
//! \brief    Determine decode phase
//! \details  determine decode phase for decoder supporting scalability mode but not necessarily always running in scalable mode
//! \param    [in] pScalabilityState
//!                Scalability decode state
//! \param    [in] pHcpDecPhase
//!                Address of hcp decode phase
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_DetermineDecodePhase_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint32_t                            *pHcpDecPhase);

//!
//! \brief    Initiliaze Decode Parameters for virtual engine decode
//! \details  Initiliaze decode parameters for virtual engine decode. this is for decoder supporting scalability but not necessarily always running in scalable mode
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pInitParams
//!                pointer to parameters to initialize decode scalability
//! \param    [in] pucDecPassNum
//!                pointer to decode pass number
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_InitScalableParams_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams,
    uint16_t                                   *pucDecPassNum);

//!
//! \brief     Set virtual engine hint parameters for scalable decode
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pSetHintParms
//!                pointer to set hint parameter
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_SetHintParams_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_DECODE_SCALABILITY_SETHINT_PARMS pSetHintParms);

//!
//! \brief    Sync between FE and BE
//! \details  This function does 3 major things
//!              1) send hw or sw semaphore wait at start of BE0 cmd.
//!              2) use HW semaphore wait and MI ATOMIC cmd to make all BEs start running at the same time
//!              3) add COND BB END cmd to check if CABAC stream out buffer overflow.
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_FEBESync_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse,
    bool phasedSubmission);

//!
//! \brief    To judge if command buffer should be submitted.
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \return   bool
//!           True means to submit command buffer, False means not to submit.
//!
bool CodecHalDecodeScalabilityIsToSubmitCmdBuffer_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState);

//!
//! \brief    Read CS ENGINEID register to know which engine is in use for current workload
//! \param    [in]  pDecodeStatusBuf
//!                Decode status buffer
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_ReadCSEngineIDReg_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState,
    CodechalDecodeStatusBuffer         *pDecodeStatusBuf,
    PMOS_COMMAND_BUFFER                pCmdBufferInUse);

//!
//! \brief    State initialization for virtual engine decode supporting scalable and single pipe mode
//! \param    [in]  pDecoder
//!                Decoder device
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in]  bShortFormat
//!                short format decode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_InitializeState_G12(
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalHwInterface                 *hwInterface,
    bool                                bShortFormat);

//!
//! \brief    State initialization for virtual engine decode supporting scalable and single pipe mode
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_AdvanceRealTilePass(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState);

//!
//! \brief    Get curent tile column index in real tile decoding
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [out] col
//!                reference to column id
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_GetCurrentRealTileColumnId(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint8_t                             &col);

//! \brief    Set scalability parameters in SFC state
//! \param    [in]  scalabilityState
//!                Scalability decode state
//! \param    [in]  picParams
//!                Picture parameters for HEVC or VP9
//! \param    [in]  srcRegion
//!                SFC input surface region
//! \param    [in]  dstRegion
//!                SFC output surface region
//! \param    [out]  sfcScalabilityParams
//!                Pointer to SFC Scalability state parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_SetSfcState(
    PCODECHAL_DECODE_SCALABILITY_STATE          scalabilityState,
    void                                       *picParams,
    CODECHAL_RECTANGLE                         *srcRegion,
    CODECHAL_RECTANGLE                         *dstRegion,
    PCODECHAL_DECODE_SFC_SCALABILITY_PARAMS     sfcScalabilityParams);

MOS_STATUS CodecHalDecodeScalability_AllocateResources_VariableSizes_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE        pScalabilityState,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS         pHcpBufSizeParam,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS      pAllocParam);

MOS_STATUS CodecHalDecodeScalability_DecidePipeNum_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams);

MOS_STATUS CodechalDecodeScalability_MapPipeNumToLRCACount_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalState,
    uint32_t                             *LRCACount);

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS CodechalDecodeScalability_DebugOvrdDecidePipeNum_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState);
#endif

void CodecHalDecodeScalability_DecPhaseToSubmissionType_G12(
    PCODECHAL_DECODE_SCALABILITY_STATE_G12 pScalabilityState,
    PMOS_COMMAND_BUFFER pCmdBuffer);

#endif // __CODECHAL_DECODE_SCALABILITY_G12_H__
