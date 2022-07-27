/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     hal_oca_interface_next.cpp
//! \brief    Implementation of functions for Hal OCA Interface
//! \details  Implementation of functions for Hal OCA Interface
//!

#include <stdint.h>
#include <map>
#include <memory>
#include <utility>
#include "codec_def_common.h"
#include "mhw_mi.h"
#include "mos_defs.h"
#include "mos_defs_specific.h"
#include "mos_oca_interface.h"
#include "mos_os_hw.h"
#include "mos_os_specific.h"
#include "mos_os.h"
#include "hal_oca_interface_next.h"
#include "mhw_mmio.h"
#include "mos_interface.h"
#include "mos_oca_interface_specific.h"
#include "mos_oca_rtlog_mgr.h"
#include "vphal.h"
#include "vphal_debug.h"
#include "codechal_oca_debug.h"
#include "mhw_utilities_next.h"

namespace mhw { namespace mi { class Itf; } }

std::map<uint32_t*, MOS_OCA_BUFFER_HANDLE> HalOcaInterfaceNext::s_hOcaMap;

/****************************************************************************************************/
/*                                      HalOcaInterfaceNextNext                                             */
/****************************************************************************************************/
MOS_STATUS HalOcaInterfaceNext::MhwMiLoadRegisterImmCmd(
    std::shared_ptr<mhw::mi::Itf>    miItf,
    PMOS_COMMAND_BUFFER              pCmdBuffer,
    MHW_MI_LOAD_REGISTER_IMM_PARAMS *params)
{

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
//! \param  [in/out] cmdBuffer
//!         Command buffer for current BB. ocaBufHandle in cmdBuffer will be updated.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] gpuContextHandle
//!         Gpu context handle 
//! \param  [in] miItf
//!         Reference to Mhw MiItf.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \param  [in] offsetOf1stLevelBB
//!         Offset for current BB in cmdBuffer.
//! \param  [in] bUseSizeOfCmdBuf
//!         If true, use size of cmdBuffer for batch buffer, else use sizeOf1stLevelBB.
//! \param  [in] sizeOf1stLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::On1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext,
        uint32_t gpuContextHandle, std::shared_ptr<mhw::mi::Itf> miItf, MHW_MI_MMIOREGISTERS &mmioRegisters,
        uint32_t offsetOf1stLevelBB, bool bUseSizeOfCmdBuf, uint32_t sizeOf1stLevelBB)
{
}

//!
//! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
//! \param  [in/out] cmdBuffer
//!         Command buffer for current BB. ocaBufHandle in cmdBuffer will be updated.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] gpuContextHandle
//!         Gpu context handle
//! \param  [in] miItf
//!         Reference to Mhw MiItf.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \param  [in] offsetOf1stLevelBB
//!         Offset for current BB in cmdBuffer.
//! \param  [in] bUseSizeOfCmdBuf
//!         If true, use size of cmdBuffer for batch buffer, else use sizeOf1stLevelBB.
//! \param  [in] sizeOf1stLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::On1stLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext,
        uint32_t gpuContextHandle, std::shared_ptr<mhw::mi::Itf> miItf, MmioRegistersMfx &mmioRegisters,
        uint32_t offsetOf1stLevelBB, bool bUseSizeOfCmdBuf, uint32_t sizeOf1stLevelBB)
{
}

//!
//! \brief  Oca operation which should be called before adding batch buffer end command for 1st
//!         level batch buffer.
//! \param  [in/out] cmdBuffer
//!         Command buffer for current BB. ocaBufHandle in cmdBuffer will be updated.
//! \param  [in] osInterface
//!         Reference to MOS_INTERFACE.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::On1stLevelBBEnd(MOS_COMMAND_BUFFER &cmdBuffer, MOS_INTERFACE &osInterface)
{
    if (nullptr == osInterface.pOsContext)
    {
        OnOcaError(nullptr, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }
    MOS_CONTEXT &           mosContext     = *osInterface.pOsContext;
    MosOcaInterface *pOcaInterface         = &MosOcaInterfaceSpecific::GetInstance();
    MOS_OCA_BUFFER_HANDLE   ocaBufHandle   = 0;
    MOS_STATUS              status         = MOS_STATUS_SUCCESS;

    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        // Will come here for UMD_OCA not being enabled case.
        return;
    }
    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    status = pOcaInterface->On1stLevelBBEnd(ocaBufHandle, osInterface.pOsContext);
    if (MOS_FAILED(status))
    {
        // UnlockOcaBuf and RemoveOcaBufferHandle should also be called
        // for failure case to avoid oca buffer leak.
        OnOcaError(osInterface.pOsContext, status, __FUNCTION__, __LINE__);
    }
    status = pOcaInterface->UnlockOcaBuf(ocaBufHandle);
    if (MOS_FAILED(status))
    {
        OnOcaError(osInterface.pOsContext, status, __FUNCTION__, __LINE__);
    }

    // remove the oca buffer handle
    RemoveOcaBufferHandle(cmdBuffer, mosContext);
}

//!
//! \brief  Oca operation which should be called before sending start sub level batch buffer command.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] pMosResource
//!         Pointer to the MOS_RESOURCE.
//! \param  [in] offsetOfSubLevelBB
//!         Offset for current BB in pMosResource.
//! \param  [in] bUseSizeOfResource
//!         If true, use size of pMosResource for batch buffer, else use sizeOfIndirectState.
//! \param  [in] sizeOfSubLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::OnSubLevelBBStart(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pMosResource, uint32_t offsetOfSubLevelBB, bool bUseSizeOfResource, uint32_t sizeOfSubLevelBB)
{
    MosInterface::SetObjectCapture((PMOS_RESOURCE)pMosResource);
}

//!
//! \brief  Oca operation which should be called when indirect states being added.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] pMosResource
//!         Pointer to the MOS_RESOURCE.
//! \param  [in] offsetOfIndirectState
//!         Offset for current state in pMosResource.
//! \param  [in] bUseSizeOfResource
//!         If true, use size of pMosResource for indirect state, else use sizeOfIndirectState.
//! \param  [in] sizeOfIndirectState
//!         Size of indirect state. Ignore if bUseSizeOfResource == true.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::OnIndirectState(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pMosResource, uint32_t offsetOfIndirectState, bool bUseSizeOfResource, uint32_t sizeOfIndirectState)
{
    MosInterface::SetObjectCapture((PMOS_RESOURCE)pMosResource);
}

//!
//! \brief  Oca operation which should be called before adding dispatch states,
//!         e.g. VEB_DI_IECP_STATE and MEDIA_OBJECT_WALKER.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] miItf
//!         Reference to Mhw MiItf.
//! \param  [in] mmioRegisters
//!         mmio registers for current engine.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::OnDispatch(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, std::shared_ptr<mhw::mi::Itf> miItf, MHW_MI_MMIOREGISTERS &mmioRegisters)
{
}

//!
//! \brief  Add string to oca log section
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] str
//!         string to be added.
//! \param  [in] maxCount
//!         size of the buffer pointed by str.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::TraceMessage(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, const char *str, uint32_t maxCount)
{
    MosOcaInterface *pOcaInterface          = &MosOcaInterfaceSpecific::GetInstance();
    MOS_OCA_BUFFER_HANDLE   ocaBufHandle    = 0;
    MOS_STATUS              status          = MOS_STATUS_SUCCESS;

    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        return;
    }
    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    status = pOcaInterface->TraceMessage(ocaBufHandle, &mosContext, str, maxCount);
    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

//!
//! \brief  Add vp kernel info to oca log section.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] osInterface
//!         Reference to MOS_INTERFACE.
//! \param  [in] res
//!         Reference to MOS_RESOURCE.
//! \param  [in] hwCmdType
//!         Hw command type.
//! \param  [in] locationInCmd
//!         Location in command.
//! \param  [in] offsetInRes
//!         Offset in resource.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::DumpResourceInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_INTERFACE &osInterface, MOS_RESOURCE &res, MOS_HW_COMMAND hwCmdType, uint32_t locationInCmd, uint32_t offsetInRes)
{
    if (nullptr == osInterface.pOsContext)
    {
        OnOcaError(nullptr, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }

    MOS_CONTEXT &           mosContext      = *osInterface.pOsContext;
    MosOcaInterface         *pOcaInterface  = &MosOcaInterfaceSpecific::GetInstance();
    MOS_STATUS              status          = MOS_STATUS_SUCCESS;
    MOS_OCA_BUFFER_HANDLE   ocaBufHandle    = 0;

    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        return;
    }

    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    if (Mos_ResourceIsNull(&res))
    {
        OnOcaError(&mosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }

    status = pOcaInterface->AddResourceToDumpList(ocaBufHandle, &mosContext, res, hwCmdType, locationInCmd, offsetInRes);
    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

    //!
//! \brief  Trace OCA Sku Value.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] osInterface
//!         Reference to MOS_INTERFACE.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::TraceOcaSkuValue(MOS_COMMAND_BUFFER &cmdBuffer, MOS_INTERFACE &osInterface)
{
}

//!
//! \brief  Add vp kernel info to oca log section.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] vpKernelID
//!         Value of enum VpKernelID.
//! \param  [in] fcKernelCount
//!         If vpKernelID == kernelCombinedFc, fcKernelCount is the kernel count for fc, otherwise, it's not used.
//! \param  [in] fcKernelList
//!         If vpKernelID == kernelCombinedFc, fcKernelList is the kernel list for fc, otherwise, it's not used.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::DumpVpKernelInfo(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, int vpKernelID, int fcKernelCount, int *fcKernelList)
{
    MosOcaInterface *pOcaInterface          = &MosOcaInterfaceSpecific::GetInstance();
    MOS_STATUS              status          = MOS_STATUS_SUCCESS;
    MOS_OCA_BUFFER_HANDLE   ocaBufHandle    = 0;
    uint32_t                updateSize      = 0;

    if (nullptr == pOcaInterface || !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled())
    {
        return;
    }

    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    if (kernelCombinedFc == vpKernelID && (fcKernelCount <= 0 || nullptr == fcKernelList))
    {
        OnOcaError(&mosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }

    if (kernelCombinedFc != vpKernelID)
    {
        fcKernelCount = 0;
        fcKernelList = nullptr;
    }

    MOS_OCA_LOG_HEADER_VP_KERNEL_INFO header = {};
    header.header.type                      = MOS_OCA_LOG_TYPE_VP_KERNEL_INFO;
    header.header.headerSize                = sizeof(MOS_OCA_LOG_HEADER_VP_KERNEL_INFO);
    header.header.dataSize                  = fcKernelCount * sizeof(int);
    header.vpKernelID                       = vpKernelID;
    header.fcKernelCount                    = fcKernelCount;
    status = pOcaInterface->DumpDataBlock(ocaBufHandle, &mosContext, (PMOS_OCA_LOG_HEADER)&header, fcKernelList);
    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

//!
//! \brief  Add vphal parameters to oca log section.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] pVphalDumper
//!         Pointer to vphal dumper object.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::DumpVphalParam(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pVphalDumper)
{
    MosOcaInterface *pOcaInterface       = &MosOcaInterfaceSpecific::GetInstance();
    MOS_STATUS status                    = MOS_STATUS_SUCCESS;
    MOS_OCA_BUFFER_HANDLE ocaBufHandle   = 0;
    uint32_t updateSize                  = 0;

    if (nullptr == pOcaInterface                                   ||
        !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled() ||
        nullptr == pVphalDumper)
    {
        return;
    }
    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    VPHAL_OCA_RENDER_PARAM *pVphalParam = ((VphalOcaDumper *)pVphalDumper)->GetRenderParam();
    if (nullptr == pVphalParam)
    {
        return;
    }

    MOS_OCA_LOG_HEADER_VPHAL_PARAM header = {};
    header.header.type                = MOS_OCA_LOG_TYPE_VPHAL_PARAM;  // 00000003
    header.header.headerSize          = sizeof(MOS_OCA_LOG_HEADER_VPHAL_PARAM);
    header.header.dataSize            = pVphalParam->Header.size;
    status = pOcaInterface->DumpDataBlock(ocaBufHandle, &mosContext, (PMOS_OCA_LOG_HEADER)&header, pVphalParam);
    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

//!
//! \brief  Add codechal parameters to oca log section.
//! \param  [in] cmdBuffer
//!         Command buffer for current BB.
//! \param  [in] mosContext
//!         Reference to MOS_CONTEXT.
//! \param  [in] pCodechalDumper
//!         Pointer to codechal dumper object.
//! \return void
//!         No return value. Handle all exception inside the function.
//!
void HalOcaInterfaceNext::DumpCodechalParam(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext, void *pCodechalDumper, CODECHAL_STANDARD codec)
{
    MosOcaInterface *pOcaInterface       = &MosOcaInterfaceSpecific::GetInstance();
    MOS_STATUS status                    = MOS_STATUS_SUCCESS;
    MOS_OCA_BUFFER_HANDLE ocaBufHandle   = 0;
    uint32_t updateSize                  = 0;

    if (nullptr == pOcaInterface                                   ||
        !((MosOcaInterfaceSpecific*)pOcaInterface)->IsOcaEnabled() ||
        nullptr == pCodechalDumper)
    {
        return;
    }
    if ((ocaBufHandle = GetOcaBufferHandle(cmdBuffer, mosContext)) == MOS_OCA_INVALID_BUFFER_HANDLE)
    {
        // May come here for workloads not enabling UMD_OCA.
        return;
    }

    CODECHAL_OCA_DECODE_HEADER *pCodechalParam = ((CodechalOcaDumper *)pCodechalDumper)->GetDecodeParam();
    if (nullptr == pCodechalParam)
    {
        return;
    }

    MOS_OCA_LOG_HEADER_CODECHAL_PARAM header = {};
    header.header.type                       = MOS_OCA_LOG_TYPE_CODECHAL_PARAM;  // 00000007
    header.header.headerSize                 = sizeof(MOS_OCA_LOG_HEADER_CODECHAL_PARAM);
    header.header.dataSize                   = pCodechalParam->Header.size;
    header.codec                             = (uint32_t)codec;
    status = pOcaInterface->DumpDataBlock(ocaBufHandle, &mosContext, (PMOS_OCA_LOG_HEADER)&header, pCodechalParam);

    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
}

bool HalOcaInterfaceNext::IsLargeResouceDumpSupported()
{
    return true;
}

void HalOcaInterfaceNext::AddRTLogReource(MOS_COMMAND_BUFFER &cmdBuffer,
                               MOS_CONTEXT &mosContext,
                               MOS_INTERFACE &osInterface,
                               MHW_RESOURCE_PARAMS ResourceParams,
                               bool bUseSizeOfResource,
                               uint32_t sizeOfIndirectState)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    MosOcaRTLogMgr &ins = (MosOcaRTLogMgr &)MosOcaRTLogMgr::GetInstance();
    MOS_RESOURCE tempResource = ins.GetOcaRTlogResource();
    // MosOcaInterface *pOcaInterface = &MosOcaInterfaceSpecific::GetInstance();
    MOS_LINUX_BO *bo = cmdBuffer.OsResource.bo;
    OCA_LOG_SECTION_HEADER *header = (OCA_LOG_SECTION_HEADER *)((uint64_t)bo->virt + bo->size - OCA_LOG_SECTION_SIZE_MAX);
    if (header->magicNum != OCA_LOG_SECTION_MAGIC_NUMBER)
    {
        return;
    }

    MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));

    ResourceParams.dwOffset = 0;
    uint32_t cmd = 0;
    ResourceParams.presResource = &tempResource;
    ResourceParams.pdwCmd = &cmd;
    // Align logic in Mhw_AddResourceToCmd
    ResourceParams.dwLocationInCmd = ((int32_t)((uint8_t*)&header->rtlogPatchAddr - (uint8_t*)cmdBuffer.pCmdBase) - cmdBuffer.iOffset) / sizeof(uint32_t);
    ResourceParams.HwCommandType = MOS_OCA_RESERVED;
    ResourceParams.dwSharedMocsOffset = 1 - ResourceParams.dwLocationInCmd;

    if (osInterface.bUsesGfxAddress)
    {
        status = Mhw_AddResourceToCmd_GfxAddress(&osInterface, &cmdBuffer, &ResourceParams);
    }
    else
    {
        status = Mhw_AddResourceToCmd_PatchList(&osInterface, &cmdBuffer, &ResourceParams);
    }

    if (MOS_FAILED(status))
    {
        OnOcaError(&mosContext, status, __FUNCTION__, __LINE__);
    }
    HalOcaInterfaceNext::OnIndirectState(cmdBuffer, mosContext, &tempResource, 0, false, ins.m_ocaRtHeap.size/*tempMgr->m_RTLogSectionMgr->m_HeapSize*/);
}

void HalOcaInterfaceNext::DumpCpParam(MosOcaInterface &ocaInterface, MOS_OCA_BUFFER_HANDLE &ocaBufHandle, PMOS_CONTEXT mosCtx, void *pCpDumper)
{
}

void HalOcaInterfaceNext::OnOcaError(PMOS_CONTEXT mosCtx, MOS_STATUS status, const char *functionName, uint32_t lineNumber)
{
    MosOcaInterfaceSpecific::OnOcaError(mosCtx, status, functionName, lineNumber);
}

MOS_OCA_BUFFER_HANDLE HalOcaInterfaceNext::GetOcaBufferHandle(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext)
{
    MosOcaInterface *pOcaInterface = &MosOcaInterfaceSpecific::GetInstance();
    if(pOcaInterface == nullptr)
    {
        OnOcaError(&mosContext, MOS_STATUS_NULL_POINTER, __FUNCTION__, __LINE__);
        return MOS_OCA_INVALID_BUFFER_HANDLE;
    }
    PMOS_MUTEX mutex = pOcaInterface->GetMutex();
    if (mutex == nullptr)
    {
        return MOS_OCA_INVALID_BUFFER_HANDLE;
    }
    MosOcaAutoLock autoLock(mutex);
    std::map<uint32_t*, MOS_OCA_BUFFER_HANDLE>::iterator it = s_hOcaMap.find(cmdBuffer.pCmdBase);
    if (it == s_hOcaMap.end())
    {
        // May come here for workloads not enabling UMD_OCA.
        return MOS_OCA_INVALID_BUFFER_HANDLE;
    }
    if (it->second >= MAX_NUM_OF_OCA_BUF_CONTEXT || it->second < 0)
    {
        OnOcaError(&mosContext, MOS_STATUS_NULL_POINTER, __FUNCTION__, __LINE__);
        return MOS_OCA_INVALID_BUFFER_HANDLE;
    }
    return it->second;
}

void HalOcaInterfaceNext::RemoveOcaBufferHandle(MOS_COMMAND_BUFFER &cmdBuffer, MOS_CONTEXT &mosContext)
{
    MosOcaInterface *pOcaInterface       = &MosOcaInterfaceSpecific::GetInstance();
    if(pOcaInterface == nullptr)
    {
        OnOcaError(&mosContext, MOS_STATUS_NULL_POINTER, __FUNCTION__, __LINE__);
        return;
    }
    PMOS_MUTEX      mutex = pOcaInterface->GetMutex();
    if (mutex == nullptr)
    {
        OnOcaError(&mosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return;
    }
    MosOcaAutoLock autoLock(mutex);
    std::map<uint32_t*, MOS_OCA_BUFFER_HANDLE>::iterator it = s_hOcaMap.find(cmdBuffer.pCmdBase);
    if (it == s_hOcaMap.end())
    {
        return;
    }
    s_hOcaMap.erase(it);
}

/****************************************************************************************************/
/*                         Private functions to ensure class singleton.                             */
/****************************************************************************************************/
HalOcaInterfaceNext::HalOcaInterfaceNext()
{
}

HalOcaInterfaceNext::HalOcaInterfaceNext(HalOcaInterfaceNext &)
{
}

HalOcaInterfaceNext& HalOcaInterfaceNext::operator= (HalOcaInterfaceNext &)
{
    return *this;
}
