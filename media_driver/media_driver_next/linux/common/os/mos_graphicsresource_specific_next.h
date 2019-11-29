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
//! \file     mos_graphicsresource_specific_next.h
//! \brief   Container class for the linux/Android specfic graphic resource
//!

#ifndef __GRAPHICS_RESOURCE_SPECIFIC_NEXT_H__
#define __GRAPHICS_RESOURCE_SPECIFIC_NEXT_H__

#include "mos_graphicsresource_next.h"

class GraphicsResourceSpecificNext : public GraphicsResourceNext
{
public:
    //!
    //! \brief  Constructor
    //!
    GraphicsResourceSpecificNext();

    //!
    //! \brief  Destructor
    //!
    ~GraphicsResourceSpecificNext();

    //!
    //! \brief  to sync render target for multi-threading decoding mode
    //!
    struct HybridSem
    {
    public:
        //!
        //! \brief   Semaphore queue for hybrid decoding multi-threading case
        //!
        PMOS_SEMAPHORE* m_pCurrentFrameSemaphore   = nullptr;

        //!
        //! \brief   Semaphore queue for hybrid decoding multi-threading case; post when a
        //!          surface is not used as reference frame
        //!
        PMOS_SEMAPHORE* m_pReferenceFrameSemaphore = nullptr;

        //!
        //! \brief   Flag to mark whether the semaphore has been initialized
        //!
        bool m_semInitialized = false;

    };

    //!
    //! \brief  Add a sync tag to the graphic resource
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Parameters to do the synchronization
    //! \return MOS_SUCCESS on success case
    //!
    MOS_STATUS SetSyncTag(OsContextNext* osContextPtr, SyncParams& params, uint32_t streamIndex);

    //!
    //! \brief  Check whether the resource is nullptr
    //! \return ture if the resource is nullptr, false on other cases
    //!
    bool ResourceIsNull();

    //!
    //! \brief  Allocate the graphic memory to back up the graphic resource
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Resource creation Params
    //! \return MOS_STATUS_SUCCESS on success case, MOS error status on fail cases
    //!
    MOS_STATUS Allocate(OsContextNext* osContextPtr, CreateParams& params);

    //!
    //! \brief  Frees the specified resource with flag, if locked, unlocks it.
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] freeFlag
    //!         flags for the free operation
    //!
    void Free(OsContextNext* osContextPtr, uint32_t  freeFlag = 0);

    //!
    //! \brief  Check whether the specific graphic resouces is equal to the current one
    //! \param  [in] toCompare
    //!         ptr to the graphics resource to be compared with 
    //! \return Returns true if the two resources are equal and false otherwise.
    //!
    bool IsEqual(GraphicsResourceNext* toCompare);

    //!
    //! \brief  Check whether the current graphic resouce is valid
    //! \return Returns true if a resource is valid and false otherwise.
    //!
    bool IsValid();

    //!
    //! \brief  Locks a resource and returns a mapped system memory pointer.
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \param  [in] params
    //!         Resource lock Params
    //! \return CPU side lock address in success case, nullptr in fail cases
    //!
    void* Lock(OsContextNext* osContextPtr, LockParams& params);

    //!
    //! \brief  Unlocks a resource that has already been locked, if no lock has
    //!         occurred, this function does nothing
    //! \param  [in] osContextPtr
    //!         Pointer to the osContext handle
    //! \return MOS_SUCCESS in success case, MOS error status in fail cases
    //!
    MOS_STATUS Unlock(OsContextNext* osContextPtr);

    //!
    //! \brief  Converts an OS specific resource to a MOS resource.
    //! \param  [in] mosResourcePtr 
    //!         ptr to the MosResource to be filled w/ the conversion result
    //! \return MOS_SUCCESS on success case
    //!
    MOS_STATUS ConvertToMosResource(MOS_RESOURCE* mosResourcePtr);

    MOS_LINUX_BO*  GetBufferObject(){return m_bo;};
    
    //!
    //! \brief    Allocate External Resource
    //! \details  Derive the MOS_RESOURCE_HANDLE to indicate external resource.
    //!           It could be user provided resource with no GraphicResource info firstly
    //!           and get the info later; Or the resource converted from DDI.
    //!
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] params
    //!           Pointer to the parameters for allocating resource
    //! \param    [out] resource
    //!           MOS Resource handle of the allocated resource.
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS AllocateExternalResource(
        MOS_STREAM_HANDLE        streamState,
        PMOS_ALLOC_GFXRES_PARAMS params,
        MOS_RESOURCE_HANDLE &    resource);

    //!
    //! \brief    Free External Resource
    //! \details  Free the MOS_RESOURCE_HANDLE of external resource. 
    //!           Undefined behavior if using to free internal resource.
    //!
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] resource
    //!           MOS Resource handle of the allocated resource.
    //! \param    [in] flag
    //!           User defined free flag of the resource.
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS FreeExternalResource(
        MOS_STREAM_HANDLE   streamState,
        MOS_RESOURCE_HANDLE resource,
        uint32_t            flag);

    //!
    //! \brief    Lock External Resource
    //!
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] resource
    //!           MOS Resource handle of the resource to lock.
    //! \param    [in] flags
    //!           Control flags of locking resource.
    //!
    //! \return   void *
    //!           Locked memory data pointer, nullptr if lock failed.
    //!
    static void *LockExternalResource(
        MOS_STREAM_HANDLE   streamState,
        MOS_RESOURCE_HANDLE resource,
        PMOS_LOCK_PARAMS    flags);

    //!
    //! \brief    Unlock External Resource
    //!
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] resource
    //!           MOS Resource handle of the allocated resource.
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    static MOS_STATUS UnlockExternalResource(
        MOS_STREAM_HANDLE   streamState,
        MOS_RESOURCE_HANDLE resource);

protected:
    //!
    //! \brief  Converts MOS format infot GMM format.
    //! \return the converting result.
    //!
    GMM_RESOURCE_FORMAT ConvertMosFmtToGmmFmt(MOS_FORMAT format);

private:

    //!
    //! \brief  Pointer to the GMM info structure
    //!
    GMM_RESOURCE_INFO* m_gmmResInfo = nullptr;

    //!
    //! \brief  Whether the graphic resource is mapped at CPU side
    //!
    bool m_mapped = false;

    //!
    //! \brief  The map operation type we use
    //!
    MOS_MMAP_OPERATION m_mmapOperation = MOS_MMAP_OPERATION_NONE;

    //!
    //! \brief  the ptr to the buffer object of the graphic buffer
    //!
    MOS_LINUX_BO* m_bo = nullptr;

    //!
    //! \brief  the semaphore used in hybrid decoder corresponding to the graphic buffer
    //!
    HybridSem m_hybridSem = {};

    uint8_t*  m_systemShadow = nullptr;     //!< System shadow surface for s/w untiling
};
#endif // #ifndef __GRAPHICS_RESOURCE_SPECIFIC_NEXT_H__

