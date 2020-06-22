/*
* Copyright (c) 2016-2018, Intel Corporation
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
//! \file     mos_os_virtualengine.h
//! \brief    Common interface and structure used in MOS OS VirtualEngine
//! \details  Common interface and structure used in MOS OS VirtualEngine, and only necessary when KMD supports virtual engine
//!
#ifndef __MOS_OS_VIRTUALENGINE_H__
#define __MOS_OS_VIRTUALENGINE_H__

#include "mos_os.h"
#include "mos_os_virtualengine_specific.h"

#define MOS_VE_HAVE_SECONDARY_CMDBUFFER    0x2

typedef struct _MOS_VIRTUALENGINE_INIT_PARAMS
{
    bool       bScalabilitySupported;

    //below only valid when scalability is supported
    bool       bFESeparateSubmit;               //!< for decode only
    uint8_t    ucMaxNumOfSdryCmdBufInOneFrame;
    uint8_t    ucMaxNumPipesInUse;
    uint8_t    ucNumOfSdryCmdBufSets;
}MOS_VIRTUALENGINE_INIT_PARAMS, *PMOS_VIRTUALENGINE_INIT_PARAMS;

typedef struct _MOS_VIRTUALENGINE_SET_PARAMS
{
    bool       bSameEngineAsLastSubmission;
    bool       bNeedSyncWithPrevious;
    bool       bSFCInUse; 

    //below only valid when scalability is supported
    bool       bScalableMode;
    bool       bHaveFrontEndCmds;
    uint8_t    ucScalablePipeNum;
    MOS_RESOURCE       veBatchBuffer[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
}MOS_VIRTUALENGINE_SET_PARAMS, *PMOS_VIRTUALENGINE_SET_PARAMS;

class MosVeInterface;
typedef struct _MOS_VIRTUALENGINE_INTERFACE
{
    MosVeInterface                 *veInterface;

    bool                            bScalabilitySupported;
    PMOS_INTERFACE                  pOsInterface;
#if (_DEBUG || _RELEASE_INTERNAL)
    uint8_t                         EngineLogicId[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
    uint8_t                         ucEngineCount;
#endif
    uint8_t                         ucMaxNumPipesInUse;
    uint8_t                         ucNumOfSdryCmdBufInCurFrm;  //!< number of secondary cmd buffers for current frame
    uint32_t                        dwSdryCmdBufSize;           //!< secondary cmd buffer size for current frame
    uint8_t                         ucMaxNumOfSdryCmdBufInOneFrame;
    uint32_t                        dwTotalNumOfSdryCmdBufs;
    uint8_t                         ucNumOfSdryCmdBufSets;
    uint8_t                         ucSdryCmdbufPoolIndx;
    bool                           *pbSdryCmdBufPrepared;
    PMOS_COMMAND_BUFFER             pSecondryCmdBufPool;
    MOS_VIRTUALENGINE_HINT_PARAMS   ScalabilityHintParams;
    MOS_VIRTUALENGINE_HINT_PARAMS   SinglePipeHintParams;

    //!
    //! \brief     check if scalability is supported
    //! \param    [in]   pVEInterface
    //!                virtual engine interface
    //! \param    [out] pbScalabilitySupported
    //!                pointer to a bool value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEIsScalabilitySupported)(
        PMOS_VIRTUALENGINE_INTERFACE     pVEInterface,
        bool                            *pbScalabilitySupported);

    //!
    //! \brief     Reset secondary cmd buffer state
    //! \details  Reset secondary cmd buffer state in scalability virtual engine interface
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \return   void
    //!
    void (*pfnVEResetSecdryCmdBufStates)(
        PMOS_VIRTUALENGINE_INTERFACE     pVEInterface);

    //!
    //! \brief    Verify secondary cmd buffer size
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in]  dwNewRequestSize
    //!                new request cmd buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEVerifySecdryCmdBufSize)(
        PMOS_VIRTUALENGINE_INTERFACE     pVEInterface,
        uint32_t                         dwNewRequestSize);

    //!
    //! \brief    resize secondary cmd buffer
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in]  dwNewRequestSize
    //!                new request cmd buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEResizeSecdryCmdBuf)(
        PMOS_VIRTUALENGINE_INTERFACE     pVEInterface,
        uint32_t                         dwNewRequestSize);

    //!
    //! \brief    get secondary cmd buffer 
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in]  pScdryCmdBuf
    //!                pointer to secondry cmd buffer
    //! \param    [in]  dwBufIdxPlus1
    //!                secondary cmd buffer index plus 1
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEGetSecdryCmdBuf)(
        PMOS_VIRTUALENGINE_INTERFACE    pVEInterface,
        PMOS_COMMAND_BUFFER             pScdryCmdBuf,
        uint32_t                        dwBufIdxPlus1);

    //!
    //! \brief    return secondary cmd buffer 
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in] pScdryCmdBuf
    //!                pointer to secondry cmd buffer address
    //! \param    [in]  dwBufIdxPlus1
    //!                secondary cmd buffer index plus 1
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEReturnSecdryCmdBuf)(
        PMOS_VIRTUALENGINE_INTERFACE    pVEInterface,
        PMOS_COMMAND_BUFFER             pScdryCmdBuf,
        uint32_t                        dwBufIdxPlus1);

    //!
    //! \brief    done virtual engine secondary command buffers
    //! \details  UnLock virtual engine secondary command buffers
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEDoneSecdryCmdBuf)(
        PMOS_VIRTUALENGINE_INTERFACE    pVEInterface);

    //!
    //! \brief    set hint parameters 
    //! \details  set hint parameters for virtual engine scalability or single pipe mode 
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in] pVEParams
    //!                pointer to VE parameter data structure
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVESetHintParams) (
        PMOS_VIRTUALENGINE_INTERFACE   pVEInterface,
        PMOS_VIRTUALENGINE_SET_PARAMS  pVEParams);

    //!
    //! \brief    get hint parameters 
    //! \details  get hint parameters for virtual engine scalability or single pipe mode 
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in] bScalableMode
    //!                flag to indicate if scalability mode
    //! \param    [in] ppHintParams
    //!                pointer to VE hint parameter address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVEGetHintParams)(
        PMOS_VIRTUALENGINE_INTERFACE   pVEInterface,
        bool                           bScalableMode,
        PMOS_VIRTUALENGINE_HINT_PARAMS *ppHintParams);

    //!
    //! \brief    check hint parameters 
    //! \details  check hint parameters for virtual engine scalability or single pipe mode 
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \param    [in] ppHintParams
    //!                pointer to VE hint parameter address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVECheckHintParamsValidity) (
        PMOS_VIRTUALENGINE_INTERFACE   pVEInterface);

    //!
    //! \brief    Destroy resources for virtual engine scalability
    //! \param    [in]  pVEInterface
    //!                virtual engine interface
    //! \return   void
    //!
    void (* pfnVEDestroy)(
        PMOS_VIRTUALENGINE_INTERFACE    pVEInterface);
}MOS_VIRTUALENGINE_INTERFACE, *PMOS_VIRTUALENGINE_INTERFACE;

//!
//! \brief     initialize virtual engine interface
//! \details  initialize virtual engine interface
//! \param    [in]  PMOS_INTERFACE
//!                pointer to mos interface
//! \param    [in]  PMOS_VIRTUALENGINE_INIT_PARAMS pVEInitParms
//!                pointer to VE init parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS Mos_VirtualEngineInterface_Initialize(
    PMOS_INTERFACE                    pOsInterface,
    PMOS_VIRTUALENGINE_INIT_PARAMS    pVEInitParms);

#endif //__MOS_OS_VIRTUALENGINE_SCALABILITY_H__

