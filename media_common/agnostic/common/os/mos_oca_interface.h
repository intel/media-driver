/*
* Copyright (c) 2009-2021, Intel Corporation
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
//! \file     mos_oca_interface.h
//! \brief    Common interface and structure for OCA
//!

#ifndef __MOS_OCA_INTERFACE_H__
#define __MOS_OCA_INTERFACE_H__

typedef uint64_t MOS_OCA_BUFFER_HANDLE;
#define OCA_HEAP_INVALID_OFFSET ((uint32_t)-1)

#define MOS_OCA_INVALID_BUFFER_HANDLE -1

#define MAX_NUM_OF_OCA_BUF_CONTEXT   32

#define OCA_LOG_SECTION_SIZE_MAX        0x3000
#define OCA_LOG_SECTION_MAGIC_NUMBER    (0x5F691B7E574ACE30)

#define MOS_OCA_MAX_STRING_LEN          (1024)          //!< The max string len for MosOcaStateHeapLog::TraceMessage.
#define OCA_MAX_RESOURCE_INFO_COUNT_MAX 60

typedef struct _MOS_OCA_LOG_HEADER
{
    uint32_t    m_Type;             //!< Oca log type. Refer to MOS_OCA_LOG_TYPE.
    uint32_t    m_HeaderSize;       //!< The size for extented message header.
    uint32_t    m_DataSize;         //!< The size of data block without message header.
}MOS_OCA_LOG_HEADER, *PMOS_OCA_LOG_HEADER;

class MosOcaInterface
{
public:
    //!
    //! \brief  Destructor
    //!
    virtual ~MosOcaInterface() {}

    //!
    //! \brief  Get the idle oca buffer, which is neither used by hw nor locked, and lock it for edit.
    //! \param  [in] pMosContext
    //!         pointer to MOS_INTERFACE
    //! \param  [in] CurrentGpuContextHandle
    //!         Gpu context handle 
    //! \return MOS_OCA_BUFFER_HANDLE
    //!         return the handle for oca buffer
    //!
    virtual MOS_OCA_BUFFER_HANDLE LockOcaBufAvailable(PMOS_CONTEXT pMosContext, uint32_t CurrentGpuContextHandle)
    {
        return 0;
    }

    //!
    //! \brief  Unlock the oca buffer when edit complete.
    //! \param  [in] hOcaBuf
    //!         Oca buffer handle.
    //! \return MOS_STATUS
    //!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS UnlockOcaBuf(MOS_OCA_BUFFER_HANDLE hOcaBuf)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Oca operation which should be called at the beginning of 1st level batch buffer start.
    //! \param  [out] gpuVaOcaBuffer
    //!         The gfx virtual address of oca buffer, which should be set to GPR11 by LRI at the
    //!         beginning of 1st level batch buffer no matter return value is MOS_STATUS_SUCCESS or not.
    //! \param  [in] hOcaBuf
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
    virtual MOS_STATUS On1stLevelBBStart(uint64_t &gpuVaOcaBuffer, MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT pMosContext, void *pMosResource,
        uint32_t offsetOf1stLevelBB, bool bUseSizeOfResource, uint32_t sizeOf1stLevelBB)
    {
        // The GPR11 need to be reset to 0 to disable UMD_OCA for current workload.
        gpuVaOcaBuffer = 0;
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Oca operation which should be called before adding batch buffer end command for 1st
    //!         level batch buffer.
    //! \param  [in] hOcaBuf
    //!         Oca buffer handle.
    //! \param  [in] mosCtx
    //!         DDI device context.
    //! \return MOS_STATUS
    //!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS On1stLevelBBEnd(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT mosCtx)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Oca operation which should be called before sending start sub level batch buffer command.
    //! \param  [in] hOcaBuf
    //!         Oca buffer handle.
    //! \param  [in] pMosContext
    //!         Pointer to MOS_CONTEXT.
    //! \param  [in] pMosResource
    //!         Pointer to the MOS_RESOURCE.
    //! \param  [in] offsetOfSubLevelBB
    //!         Offset for current BB in pMosResource.
    //! \param  [in] bUseSizeOfResource
    //!         If true, use size of pMosResource for batch buffer, else use sizeOfIndirectState.
    //! \param  [in] sizeOfSubLevelBB
    //!         Size of BB. Ignore if bUseSizeOfResource == true.
    //! \return MOS_STATUS
    //!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS OnSubLevelBBStart(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT pMosContext, void *pMosResource, uint32_t offsetOfSubLevelBB, bool bUseSizeOfResource, uint32_t sizeOfSubLevelBB)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Oca operation which should be called when indirect states being added.
    //! \param  [in] hOcaBuf
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
    virtual MOS_STATUS OnIndirectState(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT pMosContext, void *pMosResource, uint32_t offsetOfIndirectState, bool bUseSizeOfResource, uint32_t sizeOfIndirectState)
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
    //! \param  [in] hOcaBuf
    //!         Oca buffer handle.
    //! \param  [in] mosCtx
    //!         DDI device context.
    //! \return MOS_STATUS
    //!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS OnDispatch(uint32_t &offsetInIndirectStateHeap, MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT mosCtx)
    {
        offsetInIndirectStateHeap = OCA_HEAP_INVALID_OFFSET;
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Add string to oca log section
    //! \param  [in] hOcaBuf
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
    virtual MOS_STATUS TraceMessage(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT mosCtx, const char *str, uint32_t maxCount)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Add resource to dump list.
    //! \param  [in] hOcaBuf
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
    virtual MOS_STATUS AddResourceToDumpList(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT mosCtx, MOS_RESOURCE &resource, MOS_HW_COMMAND hwCmdType, uint32_t locationInCmd, uint32_t offsetInRes)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Add data block to oca log section.
    //! \param  [in] hOcaBuf
    //!         Oca buffer handle.
    //! \param  [in] mosCtx
    //!         DDI device context.
    //! \param  [in] pHeader
    //!         Log header. It can be extended by user. The acutal size of header is pHeader->m_HeaderSize.
    //! \param  [in] pData
    //!         Data block without log header. The acutal size of data block is pHeader->m_DataSize.
    //! \return MOS_STATUS
    //!         Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS DumpDataBlock(MOS_OCA_BUFFER_HANDLE hOcaBuf, PMOS_CONTEXT mosCtx, PMOS_OCA_LOG_HEADER pHeader, void *pData)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    //!
    //! \brief  Get the mutex for oca buffer handles protection.
    //! \return PMOS_MUTEX
    //!         Return PMOS_MUTEX if successful, otherwise nullptr
    //!
    virtual PMOS_MUTEX GetMutex()
    {
        return nullptr;
    }
};

#endif // #ifndef __MOS_OCA_INTERFACE_H__
