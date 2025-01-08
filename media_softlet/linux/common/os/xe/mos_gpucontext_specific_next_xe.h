/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     mos_gpucontext_specific_next_xe.h
//! \brief    Container class for the linux/Android specfic gpu context
//!

#ifndef __GPU_CONTEXT_SPECIFIC_NEXT_XE_H__
#define __GPU_CONTEXT_SPECIFIC_NEXT_XE_H__

#include "mos_bufmgr_xe.h"
#include "mos_gpucontext_specific_next.h"

//!
//! \class  GpuContextSpecific
//! \brief  Linux/Android specific gpu context 
//!
class GpuContextSpecificNextXe : public GpuContextSpecificNext
{
public:
    //!
    //! \brief  Constructor
    //!
    GpuContextSpecificNextXe(
        const MOS_GPU_NODE gpuNode,
        CmdBufMgrNext      *cmdBufMgr,
        GpuContextNext    *reusedContext) : GpuContextSpecificNext(gpuNode, cmdBufMgr, reusedContext){}

    //!
    //! \brief  Destructor
    //!
    ~GpuContextSpecificNextXe();

    virtual MOS_STATUS SubmitCommandBuffer(
        MOS_STREAM_HANDLE   streamState,
        PMOS_COMMAND_BUFFER cmdBuffer,
        bool                nullRendering) override;

    //!
    //! \brief  Set the Gpu priority for workload scheduling.
    //! \param [in] priority
    //!             priority to set for current workload.
    //! \return void
    //!
    virtual void UpdatePriority(int32_t priority) override;

    //!
    //! \brief    Allocate gpu status buffer for gpu sync
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    //MOS_STATUS AllocateGPUStatusBuf();

protected:
    virtual MOS_STATUS ReportEngineInfo(
        void *engine_map,
        int engineNum, bool engineSelectEnable = false) override;

    virtual MOS_STATUS Init3DCtx(PMOS_CONTEXT osParameters,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map) override;

    virtual MOS_STATUS InitComputeCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable) override;

    virtual MOS_STATUS InitVdVeCtx(PMOS_CONTEXT osParameters,
                MOS_STREAM_HANDLE streamState,
                PMOS_GPUCTX_CREATOPTIONS createOption,
                unsigned int *nengine,
                void *engine_map,
                MOS_GPU_NODE gpuNode,
                bool *isEngineSelectEnable) override;

    virtual MOS_STATUS InitBltCtx(PMOS_CONTEXT osParameters,
                unsigned int *nengine,
                void *engine_map) override;

    virtual int32_t ParallelSubmitCommands(std::map<uint32_t, PMOS_COMMAND_BUFFER> secondaryCmdBufs,
                                   PMOS_CONTEXT osContext,
                                   uint32_t execFlag,
                                   int32_t dr4) override;

    virtual MOS_STATUS PatchCommandBuffer(MOS_STREAM_HANDLE   streamState,
                PMOS_COMMAND_BUFFER cmdBuffer);

    virtual MOS_STATUS EndSubmitCommandBuffer(MOS_STREAM_HANDLE   streamState,
                PMOS_COMMAND_BUFFER cmdBuffer,
                bool                cmdBufMapIsReused);

private:
    void ClearSecondaryCmdBuffer(bool cmdBufMapIsReused);

MEDIA_CLASS_DEFINE_END(GpuContextSpecificNextXe)
};
#endif  // __GPU_CONTEXT_SPECIFIC_NEXT_XE_H__
