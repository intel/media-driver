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
//! \file     mos_oca_specific.cpp
//! \brief    Linux specfic OCA class
//!

#include "mos_os.h"
#include "mos_os_specific.h"
#include "mos_oca_interface_specific.h"
#include "mos_util_debug.h"
#include "mos_utilities.h"
#include <algorithm>

//!
//! \def MOS_OCA_CHK_NULL_RETURN(_ptr)
//!  Check if _ptr == nullptr. If so, assert, return an error and update LastErrorStatus.
//!  Should only be used by interface functions in MosOcaInterface.
//!
#define MOS_OCA_CHK_NULL_RETURN(mosCtx, _ptr, func, line)                               \
{                                                                                       \
    if ((_ptr) == nullptr)                                                              \
    {                                                                                   \
        MosOcaInterfaceSpecific::OnOcaError(mosCtx, MOS_STATUS_NULL_POINTER, func, line);    \
        return MOS_STATUS_NULL_POINTER;                                                 \
    }                                                                                   \
}

//!
//! \def MOS_OCA_CHK_STATUS_RETURN(_stmt)
//!  Check if MOS_STATUS_SUCCESS != _stmt. If so, assert, return an error and update LastErrorStatus.
//!  Should only be used by interface functions in MosOcaInterface.
//!
#define MOS_OCA_CHK_STATUS_RETURN(mosCtx, _stmt, func, line)                            \
    {                                                                                   \
    MOS_STATUS stmtStatus = (MOS_STATUS)(_stmt);                                        \
    if (stmtStatus != MOS_STATUS_SUCCESS)                                               \
    {                                                                                   \
        MosOcaInterfaceSpecific::OnOcaError(mosCtx, stmtStatus, func, line);                 \
        return stmtStatus;                                                              \
    }                                                                                   \
}

/****************************************************************************************************/
/*                                     MosOcaInterfaceSpecific                                              */
/****************************************************************************************************/


MOS_STATUS  MosOcaInterfaceSpecific::s_ocaStatus          = MOS_STATUS_SUCCESS;
uint32_t MosOcaInterfaceSpecific::s_lineNumForOcaErr      = 0;
bool MosOcaInterfaceSpecific::s_bOcaStatusExistInReg      = false;
int32_t MosOcaInterfaceSpecific::s_refCount               = 0;

//!
//! \brief  Get the idle oca buffer, which is neither used by hw nor locked, and lock it for edit.
//! \param  [in] pMosContext
//!         pointer to MOS_INTERFACE
//! \param  [in] CurrentGpuContextHandle
//!         Gpu context handle 
//! \return MOS_OCA_BUFFER_HANDLE
//!         return the handle for oca buffer
//!
MOS_OCA_BUFFER_HANDLE MosOcaInterfaceSpecific::LockOcaBufAvailable(PMOS_CONTEXT pMosContext, uint32_t CurrentGpuContextHandle)
{
    for (int i = m_indexOfNextOcaBufContext; i < MAX_NUM_OF_OCA_BUF_CONTEXT; ++i)
    {
        if (m_ocaBufContextList[i].inUse)
        {
            continue;
        }
        m_ocaBufContextList[i].inUse = true;
        m_ocaBufContextList[i].logSection.resInfo.maxResInfoCount = m_config.maxResInfoCount;
        m_indexOfNextOcaBufContext = (i + 1) % MAX_NUM_OF_OCA_BUF_CONTEXT;
        return i;
    }

    for (int i = 0; i < m_indexOfNextOcaBufContext; ++i)
    {
        if (m_ocaBufContextList[i].inUse)
        {
            continue;
        }
        m_ocaBufContextList[i].inUse = true;
        m_ocaBufContextList[i].logSection.resInfo.maxResInfoCount = m_config.maxResInfoCount;
        m_indexOfNextOcaBufContext = (i + 1) % MAX_NUM_OF_OCA_BUF_CONTEXT;
        return i;
    }
    MosOcaInterfaceSpecific::OnOcaError(pMosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
    return MOS_OCA_INVALID_BUFFER_HANDLE;
}

//!
//! \brief  Unlock the oca buffer when edit complete.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::UnlockOcaBuf(MOS_OCA_BUFFER_HANDLE ocaBufHandle)
{
    if (ocaBufHandle >= MAX_NUM_OF_OCA_BUF_CONTEXT || ocaBufHandle < 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_ocaBufContextList[ocaBufHandle].inUse               = false;
    m_ocaBufContextList[ocaBufHandle].logSection.offset   = 0;
    m_ocaBufContextList[ocaBufHandle].logSection.base     = nullptr;
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
//! \param  [out] gpuVaOcaBuffer
//!         The gfx virtual address of oca buffer, which should be set to GPR11 by LRI at the
//!         beginning of 1st level batch buffer no matter return value is MOS_STATUS_SUCCESS or not.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] pMosContext
//!         Pointer to MOS_CONTEXT.
//! \param  [in] pMosResource
//!         Pointer to the MOS_RESOURCE.
//! \param  [in] offsetOf1stLevelBB
//!         Offset for current BB in pMosResource.
//! \param  [in] bUseSizeOfResource
//!         If true, use size of pMosResource for batch buffer, else use sizeOf1stLevelBB.
//! \param  [in] sizeOf1stLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::On1stLevelBBStart(uint64_t &gpuVaOcaBuffer, MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT pMosContext, void *pMosResource, uint32_t offsetOf1stLevelBB, bool bUseSizeOfResource, uint32_t sizeOf1stLevelBB)
{
    // The GPR11 need to be reset to 0 to disable UMD_OCA for current workload if any error happened.
    gpuVaOcaBuffer = 0;
    if (!m_isOcaEnabled)
    {
        return MOS_STATUS_SUCCESS; 
    }

    PMOS_RESOURCE pOsResource = (PMOS_RESOURCE)pMosResource;
    if (ocaBufHandle >= MAX_NUM_OF_OCA_BUF_CONTEXT || ocaBufHandle < 0)
    {
        MosOcaInterfaceSpecific::OnOcaError(pMosContext, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    m_ocaBufContextList[ocaBufHandle].is1stLevelBBStarted = true;
    InitLogSection(ocaBufHandle, pOsResource);  
    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Oca operation which should be called before adding batch buffer end command for 1st
//!         level batch buffer.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] mosCtx
//!         DDI device context.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::On1stLevelBBEnd(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx)
{
    if (!m_isOcaEnabled)
    {
        return MOS_STATUS_SUCCESS; 
    }
    if (ocaBufHandle >= MAX_NUM_OF_OCA_BUF_CONTEXT || ocaBufHandle < 0)
    {
        MosOcaInterfaceSpecific::OnOcaError(mosCtx, MOS_STATUS_INVALID_PARAMETER, __FUNCTION__, __LINE__);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!m_ocaBufContextList[ocaBufHandle].is1stLevelBBStarted)
    {
        MOS_OS_ASSERTMESSAGE("On1stLevelBBEnd is called without On1stLevelBBStart being called!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    AddResourceInfoToLogSection(ocaBufHandle, mosCtx);
    m_ocaBufContextList[ocaBufHandle].is1stLevelBBStarted = false;

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  Oca operation which should be called before sending start sub level batch buffer command.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] pMosContext
//!         Pointer to MOS_CONTEXT.
//! \param  [in] pMosResource
//!         Pointer to the MOS_RESOURCE.
//! \param  [in] offsetOfSubLevelBB
//!         Offset for current BB in pMosResource.
//! \param  [in] bUseSizeOfResource
//!         If true, use size of pMosResource for batch buffer, else use sizeOfSubLevelBB.
//! \param  [in] sizeOfSubLevelBB
//!         Size of BB. Ignore if bUseSizeOfResource == true.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::OnSubLevelBBStart(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT pMosContext, void *pMosResource, uint32_t offsetOfSubLevelBB, bool bUseSizeOfResource, uint32_t sizeOfSubLevelBB)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

//!
//! \brief  Oca operation which should be called when indirect states being added.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] pMosContext
//!         Pointer to MOS_CONTEXT.
//! \param  [in] pMosResource
//!         Pointer to the MOS_RESOURCE.
//! \param  [in] offsetOfIndirectState
//!         Offset for current state in pMosResource.
//! \param  [in] bUseSizeOfResource
//!         If true, use size of pMosResource for indirect state, else use sizeOfIndirectState.
//! \param  [in] sizeOfIndirectState
//!         Size of indirect state. Ignore if bUseSizeOfResource == true.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::OnIndirectState(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT pMosContext, void *pMosResource, uint32_t offsetOfIndirectState, bool bUseSizeOfResource, uint32_t sizeOfIndirectState)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

//!
//! \brief  Oca operation which should be called before adding dispatch states,
//!         e.g. VEB_DI_IECP_STATE and MEDIA_OBJECT_WALKER.
//! \param  [out] offsetInIndirectStateHeap
//!         The start offset of current dispatch in indirect state heap, which should be set to low 32 bits
//!         of GPR12 by LRI before dispatch commands being added.
//!         OCA_HEAP_INVALID_OFFSET means no need to configure GPR12, otherwise the register need be configured
//!         no matter return value being MOS_STATUS_SUCCESS or not.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] mosCtx
//!         DDI device context.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::OnDispatch(uint32_t &offsetInIndirectStateHeap, MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

//!
//! \brief  Add string to oca log section
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] mosCtx
//!         DDI device context.
//! \param  [in] str
//!         string to be added.
//! \param  [in] maxCount
//!         size of the buffer pointed by str.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::TraceMessage(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx, const char *str, uint32_t maxCount)
{
    if (!m_isOcaEnabled)
    {
        return MOS_STATUS_SUCCESS; 
    }
    if (nullptr == str || maxCount <= 0 || maxCount > MOS_OCA_MAX_STRING_LEN)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    if (!IsLogSectionEnabled(ocaBufHandle))
    {
        return MOS_STATUS_SUCCESS;
    }

    uint32_t len = strnlen(str, maxCount);
    MOS_OCA_LOG_HEADER header = {MOS_OCA_LOG_TYPE_STRING, sizeof(MOS_OCA_LOG_HEADER), len};

    return DumpDataBlock(ocaBufHandle, &header, (void *)str);
}

//!
//! \brief  Add resource to dump list.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] mosCtx
//!         DDI device context.
//! \param  [in] resource
//!         Reference to MOS_RESOURCE.
//! \param  [in] hwCmdType
//!         Hw command Type.
//! \param  [in] locationInCmd
//!         Location in command.
//! \param  [in] offsetInRes
//!         Offset in resource.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::AddResourceToDumpList(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx, MOS_RESOURCE &resource, MOS_HW_COMMAND hwCmdType, uint32_t locationInCmd, uint32_t offsetInRes)
{
    uint32_t            i           = 0;
    MOS_MEMCOMP_STATE   resMmcMode  = {};
    if (!m_isOcaEnabled || !m_isInitialized)
    {
        return MOS_STATUS_SUCCESS; 
    }

    if (!m_ocaBufContextList[ocaBufHandle].is1stLevelBBStarted)
    {
        MOS_OS_ASSERTMESSAGE("AddResourceToDumpList is called before oca buffer initialized!");
        MosOcaInterfaceSpecific::OnOcaError(mosCtx, MOS_STATUS_UNINITIALIZED, __FUNCTION__, __LINE__);
        return MOS_STATUS_UNINITIALIZED;
    }

    if (nullptr == m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList)
    {
        MOS_OS_NORMALMESSAGE("OCA resource info dump not enabled.");
        return MOS_STATUS_SUCCESS;
    }

    if (nullptr == resource.pGmmResInfo || Mos_ResourceIsNull(&resource))
    {
        return MOS_STATUS_SUCCESS;
    }

    if (hwCmdType >= MOS_HW_COMMAND_MAX || MOS_HW_COMMAND_MAX > 64)
    {
        MosOcaInterfaceSpecific::OnOcaError(mosCtx, MOS_STATUS_UNINITIALIZED, __FUNCTION__, __LINE__);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount; ++i)
    {
        if (m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].allocationHandle == resource.bo->handle)
        {
            if (offsetInRes > m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].offsetInRes)
            {
                // Only update resource info for the largest offsetInRes case.
                m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].hwCmdType     = (uint32_t)hwCmdType;
                m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].offsetInRes   = offsetInRes;
                m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].locationInCmd = locationInCmd;
            }
            return MOS_STATUS_SUCCESS;
        }
    }

    if (m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount >= m_ocaBufContextList[ocaBufHandle].logSection.resInfo.maxResInfoCount)
    {
        // Not reture error but record the resource count skipped.
        ++m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCountSkipped;
        MOS_OS_NORMALMESSAGE("The resource is skipped to be dumpped to oca buffer.");
        return MOS_STATUS_SUCCESS;
    }

    MosStreamState steamState = {};
    steamState.osDeviceContext = mosCtx->m_osDeviceContext;
    if (MOS_FAILED(MosInterface::GetMemoryCompressionMode(&steamState, &resource, resMmcMode)))
    {
        resMmcMode = MOS_MEMCOMP_DISABLED;
    }
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].gfxAddress               = resource.bo->offset64;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].sizeAllocation           = resource.pGmmResInfo->GetSizeAllocation();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].sizeSurface              = resource.pGmmResInfo->GetSizeSurface();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].sizeSurfacePhy           = resource.pGmmResInfo->GetSizeSurface();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].sizeMainSurface          = resource.pGmmResInfo->GetSizeMainSurface();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].allocationHandle         = resource.bo->handle;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].hwCmdType                = (uint32_t)hwCmdType;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].locationInCmd            = locationInCmd;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].offsetInRes              = offsetInRes;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].pitch                    = (uint32_t)resource.pGmmResInfo->GetRenderPitch();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].width                    = (uint32_t)resource.pGmmResInfo->GetBaseWidth();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].height                   = (uint32_t)resource.pGmmResInfo->GetBaseHeight();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].format                   = (uint32_t)resource.Format;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].gmmFormat                = (uint32_t)resource.pGmmResInfo->GetResourceFormat();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].gmmTileMode              = (uint32_t)resource.pGmmResInfo->GmmGetTileMode();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].gmmClient                = (uint32_t)resource.pGmmResInfo->GetClientType();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].gmmResUsageType          = (uint32_t)resource.pGmmResInfo->GetCachePolicyUsage();
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].mmcMode                  = (uint32_t)resMmcMode;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].mmcHint                  = (uint32_t)resource.pGmmResInfo->GetMmcHint(0);
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].auxYOffset               = resource.pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_Y_CCS);
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].auxUVOffset              = resource.pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_UV_CCS);
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].auxCCSOffset             = resource.pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CCS);
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].auxCCOffset              = resource.pGmmResInfo->GetUnifiedAuxSurfaceOffset(GMM_AUX_CC);

    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isLocalOnly        = resource.pGmmResInfo->GetResFlags().Info.LocalOnly;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isNonLocalOnly     = resource.pGmmResInfo->GetResFlags().Info.NonLocalOnly;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isNotLockable      = resource.pGmmResInfo->GetResFlags().Info.NotLockable;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isShared           = resource.pGmmResInfo->GetResFlags().Info.Shared;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isCameraCapture    = resource.pGmmResInfo->GetResFlags().Gpu.CameraCapture;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList[i].flags.isRenderTarget     = resource.pGmmResInfo->GetResFlags().Gpu.RenderTarget;

    ++m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount;

    return MOS_STATUS_SUCCESS;
}

void MosOcaInterfaceSpecific::AddResourceInfoToLogSection(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx)
{
    if (!m_ocaBufContextList[ocaBufHandle].is1stLevelBBStarted)
    {
        return;
    }
    if (0 == m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount)
    {
        return;
    }
    if (!IsLogSectionEnabled(ocaBufHandle))
    {
        return;
    }

    MOS_OCA_LOG_HEADER_RESOURCE_INFO header = {};
    header.header.type                      = MOS_OCA_LOG_TYPE_RESOURCE_INFO;
    header.header.headerSize                = sizeof(MOS_OCA_LOG_HEADER_RESOURCE_INFO);
    header.header.dataSize                  = m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount * sizeof(MOS_OCA_RESOURCE_INFO);
    header.resCount                         = m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount;
    header.resCountSkipped                  = m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCountSkipped;

    MOS_STATUS status = DumpDataBlock(ocaBufHandle, (PMOS_OCA_LOG_HEADER)&header, m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resInfoList);
    if (MOS_FAILED(status))
    {
       MosOcaInterfaceSpecific::OnOcaError(mosCtx, status, __FUNCTION__, __LINE__);
    }
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount        = 0;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCountSkipped = 0;
    return;
}

//!
//! \brief  Add data block to oca log section.
//! \param  [in] ocaBufHandle
//!         Oca buffer handle.
//! \param  [in] mosCtx
//!         DDI device context.
//! \param  [in] pHeader
//!         Log header. It can be extended by user. The acutal size of header is pHeader->headerSize.
//! \param  [in] pData
//!         Data block without log header. The acutal size of data block is pHeader->dataSize.
//! \return MOS_STATUS
//!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS MosOcaInterfaceSpecific::DumpDataBlock(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_CONTEXT mosCtx, PMOS_OCA_LOG_HEADER pHeader, void *pData)
{
    if (!m_isOcaEnabled)
    {
        return MOS_STATUS_SUCCESS; 
    }
    if (ocaBufHandle >= MAX_NUM_OF_OCA_BUF_CONTEXT || nullptr == pHeader)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (!IsLogSectionEnabled(ocaBufHandle))
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_OCA_CHK_STATUS_RETURN(mosCtx, DumpDataBlock(ocaBufHandle, pHeader, pData), __FUNCTION__, __LINE__);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaInterfaceSpecific::DumpDataBlock(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_OCA_LOG_HEADER pHeader, void *pData)
{
    if (pHeader->headerSize < sizeof(MOS_OCA_LOG_HEADER) ||
        pHeader->type >= MOS_OCA_LOG_TYPE_COUNT || pHeader->type <= MOS_OCA_LOG_TYPE_INVALID ||
        (pHeader->dataSize > 0 && nullptr == pData) || (pHeader->dataSize == 0 && pData))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t copySize = pHeader->headerSize + pHeader->dataSize;
    if (m_ocaBufContextList[ocaBufHandle].logSection.offset + copySize > m_ocaLogSectionSizeLimit)
    {
        return MOS_STATUS_NOT_ENOUGH_BUFFER;
    }

    MOS_OS_CHK_STATUS_RETURN(InsertData(ocaBufHandle, (uint8_t *)pHeader, pHeader->headerSize));
    if (pHeader->dataSize > 0)
    {
        MOS_OS_CHK_STATUS_RETURN(InsertData(ocaBufHandle, (uint8_t *)pData, pHeader->dataSize));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MosOcaInterfaceSpecific::InsertData(MOS_OCA_BUFFER_HANDLE ocaBufHandle, uint8_t *p, uint32_t size)
{
    if (nullptr == p || 0 == size)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    uint32_t sizeCpy = std::min(m_ocaLogSectionSizeLimit - m_ocaBufContextList[ocaBufHandle].logSection.offset, size);
    MOS_OS_CHK_STATUS_RETURN(MOS_SecureMemcpy((uint8_t *)m_ocaBufContextList[ocaBufHandle].logSection.base + m_ocaBufContextList[ocaBufHandle].logSection.offset, sizeCpy, p, sizeCpy));
    m_ocaBufContextList[ocaBufHandle].logSection.offset += sizeCpy;
    return MOS_STATUS_SUCCESS;
}

uint32_t MosOcaInterfaceSpecific::IncreaseSize(uint32_t cmdBufSize)
{
    return cmdBufSize + OCA_LOG_SECTION_SIZE_MAX;
}

void MosOcaInterfaceSpecific::InitOcaLogSection(MOS_LINUX_BO *bo)
{
    if (bo == nullptr || bo->virt == nullptr || bo->size <= OCA_LOG_SECTION_SIZE_MAX)
    {
        return;
    }
    uint64_t *ocaLogSectionBaseAddr = (uint64_t *)((uint64_t)bo->virt + bo->size - OCA_LOG_SECTION_SIZE_MAX);
    *ocaLogSectionBaseAddr = OCA_LOG_SECTION_MAGIC_NUMBER;
}

void MosOcaInterfaceSpecific::InitLogSection(MOS_OCA_BUFFER_HANDLE ocaBufHandle, PMOS_RESOURCE resCmdBuf)
{
    MOS_LINUX_BO *boCmdBuf = resCmdBuf->bo;
    if (boCmdBuf->size <= OCA_LOG_SECTION_SIZE_MAX)
    {
        MOS_OS_NORMALMESSAGE("1st level BB size is not larger than OCA_LOG_SECTION_SIZE_MAX.");
        return;
    }

    uint64_t *logSectionBase = (uint64_t*)((char *)boCmdBuf->virt + boCmdBuf->size - OCA_LOG_SECTION_SIZE_MAX);
    uint64_t magicNum = *logSectionBase;
    if (OCA_LOG_SECTION_MAGIC_NUMBER != magicNum)
    {
        MOS_OS_NORMALMESSAGE("Log section not exists in current 1st level BB.");
        return;
    }

    m_ocaBufContextList[ocaBufHandle].logSection.base                    = logSectionBase;
    // Reserve an uint64 for magic number;
    m_ocaBufContextList[ocaBufHandle].logSection.offset                  = sizeof(uint64_t);
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCount        = 0;
    m_ocaBufContextList[ocaBufHandle].logSection.resInfo.resCountSkipped = 0;
}

bool MosOcaInterfaceSpecific::IsLogSectionEnabled(MOS_OCA_BUFFER_HANDLE ocaBufHandle)
{
    return nullptr != m_ocaBufContextList[ocaBufHandle].logSection.base;
}

//!
//! \brief  Oca Error Handler.
//! \param  [in] mosCtx
//!         DDI device context.
//! \param  [in] status
//!         status for error.
//! \param  [in] functionName
//!         The failure function name.
//! \param  [in] lineNumber
//!         The line number where the failure happened.
//!
void MosOcaInterfaceSpecific::OnOcaError(PMOS_CONTEXT mosCtx, MOS_STATUS status, const char *functionName, uint32_t lineNumber)
{
    if (MOS_SUCCEEDED(status))
    {
        return;
    }

    MOS_OS_ASSERTMESSAGE("%s failed at line %d with status = %d!", functionName, lineNumber, status);

    MOS_TraceEventExt(EVENT_OCA_ERROR,
                      EVENT_TYPE_INFO,
                      &status,
                      sizeof(uint32_t),
                      nullptr,
                      0);

    if (MOS_SUCCEEDED(s_ocaStatus))
    {
        s_ocaStatus = status;
        s_lineNumForOcaErr = lineNumber;

        MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
        UserFeatureWriteData.Value.i32Data = status;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_OCA_STATUS_ID;
        MOS_UserFeature_WriteValues_ID(NULL, &UserFeatureWriteData, 1, mosCtx);

        UserFeatureWriteData.Value.i32Data = lineNumber;
        UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_OCA_ERROR_HINT_ID;
        MOS_UserFeature_WriteValues_ID(NULL, &UserFeatureWriteData, 1, mosCtx);
    }
}

//!
//! \brief  Initialize oca error handler related items.
//!
void MosOcaInterfaceSpecific::InitOcaErrorHandler()
{
    if (!s_bOcaStatusExistInReg)
    {
        s_bOcaStatusExistInReg = true;

        // "Oca Status" should already be added into reg for MOS_FAILED(s_ocaStatus) case by OnOcaError.
        if (MOS_SUCCEEDED(s_ocaStatus))
        {
            MOS_USER_FEATURE_VALUE_WRITE_DATA UserFeatureWriteData = __NULL_USER_FEATURE_VALUE_WRITE_DATA__;
            UserFeatureWriteData.Value.i32Data = s_ocaStatus;
            UserFeatureWriteData.ValueID = __MEDIA_USER_FEATURE_VALUE_OCA_STATUS_ID;
            MOS_UserFeature_WriteValues_ID(NULL, &UserFeatureWriteData, 1, (MOS_CONTEXT_HANDLE) nullptr);
        }
    }
}

//!
//! \brief  Oca Interface Initialize.
//!
void MosOcaInterfaceSpecific::Initialize()
{
    MosOcaInterfaceSpecific::InitOcaErrorHandler();
    if (m_isInitialized == false)
    {
        // read isOcaEnabled from reg key
        MOS_USER_FEATURE_VALUE_DATA userFeatureData     = {};
        MosUtilities::MosZeroMemory(&userFeatureData, sizeof(userFeatureData));
        MosUtilities::MosUserFeatureReadValueID(
            nullptr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_UMD_OCA_ID,
            &userFeatureData,
            (MOS_CONTEXT_HANDLE) nullptr);
        m_isOcaEnabled = (int32_t)userFeatureData.i32Data;
        if (!m_isOcaEnabled)
        {
            return;
        }
        m_config.maxResInfoCount = OCA_MAX_RESOURCE_INFO_COUNT_MAX;
        m_resInfoPool = MOS_NewArray(MOS_OCA_RESOURCE_INFO, m_config.maxResInfoCount * MAX_NUM_OF_OCA_BUF_CONTEXT);
        if (nullptr == m_resInfoPool)
        {
            MOS_DeleteArray(m_resInfoPool);
            return;
        }
        MosUtilities::MosZeroMemory(m_resInfoPool, sizeof(MOS_OCA_RESOURCE_INFO)*m_config.maxResInfoCount * MAX_NUM_OF_OCA_BUF_CONTEXT);
        for (int i = 0; i < MAX_NUM_OF_OCA_BUF_CONTEXT; ++i)
        {
            m_ocaBufContextList[i].logSection.resInfo.resInfoList = m_resInfoPool + i * m_config.maxResInfoCount;
        }
        if (nullptr != m_ocaMutex)
        {
            return;
        }

        m_ocaMutex = MosUtilities::MosCreateMutex();
        if (nullptr == m_ocaMutex)
        {
            return;
        }

        m_isInitialized = true;
    }
}

MosOcaInterface& MosOcaInterfaceSpecific::GetInstance()
{
    static MosOcaInterfaceSpecific instance;
    return instance;
}

MosOcaInterfaceSpecific::MosOcaInterfaceSpecific()
{
}

MosOcaInterfaceSpecific::MosOcaInterfaceSpecific(MosOcaInterfaceSpecific&)
{
}

MosOcaInterfaceSpecific& MosOcaInterfaceSpecific::operator= (MosOcaInterfaceSpecific&)
{
    return *this;
}

MosOcaInterfaceSpecific::~MosOcaInterfaceSpecific()
{
    Uninitialize();
}

//!
//! \brief  Oca Interface uninitialize.
//!
void MosOcaInterfaceSpecific::Uninitialize()
{
    if (m_isInitialized == true)
    {
        if (nullptr != m_ocaMutex)
        {
            MosUtilities::MosDestroyMutex(m_ocaMutex);
            m_ocaMutex = nullptr;
        }
        if (m_resInfoPool != nullptr)
        {
            MOS_DeleteArray(m_resInfoPool);
            m_resInfoPool = nullptr;
            for (int i = 0; i < MAX_NUM_OF_OCA_BUF_CONTEXT; ++i)
            {
                m_ocaBufContextList[i].logSection.resInfo.resInfoList = nullptr;
            }
        }
        m_isInitialized = false;
    }
}

//!
//! \brief  Oca Interface Initialize.
//!
void MosOcaInterfaceSpecific::InitInterface()
{
    if (MosUtilities::MosAtomicIncrement(&s_refCount) == 1)
    {
        MosOcaInterfaceSpecific &ins = (MosOcaInterfaceSpecific &)MosOcaInterfaceSpecific::GetInstance();
        ins.Initialize();
    }
    return;
}

//!
//! \brief  Oca Interface uninitialize.
//!
void MosOcaInterfaceSpecific::UninitInterface()
{
    if (MosUtilities::MosAtomicDecrement(&s_refCount) == 0)
    {
        MosOcaInterfaceSpecific &ins = (MosOcaInterfaceSpecific &)MosOcaInterfaceSpecific::GetInstance();
        ins.Uninitialize();
    }
    return;
}

