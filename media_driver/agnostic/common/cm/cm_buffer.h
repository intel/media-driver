/*
* Copyright (c) 2017, Intel Corporation
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
//! \file      cm_buffer.h 
//! \brief     Contains  Class CmBuffer/CmBufferUp definitions 
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFER_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFER_H_

#include "cm_def.h"

// Parameters used to set the surface state of the buffer
struct CM_BUFFER_STATE_PARAM
{
    uint32_t uiSize;
    uint32_t uiBaseAddressOffset;
    CM_SURFACE_MEM_OBJ_CTRL mocs;
};

namespace CMRT_UMD
{
class CmEvent;

//! \brief      CmBufferSVM Class to manage SVM (Shared Virtual Memory) resource.
//! \details    CmBufferSVM represents a 1D surface in SVM (Shared Virtual Memory) 
//!             memory space that is shared between CPU and GPU. The SVM memory must 
//!             be page(4K Bytes) aligned and runtime allocates SVM memory when 
//!             CmDevice::CreateBufferSVM is called. CPU can access the memory by using 
//!             the address returned by GetAddress. GPU can access the memory in two ways
//!             One way is to use the surfaceIndex returned by GetIndex, similiar to all
//!             other surfaces and buffers. The other way is to pass the address to CM 
//!             kernel function(genx_main) as an argument for kernel to use.
//! \note       Right now BufferSVM feature is not working on Linux yet.
class CmBufferSVM
{
public:

    //!
    //! \brief      This function returns the SurfaceIndex object associated 
    //!             with the SVM resource.
    //! \param      [out] index
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &index) = 0;

    //!
    //! \brief      Get the pointer of allocated SVM memory starting address.
    //! \param      [out] addr
    //!             return the allocated SVM memory starting address.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetAddress(void* &addr) = 0;
};

//!
//! \brief      CmBufferUP class to manage 1D surface in user provided system memory.
//! \details    CmBufferUP represents a 1D surface in system memory. It is 
//!             created upon the UP(User Provided) memory. The UP memory 
//!             must be page(4K Bytes) aligned. CPU can access the memory 
//!             as usual.Each CmBufferUP object is associated with a SurfaceIndex 
//!             object containing a unique index value the surface is mapped to 
//!             when created by the CmDevice.The CmDevice keeps the mapping b/w 
//!             index and CmBufferUP. The SurfaceIndex is passed to CM kernel 
//!             function(genx_main) as argument to indicate the surface. It is 
//!             application's responsibility to make sure the accesses from CPU 
//!             and GPU are not overlapped. 
//!
class CmBufferUP
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated 
    //!             with this CmBufferUp object. 
    //! \param      [out] index
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &index) = 0;

    //!
    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this buffer.
    //! \param      [in] memCtrl
    //!             The selected pre-defined memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is only supported for Gen9 and plus platform.
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl) = 0;

    //!
    //! \brief      Selects one of the pre-defined mos resource usage
    //!             this cm_buffer
    //! \note       This function works on Gen9+ paltforms.
    //! \param      [in] mosUsage
    //!             The selected pre-defined MOS resource usage for memory object control setting.
    //! \retval     CM_SUCCESS if the given parameter is valid
    //! \retval     CM_FAILURE otherwise.
    //!
    CMRT_UMD_API virtual int32_t
    SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage) = 0;

};

//!
//! \brief      CmBuffer Class to manage 1D surface in video memory.
//! \details    CmBuffer represents a 1D surface in video memory. Each CmBuffer
//!             object is associated with a SurfaceIndex object containing a
//!             unique index value the surface is mapped to when created by the
//!             CmDevice. The CmDevice keeps the mapping b/w index and CmBuffer.
//!             The SurfaceIndex is passed to CM kernel function (genx_main) as
//!             argument to indicate the surface.
//!
class CmBuffer
{
public:
    //!
    //! \brief      This function returns the SurfaceIndex object associated 
    //!             with this CmBuffer object. 
    //! \param      [out] index
    //!             Reference to the pointer to SurfaceIndex.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &index) = 0;

    //!
    //! \brief      Copies data in this buffer to system memory using CPU.
    //! \details    The size of data copied is the size of data in buffer.This
    //!             API is a blocking function, i.e. the function will not
    //!             return until the copy operation is completed. Buffer
    //!             reading will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. It is application's
    //!             responsibility to make sure no other task enqueued after
    //!             the task corresponding to the dependent task but before
    //!             ReadSurface. If sysMemSize is given, it will be checked 
    //!             against the size needed for the buffer. If the sysMemSize
    //!             is less than the CmBuffer size, copy only happens for sysMemSize
    //!             bytes, not all data in CmBuffer.
    //! \param      [out] sysMem
    //!             Pointer to the system memory receiving buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurface(unsigned char* sysMem,
                CmEvent* event,
                uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Copies system memory content to this buffer usingg CPU.
    //! \details    The size of data copied is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the copy operation is completed. Buffer
    //!             writing will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. The dependent event for
    //!             WriteSurface is usually NULL. If sysMemSize is given, it
    //!             will be checked against the size needed for the buffer.
    //!             If the sysMemSize is less than the CmBuffer size, copy only
    //!             happens for sysMemSize bytes, not all data in CmBuffer.
    //! \param      [in] sysMem
    //!             Pointer to the system memory storing the buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem pointer is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurface(const unsigned char* sysMem,
                 CmEvent* event,
                 uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Sets memory in this buffer to uint32_t value using CPU.
    //! \details    The size of data initialized is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the set operation is completed. Buffer
    //!             initialization will not happen until the status of the
    //!             dependent event becomes CM_STATUS_FINISHED. The dependent
    //!             event for InitSurface is usually NULL.
    //! \param      [in] initValue
    //!             uint32_t value used to initialize to.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t InitSurface(const uint32_t initValue,
                                          CmEvent* event) = 0;

    //!
    //! \brief      Selects one of the pre-defined memory object control
    //!             settings for this buffer.
    //! \param      [in] memCtrl
    //!             The selected pre-defined memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is only supported for Gen9 and plus platform.
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL memCtrl) = 0;

    //!
    //! \brief      Sets the surface state of this buffer.
    //! \details    Set the new size, offset, and mocs to the surface index, 
    //!             so they will take effect during surface state setting. 
    //!             Usually, the surface index is an alias surface index created 
    //!             by calling CmDevice::CreateBufferAlias. This function can be used to 
    //!             reinterpret the size, offset, and mocs of an existing CmBuffer.
    //! \param      [in] surfIndex
    //!             Pointer to surface index of this buffer.
    //! \param      [in] bufferStateParam
    //!             The surface state parameter of this buffer. It contains
    //!             size, base address offset and memory object control setting. The
    //!             offset must be 16-aligned due to hardware requirement.
    //! \retval     CM_SUCCESS if the surface state is set successfully.
    //! \retval     CM_INVALID_ARG_VALUE if the surface state parameter is 
    //!             invalid.
    //! \retval     CM_NULL_POINTER if any internal used pointer is nullptr.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    SetSurfaceStateParam(SurfaceIndex *surfIndex,
                         const CM_BUFFER_STATE_PARAM *bufferStateParam) = 0;

    //!
    //! \brief      Selects one of the pre-defined mos resource usage
    //!             settings for this buffer.
    //! \note       This function works on Gen9+ paltforms.
    //! \param      [in] mosUsage
    //!             The selected pre-defined MOS resource usage for memory object control setting.
    //! \retval     CM_SUCCESS if the memory object control is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //! \note       This API is only supported for Gen9 and plus platform.
    //!
    CMRT_UMD_API virtual int32_t
    SetResourceUsage(const MOS_HW_RESOURCE_DEF mosUsage) = 0;
};

//! \brief      CmBufferStateless Class to manage 1D surface created from vedio or
//!             system memory.
//! \details    CmBufferStateless represents a 1D surface in vedio memory or system
//!             space that is stateless-accessed by GPU. The stateless buffer can be
//!             created from vedio memory, which can be only accessed by GPU. It also
//!             can be created from system memory, which is local shared memor
//!             between GPU and CPU and can be access by both GPU and CPU. GPU can
//!             access the memory by passing the graphics address to CM kernel
//!             function(genx_main) as an argument for kernel to use. CPU can access the
//!             system address dirtectly.
class CmBufferStateless
{
public:
    //!
    //! \brief      Get the staring address in graphics memory space.
    //! \param      [out] gfxAddr
    //!             Staring address in graphics memory space.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetGfxAddress(uint64_t &gfxAddr) = 0;

    //!
    //! \brief      Get the starting address in system memory space.
    //! \param      [out] sysAddr
    //!             Starting address in system memory space.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetSysAddress(void *&sysAddr) = 0;

    //!
    //! \brief      Copies data in stateless buffer to system memory using CPU.
    //! \details    The size of data copied is the size of data in buffer.This
    //!             API is a blocking function, i.e. the function will not
    //!             return until the copy operation is completed. Buffer
    //!             reading will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. It is application's
    //!             responsibility to make sure no other task enqueued after
    //!             the task corresponding to the dependent task but before
    //!             ReadSurface. If sysMemSize is given, it will be checked
    //!             against the size needed for the buffer. If the sysMemSize
    //!             is less than the CmBufferStateless size, copy only happens
    //!             for sysMemSize bytes, not all data in CmBufferStateless.
    //! \param      [out] sysMem
    //!             Pointer to the system memory receiving buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
        ReadSurface(unsigned char *sysMem,
                    CmEvent *event,
                    uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief      Copies system memory content to stateless buffer usingg CPU.
    //! \details    The size of data copied is the size of data in the
    //!             buffer. This is a blocking function, i.e. the function will
    //!             not return until the copy operation is completed. Buffer
    //!             writing will not happen until the status of the dependent
    //!             event becomes CM_STATUS_FINISHED. The dependent event for
    //!             WriteSurface is usually NULL. If sysMemSize is given, it
    //!             will be checked against the size needed for the buffer.
    //!             If the sysMemSize is less than the CmBufferStateless size,
    //!             copy onlyhappens for sysMemSize bytes, not all data in
    //!             CmBufferStateless.
    //! \param      [in] sysMem
    //!             Pointer to the system memory storing the buffer data.
    //! \param      [in] event
    //!             Pointer to the dependent event used for sychronization.
    //! \param      [in] sysMemSize
    //!             Size of the system memory in byte.
    //! \retval     CM_SUCCESS if copy is successful.
    //! \retval     CM_INVALID_ARG_VALUE if sysMem pointer is nullptr.
    //! \retval     CM_LOCK_SURFACE_FAIL if surface locking is failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
        WriteSurface(const unsigned char *sysMem,
                     CmEvent *event,
                     uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

};

}//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMBUFFER_H_
