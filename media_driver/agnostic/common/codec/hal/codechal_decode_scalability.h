/*
* Copyright (c) 2016-2019, Intel Corporation
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
//! \file     codechal_decode_scalability.h
//! \brief    Defines the decode interface extension for scalability.
//! \details  Defines all types, macros, and functions required by CodecHal for virtual engine decode supporting both single pipe and scalable mode. Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_SCALABILITY_H__
#define __CODECHAL_DECODER_SCALABILITY_H__

#include "codechal.h"
#include "codechal_hw.h"
#include "codechal_decoder.h"
#include "mos_os_virtualengine_scalability.h"

#define CODEC_VTILE_MAX_NUM  8

// first tile column width

#define CODEC_SCALABILITY_FIRST_TILE_WIDTH_4K 2048
#define CODEC_SCALABILITY_FIRST_TILE_WIDTH_8K 4096

#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD1_WIDTH            3840
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD2_WIDTH            3996
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_WIDTH            5120
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_WIDTH            7680
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD1_HEIGHT           1440
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD2_HEIGHT           1716
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_HEIGHT           2160
#define CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_HEIGHT           4320

inline static bool CodechalDecodeResolutionEqualLargerThan4k(uint32_t width, uint32_t height)
{
    return (((width * height) >= (CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD1_WIDTH * CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_HEIGHT))
        || ((width >= CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD1_WIDTH) && (height >= CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD2_HEIGHT)));
}

inline static bool CodechalDecodeResolutionEqualLargerThan5k(uint32_t width, uint32_t height)
{
    return (((width * height) >= (CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_WIDTH * CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_HEIGHT))
        || ((width >= CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD3_WIDTH) && (height >= CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD1_HEIGHT)));
}

inline static bool CodechalDecodeNonRextFormat(MOS_FORMAT format)
{
    return ((format == Format_NV12) || (format == Format_P010));
}

//!
//! \enum   CODECHAL_HCP_DECODE_SCALABLE_PHASE
//! \brief  extended HCP decode phase for scalability mode
//!
typedef enum
{
    CODECHAL_HCP_DECODE_PHASE_FE = CodechalDecode::CodechalHcpDecodePhaseMax,   //!< HCP decode virtual tile frontend phase
    CODECHAL_HCP_DECODE_PHASE_BE0,                                              //!< HCP decode virutal tile backend pipe0 phase
    CODECHAL_HCP_DECODE_PHASE_BE1,                                              //!< HCP decode virutal tile backend pipe1 phase
    CODECHAL_HCP_DECODE_PHASE_RESERVED,                                         //!< HCP decode reserved phase
    CODECHAL_HCP_DECODE_SCALABLE_PHASE_MAX                                      //!< HCP decode maximal phase in scalability mode
}CODECHAL_HCP_DECODE_SCALABLE_PHASE;

static const int CODECHAL_HCP_STREAMOUT_BUFFER_MAX_NUM = 2;

static const int CODECHAL_HCP_DECODE_SCALABLE_MAX_PHASE_NUM = (CODECHAL_HCP_DECODE_PHASE_BE1 + 1 - CODECHAL_HCP_DECODE_PHASE_FE + 1);

typedef struct _CODECHAL_DECODE_SCALABILITY_FE_STATUS
{
    uint64_t       dwCarryFlagOfReportedSizeMinusAllocSize;    //!< Carry flag of reported stream out size minus allocated size
} CODECHAL_DECODE_SCALABILITY_FE_STATUS, *PCODECHAL_DECODE_SCALABILITY_FE_STATUS;

typedef struct _CODECHAL_DECODE_SCALABILITY_FE_CABAC_STREAMOUT_BUFF_SIZE
{
    uint64_t       dwCabacStreamoutBuffSize;    //!< Cabac Streamout buff size
} CODECHAL_DECODE_SCALABILITY_FE_CABAC_STREAMOUT_BUFF_SIZE, *PCODECHAL_DECODE_SCALABILITY_FE_CABAC_STREAMOUT_BUFF_SIZE;

typedef struct _CODECHAL_DECODE_SCALABILITY_SETHINT_PARMS
{
    bool       bSameEngineAsLastSubmission;
    bool       bNeedSyncWithPrevious;
    bool       bSFCInUse;
}CODECHAL_DECODE_SCALABILITY_SETHINT_PARMS, *PCODECHAL_DECODE_SCALABILITY_SETHINT_PARMS;

typedef struct _CODECHAL_DECODE_SCALABILITY_INIT_PARAMS
{
    uint32_t         u32PicWidthInPixel;             //!< Picture width in pixel align to minimal coding block
    uint32_t         u32PicHeightInPixel;            //!< Picture height in pixel align to minimal coding block
    MOS_FORMAT       format;                         //!< Surface format
    bool             usingSFC;
    uint8_t          u8NumTileColumns;               //!< Number of tile columns for this picture
    uint8_t          u8NumTileRows;                  //!< Number of tile rows for this picture
    MOS_GPU_CONTEXT  gpuCtxInUse;                    //!< gpu context in use
    bool             usingSecureDecode;
}CODECHAL_DECODE_SCALABILITY_INIT_PARAMS, *PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS;

typedef struct _CODECHAL_DECODE_SCALABILITY_STATE CODECHAL_DECODE_SCALABILITY_STATE, *PCODECHAL_DECODE_SCALABILITY_STATE;

struct _CODECHAL_DECODE_SCALABILITY_STATE
{
    CodechalHwInterface            *pHwInterface;
    uint32_t                        Standard;
    MOS_GPU_CONTEXT                 VideoContextForFE;
    MOS_GPU_CONTEXT                 VideoContext;
    MOS_GPU_CONTEXT                 VideoContextForSP;
    MOS_GPU_CONTEXT                 VideoContextForMP;
    MOS_GPU_CONTEXT                 VideoContextFor3P;
    uint32_t                        HcpDecPhase;
    bool                            bScalableDecodeMode;

    bool                            bFESeparateSubmission;
    bool                            bShortFormatInUse;
    bool                            bUseSecdryCmdBuffer;
    bool                            bIsEvenSplit;
    uint8_t                         ucScalablePipeNum;
    uint8_t                         ucNumVdbox;
    uint32_t                        uiFirstTileColWidth;
    uint32_t                        dwHcpDecModeSwtichTh1Width;
    uint32_t                        dwHcpDecModeSwtichTh2Width;
    MOS_RESOURCE                    resSliceStateStreamOutBuffer;
    MOS_RESOURCE                    resMvUpRightColStoreBuffer;
    MOS_RESOURCE                    resIntraPredUpRightColStoreBuffer;
    MOS_RESOURCE                    resIntraPredLeftReconColStoreBuffer;
    MOS_RESOURCE                    resCABACSyntaxStreamOutBuffer[CODECHAL_HCP_STREAMOUT_BUFFER_MAX_NUM];
    bool                            bToggleCABACStreamOutBuffer;
    PMOS_RESOURCE                   presCABACStreamOutBuffer;
    MOS_RESOURCE                    resSemaMemBEs;
    MOS_RESOURCE                    resSemaMemFEBE;
    MOS_RESOURCE                    resSemaMemCompletion;
    MOS_RESOURCE                    resFEStatusBuffer;
    MOS_RESOURCE                    resFeBeSyncObject;
    MOS_RESOURCE                    resDelayMinus;
    uint32_t                        numDelay;
    uint32_t                        dwCABACSyntaxStreamOutBufferSize;
    bool                            bIsEnableEndCurrentBatchBuffLevel;
#if (_DEBUG || _RELEASE_INTERNAL)
    bool                            bAlwaysFrameSplit;
    uint32_t                        dbgOvrdWidthInMinCb;
#endif

    //For SFC Scalability
    uint32_t                        fistValidTileIndex;
    uint32_t                        lastValidTileIndex;
    uint32_t                        dstXLandingCount;
    uint32_t                        lastDstEndX;
    uint32_t                        sliceStateCLs;

    //Virtual Engine related
    PMOS_VIRTUALENGINE_INTERFACE          pVEInterface;
    PMOS_VIRTUALENGINE_HINT_PARAMS        pScalHintParms;
    PMOS_VIRTUALENGINE_HINT_PARAMS        pSingleHintParms;

    MOS_STATUS(*pfnIsHcpBufferReallocNeeded) (
        CodechalHwInterface                 *pHwInterface,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   BufferType,
        PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS pReallocParam);

    MOS_STATUS (*pfnGetHcpBufferSize) (
        CodechalHwInterface                *pHwInterface,
        MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  dwBufferType,
        PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   pHcpBufSizeParam);

    MOS_STATUS (*pfnDecidePipeNum) (
        PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
        PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams);

    MOS_STATUS(*pfnMapPipeNumToLRCACount)(
        PCODECHAL_DECODE_SCALABILITY_STATE   pScalState,
        uint32_t                             *LRCACount);

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_STATUS (*pfnDebugOvrdDecidePipeNum)(
        PCODECHAL_DECODE_SCALABILITY_STATE         pScalState);
#endif
};

#define CodecHalDecodeScalabilityIsBEPhase(pScalabilityState)                  \
    (pScalabilityState && pScalabilityState->bScalableDecodeMode &&            \
     ((pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0)    ||  \
     (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE1)     ||  \
     (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_RESERVED)))

#define CodecHalDecodeScalabilityIsFEPhase(pScalabilityState)  \
    (pScalabilityState && pScalabilityState->bScalableDecodeMode && pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE)

#define CodecHalDecodeScalabilityIsFinalBEPhase(pScalabilityState)                                                          \
     (pScalabilityState && pScalabilityState->bScalableDecodeMode &&                                                        \
      ((pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0 && pScalabilityState->ucScalablePipeNum == 1)  ||   \
       (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE1 && pScalabilityState->ucScalablePipeNum == 2)  ||   \
       (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_RESERVED &&                                             \
       pScalabilityState->ucScalablePipeNum == CODECHAL_HCP_DECODE_PHASE_RESERVED - CODECHAL_HCP_DECODE_PHASE_FE)))

#define CodecHalDecodeScalablity_DecPhaseToHwWorkMode(EngineMode, PipeWorkMode)\
do                                                                                                                      \
{                                                                                                                       \
    if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE)                                                                  \
    {                                                                                                                   \
        EngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_FE_LEGACY;                                                     \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CABAC_FE;                                                         \
    }                                                                                                                   \
    else if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0)                                                            \
    {                                                                                                                   \
        EngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_LEFT;                                                          \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;                                                         \
    }                                                                                                                   \
    else if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE1)                                                            \
    {                                                                                                                   \
        CODECHAL_DECODE_ASSERT(m_scalabilityState->ucScalablePipeNum >= 2);                                             \
        EngineMode     = (m_scalabilityState->ucScalablePipeNum == 2) ?                                                 \
                              MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT : MHW_VDBOX_HCP_MULTI_ENGINE_MODE_MIDDLE;           \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;                                                         \
    }                                                                                                                   \
    else if (m_hcpDecPhase == CODECHAL_HCP_DECODE_PHASE_RESERVED)                                                       \
    {                                                                                                                   \
        CODECHAL_DECODE_ASSERT(m_scalabilityState->ucScalablePipeNum >= CODECHAL_HCP_DECODE_PHASE_RESERVED - CODECHAL_HCP_DECODE_PHASE_FE);  \
        EngineMode     = MHW_VDBOX_HCP_MULTI_ENGINE_MODE_RIGHT;                                                         \
        PipeWorkMode   = MHW_VDBOX_HCP_PIPE_WORK_MODE_CODEC_BE;                                                         \
    }                                                                                                                   \
}while (0)

#define CodecHalDecodeScalabilityIsScalableMode(pScalabilityState) \
    (pScalabilityState ? pScalabilityState->bScalableDecodeMode : false)

#define CodecHalDecodeScalabilityIsFESeparateSubmission(pScalabilityState) \
    (pScalabilityState ? (pScalabilityState->bScalableDecodeMode && pScalabilityState->bFESeparateSubmission) : false)

#define CodecHalDecodeScalability1stDecPhase(pScalabilityState)\
    (pScalabilityState->HcpDecPhase == CodechalDecode::CodechalHcpDecodePhaseLegacyS2L || \
     (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_FE && !pScalabilityState->bShortFormatInUse))

#define CodecHalDecodeScalabilityIsLastCompletePhase(pScalabilityState)\
    (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0)

#define CodecHalDecodeScalability1stPhaseofSubmission(pScalabilityState)\
    (CodecHalDecodeScalability1stDecPhase(pScalabilityState) || \
     (pScalabilityState->HcpDecPhase == CODECHAL_HCP_DECODE_PHASE_BE0 && pScalabilityState->bFESeparateSubmission))

//HCP Decode pipe number
typedef enum _CODECHAL_DECODE_HCP_SCALABILITY_PIPE_NUM
{
    CODECHAL_DECODE_HCP_Legacy_PIPE_NUM_1 = 1,
    CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_2 = 2,
    CODECHAL_DECODE_HCP_SCALABLE_PIPE_NUM_RESERVED,
    CODECHAL_DECODE_HCP_SCALABLE_MAX_PIPE_NUM
}CODECHAL_DECODE_HCP_SCALABILITY_PIPE_NUM;

#define CODECHAL_SCALABILITY_DECODE_SECONDARY_CMDBUFSET_NUM        16
#define CODECHAL_SCALABILITY_SLICE_STATE_CACHELINES_PER_SLICE      8

//!
//! \brief    Decide pipe num of scalability decode
//! \param    [in] pScalState
//!                pointer to Scalability decode state
//! \param    [in] pInitParams
//!                pointer to initialize parameter
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_DecidePipeNum(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PCODECHAL_DECODE_SCALABILITY_INIT_PARAMS   pInitParams);

//!
//! \brief    Map the scalability pipe num to the LRCA count
//! \param    [in] pScalState
//!                pointer to Scalability decode state
//! \param    [in] LRCACount
//!                pointer to LRCA count
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeScalability_MapPipeNumToLRCACount(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalState,
    uint32_t                             *LRCACount);

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief    Debug override for scalability pipe num
//! \param    [in] pScalState
//!                pointer to Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeScalability_DebugOvrdDecidePipeNum(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState);
#endif

//!
//! \brief    Judge if Hevc buffer reallocate is needed
//! \param    [in] hwInterface
//!                HW interface for codechal
//! \param    [in] bufferType
//!                Type of the buffer
//! \param    [in] reallocParam
//!                Parameter of the reallocation
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS IsHevcBufferReallocNeeded(
    CodechalHwInterface                  *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam);

//!
//! \brief    Get Hevc buffer size
//! \param    [in] hwInterface
//!                HW interface for codechal
//! \param    [in] bufferType
//!                Type of the buffer
//! \param    [in] hcpBufSizeParam
//!                Parameter of the buffer size
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS GetHevcBufferSize(
    CodechalHwInterface                 *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam);

//!
//! \brief    Judge if Vp9 buffer reallocate is needed
//! \param    [in] hwInterface
//!                HW interface for codechal
//! \param    [in] bufferType
//!                Type of the buffer
//! \param    [in] reallocParam
//!                Parameter of the reallocation
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS IsVp9BufferReallocNeeded(
    CodechalHwInterface                  *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE   bufferType,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS reallocParam);

//!
//! \brief    Get Vp9 buffer size
//! \param    [in] hwInterface
//!                HW interface for codechal
//! \param    [in] bufferType
//!                Type of the buffer
//! \param    [in] hcpBufSizeParam
//!                Parameter of the buffer size
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS GetVp9BufferSize(
    CodechalHwInterface                 *hwInterface,
    MHW_VDBOX_HCP_INTERNAL_BUFFER_TYPE  bufferType,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS   hcpBufSizeParam);

//!
//! \brief    Allocate fixed size resources for scalability decode
//! \param    [in]  pScalabilityState
//!                pointer to Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_AllocateResources_FixedSizes(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState);

//!
//! \brief    Allocate variable size resources for scalability decode
//! \param    [in]  pScalabilityState
//!                pointer to Scalability decode state
//! \param    [in] pHcpBufSizeParam
//!                pointer to HCP Buf size parameter
//! \param    [in] pAllocParam
//!                pointer to Re-alloc parameter
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_AllocateResources_VariableSizes(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalabilityState,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    pHcpBufSizeParam,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS pAllocParam);

//!
//! \brief    Allocate CABAC streamout buffer for scalability decode
//! \param    [in]  pScalabilityState
//!                pointer to Scalability decode state
//! \param    [in] pHcpBufSizeParam
//!                pointer to HCP Buf size parameter
//! \param    [in] pAllocParam
//!                pointer to Re-alloc parameter
//! \param    [in] presCABACStreamOutBuffer
//!                pointer to cabac streamout buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS CodecHalDecodeScalability_AllocateCABACStreamOutBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE   pScalabilityState,
    PMHW_VDBOX_HCP_BUFFER_SIZE_PARAMS    pHcpBufSizeParam,
    PMHW_VDBOX_HCP_BUFFER_REALLOC_PARAMS pAllocParam,
    PMOS_RESOURCE                        presCABACStreamOutBuffer);

//!
//! \brief    Destroy resources for scalability decoder
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
void CodecHalDecodeScalability_Destroy (
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState);

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
MOS_STATUS CodecHalDecodeScalability_GetCmdBufferToUse(
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
MOS_STATUS CodecHalDecodeScalability_ReturnSdryCmdBuffer(
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
MOS_STATUS CodecHalDecodeScalability_DbgDumpCmdBuffer(
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalDebugInterface              *debugInterface,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf);
#endif

//!
//! \brief    Set FE CABAC Streamout buffer overflow status for Scalability Decode
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBufferInUse
//!                Address of command buffer in use
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalablity_SetFECabacStreamoutOverflowStatus(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

//!
//! \brief    Get FE Reported CABAC Streamout buffer size from register for Scalability Decode
//! \param    [in]  pScalabilityState
//!                 Scalability decode state
//! \param    [in]  pCmdBufferInUse
//!                 Address of command buffer in use
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalablity_GetFEReportedCabacStreamoutBufferSize(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

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
MOS_STATUS CodecHalDecodeScalability_DetermineDecodePhase(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    uint32_t                            *pHcpDecPhase);

//!
//! \brief    Determine if sending watch dog timer start cmd
//! \details  determine decode phase for decoder supporting scalability mode but not necessarily always running in scalable mode
//! \param    [in] pScalabilityState
//!                Scalability decode state
//! \param    [in] pbSend
//!                pointer to a flag indicating if to send
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_DetermineSendWatchdogTimerStart(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    bool                                *pbSend);

//!
//! \brief    switch gpu context in scalability decode mode
//! \param    [in] pScalabilityState
//!                Scalability decode state
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_SwitchGpuContext(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState);

//!
//! \brief    Initiliaze BEBE sync resources in scalability decode mode
//! \param    [in] pScalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_InitSemaMemResources(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBuffer);

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
MOS_STATUS CodecHalDecodeScalability_InitScalableParams(
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
MOS_STATUS CodecHalDecodeScalability_SetHintParams(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PCODECHAL_DECODE_SCALABILITY_SETHINT_PARMS pSetHintParms);

//!
//! \brief     Populate virtual engine hint parameters
//! \details  Populate virtual engine hint parameters. Support both scalable and single pipe decode mode.
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pPrimCmdBuf
//!                pointer to primary cmd buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_PopulateHintParams(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pPrimCmdBuf);

//!
//! \brief     Signal HW or SW semaphore for sync between FE and BE
//! \details  signal hw or sw semaphore at end of FE cmd buffer for sync between FE and BE
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_SignalFE2BESemaphore(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

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
MOS_STATUS CodecHalDecodeScalability_FEBESync(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

//!
//! \brief    To judge if command buffer should be submitted.
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \return   bool
//!           True means to submit command buffer, False means not to submit.
//!
bool CodecHalDecodeScalabilityIsToSubmitCmdBuffer(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState);

//!
//! \brief     Calculate parameters for adding HCP_TILE_CODING cmd
//! \details  Calculate parameters for adding HCP_TILE_CODING cmd in scalability decode
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pvStandardPicParams
//!                HEVC or VP9 picture parameters
//! \param    [in] pHcpTileCodingParam
//!                pointer of HCP_TILE_CODING parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
template <class TILE_CODING_PARAMS>
MOS_STATUS CodecHalDecodeScalability_CalculateHcpTileCodingParams(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    void                               *pvStandardPicParams,
    TILE_CODING_PARAMS                 *pHcpTileCodingParam)
{
    PCODEC_HEVC_PIC_PARAMS                          pHevcPicParams = nullptr;
    PCODEC_VP9_PIC_PARAMS                           pVp9PicParams = nullptr;
    uint32_t                                        i, uiMaxCbSize, uiMinCbSize, uiWidthInPixel, uiHeightInPixel;
    uint32_t                                        uiPicWidthInMinCb, uiPicHeightInMinCb, uiPicWidthInCtb;
    uint8_t                                         ucPipeIdx;
    uint16_t                                        usVTileColPos = 0;
    uint16_t                                        usVTileWidthInLCU[CODEC_VTILE_MAX_NUM];
    MOS_STATUS                                      eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_DECODE_FUNCTION_ENTER;

    CODECHAL_DECODE_CHK_NULL(pScalabilityState);
    CODECHAL_DECODE_CHK_NULL(pScalabilityState->pHwInterface);
    CODECHAL_DECODE_CHK_NULL(pvStandardPicParams);
    CODECHAL_DECODE_CHK_NULL(pHcpTileCodingParam);

    //BEs: HCP TILE CODING
    CODECHAL_DECODE_ASSERT(pScalabilityState->HcpDecPhase >= CODECHAL_HCP_DECODE_PHASE_BE0);
    ucPipeIdx = pScalabilityState->HcpDecPhase - CODECHAL_HCP_DECODE_PHASE_BE0;

    //calc virtual tile parameters
    if (pScalabilityState->Standard == CODECHAL_HEVC)
    {
        pHevcPicParams  = (PCODEC_HEVC_PIC_PARAMS)pvStandardPicParams;
        uiMinCbSize     = 1 << (pHevcPicParams->log2_min_luma_coding_block_size_minus3 + 3);
        uiMaxCbSize     = (1 << (pHevcPicParams->log2_diff_max_min_luma_coding_block_size)) * uiMinCbSize;
        uiWidthInPixel  = uiMinCbSize * (pHevcPicParams->PicWidthInMinCbsY);
        uiHeightInPixel = uiMinCbSize * (pHevcPicParams->PicHeightInMinCbsY);
        uiPicWidthInCtb = MOS_ROUNDUP_DIVIDE(uiWidthInPixel, uiMaxCbSize);
        uiPicWidthInMinCb  = pHevcPicParams->PicWidthInMinCbsY;
        uiPicHeightInMinCb = pHevcPicParams->PicHeightInMinCbsY;
    }
    else if (pScalabilityState->Standard == CODECHAL_VP9)
    {
        pVp9PicParams   = (PCODEC_VP9_PIC_PARAMS)pvStandardPicParams;
        uiMinCbSize     = CODEC_VP9_MIN_BLOCK_WIDTH;
        uiMaxCbSize     = CODEC_VP9_SUPER_BLOCK_WIDTH;
        uiWidthInPixel  = pVp9PicParams->FrameWidthMinus1 + 1;
        uiHeightInPixel = pVp9PicParams->FrameHeightMinus1 + 1;
        uiPicWidthInCtb = MOS_ROUNDUP_DIVIDE(pVp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_SUPER_BLOCK_WIDTH);
        uiPicWidthInMinCb  = MOS_ROUNDUP_DIVIDE(pVp9PicParams->FrameWidthMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
        uiPicHeightInMinCb = MOS_ROUNDUP_DIVIDE(pVp9PicParams->FrameHeightMinus1 + 1, CODEC_VP9_MIN_BLOCK_WIDTH);
    }
    else
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        CODECHAL_DECODE_ASSERTMESSAGE("invalid codec type, only HEVC/VP9 are supported in scalability mode.");
        goto finish;
    }

    //calc virtual tile width and position
    for (i = 0; i <= ucPipeIdx; i++)
    {
        usVTileWidthInLCU[i] = ((i + 1) * uiPicWidthInCtb / pScalabilityState->ucScalablePipeNum) -
            (i * uiPicWidthInCtb / pScalabilityState->ucScalablePipeNum);

        usVTileColPos += (i == 0 ? 0 : usVTileWidthInLCU[i - 1]);
    }

    if (usVTileWidthInLCU[ucPipeIdx] < 2)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("HW limitation: Virtual tile for decode should be at least 2 LCU.");
        goto finish;
    }

    MOS_ZeroMemory(pHcpTileCodingParam, sizeof(TILE_CODING_PARAMS));

    if (pScalabilityState->bIsEvenSplit)
    {
        //if the last tile, the tilewidth need to align to min CU not LCU.
        if ( ucPipeIdx == (pScalabilityState->ucScalablePipeNum - 1))
        {
            uint16_t usVTileColPosInMinCb = usVTileColPos * uiMaxCbSize / uiMinCbSize;
            pHcpTileCodingParam->TileWidthInMinCbMinus1  = uiPicWidthInMinCb - usVTileColPosInMinCb - 1;
        }
        else
        {
            pHcpTileCodingParam->TileWidthInMinCbMinus1  = usVTileWidthInLCU[ucPipeIdx] * uiMaxCbSize / uiMinCbSize - 1;
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        if (pScalabilityState->dbgOvrdWidthInMinCb && pScalabilityState->ucScalablePipeNum == 2)
        {
            if (ucPipeIdx == 1)
            {
                pHcpTileCodingParam->TileWidthInMinCbMinus1 = uiPicWidthInMinCb - pScalabilityState->dbgOvrdWidthInMinCb - 1;
                usVTileColPos = pScalabilityState->dbgOvrdWidthInMinCb * uiMinCbSize / uiMaxCbSize;
            }
            else
            {
                pHcpTileCodingParam->TileWidthInMinCbMinus1 = pScalabilityState->dbgOvrdWidthInMinCb - 1;
                usVTileColPos = 0;
            }
        }
#endif
    }
    else
    {
        if (ucPipeIdx == 0)
        {
            usVTileColPos = 0;
            if ((uiWidthInPixel * uiHeightInPixel) >= (CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_WIDTH * CODECHAL_HCP_DECODE_SCALABLE_THRESHOLD4_HEIGHT))
            {
                // need to align first Tile Column to 4096
                pScalabilityState->uiFirstTileColWidth = CODEC_SCALABILITY_FIRST_TILE_WIDTH_8K;
            }
            else
            {
                // need to align first Tile Column to 2048
                pScalabilityState->uiFirstTileColWidth = CODEC_SCALABILITY_FIRST_TILE_WIDTH_4K;
            }

            if (uiWidthInPixel <= pScalabilityState->uiFirstTileColWidth)
            {
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                MHW_ASSERTMESSAGE("HW limitation: pic width %d should be greater than min tile width %d.", uiWidthInPixel, pScalabilityState->uiFirstTileColWidth);
                goto finish;
            }

            pHcpTileCodingParam->TileWidthInMinCbMinus1  = MOS_ROUNDUP_DIVIDE(pScalabilityState->uiFirstTileColWidth, uiMinCbSize) - 1;
        }
        else if (ucPipeIdx == 1)
        {
            usVTileColPos = GFX_CEIL_DIV(pScalabilityState->uiFirstTileColWidth, uiMaxCbSize);
            pHcpTileCodingParam->TileWidthInMinCbMinus1  = uiPicWidthInMinCb - MOS_ROUNDUP_DIVIDE(pScalabilityState->uiFirstTileColWidth, uiMinCbSize) - 1;
        }
        else
        {
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            CODECHAL_DECODE_ASSERTMESSAGE("Invalid pipe number %d for scalability + MMC.", ucPipeIdx);
            goto finish;
        }
    }

    pHcpTileCodingParam->TileHeightInMinCbMinus1        = uiPicHeightInMinCb - 1;
    pHcpTileCodingParam->TileStartLCUX                  = usVTileColPos;
    pHcpTileCodingParam->ucNumDecodePipes               = pScalabilityState->ucScalablePipeNum;
    pHcpTileCodingParam->ucPipeIdx                      = ucPipeIdx;

finish:
    return eStatus;
}

//!
//! \brief    Sync for all BEs completion
//! \details  Add hw semaphore and/or MI ATOMIC cmd for all other BEs execution completion
//! \param    [in]  pScalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_BEsCompletionSync(
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    PMOS_COMMAND_BUFFER                 pCmdBufferInUse);

//!
//! \brief    Read CS ENGINEID register to know which engine is in use for current workload
//! \param    [in]  pDecodeStatusBuf
//!                Decode status buffer
//! \param    [in] pCmdBufferInUse
//!                address of command buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodecHalDecodeScalability_ReadCSEngineIDReg(
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
MOS_STATUS CodecHalDecodeScalability_InitializeState (
    CodechalDecode                      *pDecoder,
    PCODECHAL_DECODE_SCALABILITY_STATE  pScalabilityState,
    CodechalHwInterface                 *hwInterface,
    bool                                bShortFormat);

//! \brief    construct gpu context creation options when scalability supported
//! \param    [in]  scalabilityState
//!                Scalability decode state
//! \param    [in]  gpuCtxCreatOptions
//!                pointer to gpu context creation options
//! \param    [in]  codechalSetting
//!                Pointer to codechal setting
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeScalability_ConstructParmsForGpuCtxCreation(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          gpuCtxCreatOpts,
    CodechalSetting *                          codecHalSetting);

//! \brief    Check if need to recreate gpu context and if yes, do it.
//! \param    [in]  scalabilityState
//!                Scalability decode state
//! \param    [in]  gpuCtxCreatOptions
//!                pointer to gpu context creation options
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS CodechalDecodeScalability_ChkGpuCtxReCreation(
    PCODECHAL_DECODE_SCALABILITY_STATE         pScalabilityState,
    PMOS_GPUCTX_CREATOPTIONS_ENHANCED          CurgpuCtxCreatOpts);

//!
//! \brief    Convert Decode Phase to Batch Buffer Submission Type
//! \param    [in]  scalabilityState
//!                Scalability decode state
//! \param    [in] pCmdBuffer
//!                Pointer to command buffer
//! \return   void
//!           void
//!
void CodecHalDecodeScalability_DecPhaseToSubmissionType(
    PCODECHAL_DECODE_SCALABILITY_STATE pScalabilityState,
    PMOS_COMMAND_BUFFER pCmdBuffer);

#endif //__CODECHAL_DECODER_SCALABILITY_H__
