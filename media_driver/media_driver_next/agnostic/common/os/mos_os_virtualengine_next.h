/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     mos_os_virtualengine_next.h
//! \brief    Common interface and structure used in MOS OS VirtualEngine
//! \details  Common interface and structure used in MOS OS VirtualEngine, and only necessary when KMD supports virtual engine
//!
#ifndef __MOS_OS_VIRTUALENGINE_NEXT_H__
#define __MOS_OS_VIRTUALENGINE_NEXT_H__
#include "mos_os_virtualengine.h"
#include "mos_os_next.h"
#include "mos_os_virtualengine_specific.h"

class MosVeInterface
{
public:
    //!
    //! \brief    Construct
    //!
    MosVeInterface() {}

    //!
    //! \brief    Deconstruct
    //!
    virtual ~MosVeInterface() {}

    //!
    //! \brief    initialize virtual engine interface
    //! \details  initialize virtual engine interface
    //! \param    [in]  stream
    //!           MOS stream state
    //! \param    [in]  veInitParms
    //!           pointer to VE init parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(
        MOS_STREAM_HANDLE stream,
        PMOS_VIRTUALENGINE_INIT_PARAMS veInitParms);

    //!
    //! \brief     check if scalability is supported
    //! \param    [out] pbScalabilitySupported
    //!                pointer to a bool value
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS IsScalabilitySupported(
        bool *pbScalabilitySupported);

    //!
    //! \brief     Reset secondary cmd buffer state
    //! \details  Reset secondary cmd buffer state in scalability virtual engine interface
    //! \return   void
    //!
    virtual void ResetSecdryCmdBufStates() = 0;

    //!
    //! \brief    Verify secondary cmd buffer size
    //! \param    [in]  dwNewRequestSize
    //!                new request cmd buffer size
    //! \return   bool
    //!           true if size is enough, else the size < new requested size
    //!
    virtual bool VerifySecdryCmdBufSize(
        uint32_t dwNewRequestSize) = 0;

    //!
    //! \brief    resize secondary cmd buffer
    //! \param    [in]  dwNewRequestSize
    //!                new request cmd buffer size
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ResizeSecdryCmdBuf(
        uint32_t dwNewRequestSize) = 0;

    //!
    //! \brief    get secondary cmd buffer 
    //! \param    [in]  pScdryCmdBuf
    //!                pointer to secondry cmd buffer
    //! \param    [in]  dwBufIdxPlus1
    //!                secondary cmd buffer index plus 1
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetSecdryCmdBuf(
        PMOS_COMMAND_BUFFER pScdryCmdBuf,
        uint32_t dwBufIdxPlus1) = 0;

    //!
    //! \brief    return secondary cmd buffer
    //! \param    [in] pScdryCmdBuf
    //!                pointer to secondry cmd buffer address
    //! \param    [in]  dwBufIdxPlus1
    //!                secondary cmd buffer index plus 1
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS ReturnSecdryCmdBuf(
        PMOS_COMMAND_BUFFER pScdryCmdBuf,
        uint32_t dwBufIdxPlus1) = 0;

    //!
    //! \brief    done virtual engine secondary command buffers
    //! \details  UnLock virtual engine secondary command buffers
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS DoneSecdryCmdBuf() = 0;

    //!
    //! \brief    set hint parameters 
    //! \details  set hint parameters for virtual engine scalability or single pipe mode 
    //! \param    [in] pVEParams
    //!                pointer to VE parameter data structure
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHintParams(
        PMOS_VIRTUALENGINE_SET_PARAMS pVEParams) = 0;

    //!
    //! \brief    get hint parameters 
    //! \details  get hint parameters for virtual engine scalability or single pipe mode 
    //! \param    [in] bScalableMode
    //!                flag to indicate if scalability mode
    //! \param    [in] ppHintParams
    //!                pointer to VE hint parameter address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS GetHintParams(
        bool                           bScalableMode,
        PMOS_VIRTUALENGINE_HINT_PARAMS *ppHintParams) = 0;

    //!
    //! \brief    check hint parameters 
    //! \details  check hint parameters for virtual engine scalability or single pipe mode 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS CheckHintParamsValidity() = 0;

    //!
    //! \brief    Destroy resources for virtual engine scalability
    //!
    //! \return   void
    //!
    virtual void Destroy() = 0;

    //!
    //! \brief    Set Submission Type for cmd buffer
    //! \param    [out] cmdBuf
    //!           Handle of cmd buffer to set submission type
    //! \param    [in] type
    //!           Submission type to set
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetSubmissionType(
        COMMAND_BUFFER_HANDLE cmdBuf,
        MOS_SUBMISSION_TYPE   type)
    {
        MOS_OS_CHK_NULL_RETURN(cmdBuf);
        cmdBuf->iSubmissionType = type;
        return MOS_STATUS_SUCCESS;
    }

#if _DEBUG || _RELEASE_INTERNAL
    //!
    //! \brief    Get Engine Count
    //! \return   uint8_t
    //!
    uint8_t GetEngineCount()
    {
        return ucEngineCount;
    }

    //!
    //! \brief    Get Engine Logic Id
    //! \param    [in] instanceIdx
    //!                Engine instance index
    //! \return   uint8_t
    //!
    uint8_t GetEngineLogicId(uint32_t instanceIdx)
    {
        return EngineLogicId[instanceIdx];
    }
#endif // _DEBUG || _RELEASE_INTERNAL

protected:
    MOS_STREAM_HANDLE m_stream = MOS_INVALID_HANDLE;
    bool bScalabilitySupported = false;

#if _DEBUG || _RELEASE_INTERNAL
    uint8_t EngineLogicId[MOS_MAX_ENGINE_INSTANCE_PER_CLASS] = {};
    uint8_t ucEngineCount = 1;
    bool    m_enableDbgOvrdInVirtualEngine = false;
#endif // _DEBUG || _RELEASE_INTERNAL

    uint8_t ucMaxNumPipesInUse       = 1;
    bool    m_contextBasedScheduling = false;

    MOS_VIRTUALENGINE_HINT_PARAMS ScalabilityHintParams = {};
    MOS_VIRTUALENGINE_HINT_PARAMS SinglePipeHintParams = {};
};

#endif //__MOS_OS_VIRTUALENGINE_SCALABILITY_NEXT_H__

