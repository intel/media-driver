/*
* Copyright (c) 2007-2017, Intel Corporation
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
//! \file      cm_surface_2d.h
//! \brief     Contains CmSurface2D declaration.
//!
#ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2D_H_
#define MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2D_H_

#include "cm_def.h"

namespace CMRT_UMD
{
class CmEvent;

class CmSurface2D
{
public:
    //!
    //! \brief Retrieves surface index of this CmSurface2D.
    //! \param [out] pIndex
    //!        Reference to the pointer to an SurfaceIndex.
    //!        It will point to the internal SurfaceIndex.
    //! \retval CM_SUECCESS.
    //!
    CM_RT_API virtual int32_t GetIndex(SurfaceIndex* &pIndex) = 0;

    //!
    //! \brief Copies data in this CmSurface2D to system memory.
    //! \details Copied data size is the same as surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          It's the application's responsibility to make sure no other
    //!          task enqueued between the task corresponding to the event and
    //!          this fuction call.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [out] pSysMem
    //!        Pointer to the system memory receiving surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurface(unsigned char *pSysMem,
                CmEvent *pEvent,
                uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Copies data in system memory to this CmSurface2D.
    //! \details Copied data size is the same as the surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [in] pSysMem
    //!        Pointer to the system memory storing surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurface(const unsigned char *pSysMem,
                 CmEvent *pEvent,
                 uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Copies data in this CmSurface2D to system memory with system
    //!        memory stride.
    //! \details Copied data size is the same as surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          It's the application's responsibility to make sure no other
    //!          task enqueued between the task corresponding to the event and
    //!          this fuction call.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [out] pSysMem
    //!        Pointer to the system memory receiving surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] stride
    //!        System memory stride in bytes.
    //!        It equals actual surface width in bytes plus extra padding bytes.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurfaceStride(unsigned char *pSysMem,
                      CmEvent *pEvent,
                      const unsigned int stride,
                      uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Copies data in system memory to this CmSurface2D with system
    //!        memory stride.
    //! \details Copied data size is the same as the surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [in] pSysMem
    //!        Pointer to the system memory storing surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] stride
    //!        System memory stride in bytes.
    //!        It equals actual surface width in bytes plus extra padding bytes.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurfaceStride(const unsigned char *pSysMem,
                       CmEvent *pEvent,
                       const unsigned int stride,
                       uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL) = 0;

    //!
    //! \brief Sets surface data to a unified value.
    //! \details This is a blocking function, i.e. the function will not return
    //!          until the operation is completed.
    //!          Initialization will not happen until the status of the
    //!          dependent event becomes CM_STATUS_FINISHED.
    //! \param [in] initValue
    //!        The value for initialization.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \retval CM_SUCCESS if initialization is successful.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t InitSurface(const unsigned int initValue,
                                          CmEvent *pEvent) = 0;

    //!
    //! \brief Retrieves libva surface ID.
    //! \note This function is a Linux-only API.
    //! \param [out] iVASurface
    //!        Reference to a VASurfaceID receiving libva surface ID.
    //! \retval CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t GetVaSurfaceID(VASurfaceID &iVASurface) = 0;

    //!
    //! \brief Hybrid memory copy from this CmSurface2D to system memory with system
    //!        memory strides.
    //! \details Copied data size is the same as surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [out] pSysMem
    //!        Pointer to the system memory receiving surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] iWidthStride
    //!        Horizontal stride of system memory in bytes.
    //! \param [in] iHeightStride
    //!        Vertical stride of system memory in rows.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \param [in] uiOption
    //!        Option to disable/enable hybrid memory copy.
    //! \retval CM_SUCCESS if this function succeeds.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    ReadSurfaceHybridStrides(unsigned char *pSysMem,
                             CmEvent *pEvent,
                             const unsigned int iWidthStride,
                             const unsigned int iHeightStride,
                             uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                             unsigned int uiOption = 0) = 0;

    //!
    //! \brief Hybrid memory copy from system memory to this CmSurface2D with
    //!        system memory stride.
    //! \details Copied data size is the same as the surface data size.
    //!          This is a blocking function, i.e. the function will not return
    //!          until the copy
    //!          operation is completed.
    //!          Copying will not happen until the status of the dependent event
    //!          becomes CM_STATUS_FINISHED.
    //!          If sysMemSize is given, it will be checked against the size of
    //!          the surface data.
    //! \param [in] pSysMem
    //!        Pointer to the system memory storing surface data.
    //! \param [in] pEvent
    //!        Pointer to the dependent event used for sychronization.
    //! \param [in] iWidthStride
    //!        Horizontal stride of system memory in bytes.
    //! \param [in] iHeightStride
    //!        Vertical stride of system memory in rows.
    //! \param [in] sysMemSize
    //!        Size of the system memory.
    //! \param [in] uiOption
    //!        Option to disable/enable hybrid memory copy.
    //! \retval CM_SUCCESS if copy is successful.
    //! \retval CM_INVALID_ARG_VALUE if sysMemSize is given but less than what
    //!         is needed.
    //! \retval CM_LOCK_SURFACE_FAIL if surface locking fails.
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    WriteSurfaceHybridStrides(const unsigned char *pSysMem,
                              CmEvent *pEvent,
                              const unsigned int iWidthStride,
                              const unsigned int iHeightStride,
                              uint64_t sysMemSize = 0xFFFFFFFFFFFFFFFFULL,
                              unsigned int uiOption = 0) = 0;

    //!
    //! \brief Selects one of the pre-defined memory object control settings for
    //!        this CmSurface2D.
    //! \note This function only works on Gen9+ paltforms.
    //! \param [in] mem_ctrl
    //!        The selected pre-defined memory object control setting.
    //! \retval CM_SUCCESS if the given parameter is valid
    //! \retval CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    SelectMemoryObjectControlSetting(MEMORY_OBJECT_CONTROL mem_ctrl) = 0;

    //!
    //! \brief Sets frame type of this CmSurface2D.
    //! \details By default this CmSurface2D is a whole frame.
    //! \param [in] frameType
    //!        A value in enumeration CM_FRAME_TYPE, frame type of this
    //!        CmSurface2D. It should be a whole frame or a field in an
    //!        interlaced frame.
    //! \retval CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t SetProperty(CM_FRAME_TYPE frameType) = 0;

    //!
    //! \brief Sets surface state parameters for an alias of this CmSurface2D.
    //! \details If pSurfIndex is nullptr, default state of this CmSurface2D
    //!          is changed.
    //! \param [in] pSurfIndex
    //!        Pointer to the surface index of an alias of this CmSurface2D. A new
    //!        surface state is created for this alias or the existing state is updated.
    //! \param [in] pSSParam
    //!        Pointer to a new state parameter.
    //! \retval CM_INVALID_ARG_VALUE if any parameter is invalid.
    //! \retval CM_SUCCESS if successful.
    //!
    CM_RT_API virtual int32_t
    SetSurfaceStateParam(SurfaceIndex *pSurfIndex,
                         const CM_SURFACE2D_STATE_PARAM *pSSParam) = 0;

// Pay Attention: below APIs only used in UMD. If you add an API exposed to application, please add it BEFORE this line.
public:
    //!
    //! \brief Sets the surface's read sync flag for synchronization between engines.
    //! \details If the surface is shared between render engine and another engine, 
    //!          the read sync flag is to tell whether the next engine should wait till
    //!          the kernel execution ends in render engine. If the read sync flag is set,
    //!          then it means the render engine only read this surface and the next engine 
    //!          can also access it simultaneously. If the read sync flag
    //!          is not set (or set to false), then the next engine should assume the 
    //!          render engine is writing to this surface and wait till the kernel execution
    //!          ends.
    //! \param [in] bReadSync
    //!        value of read sync flag to be set to the surface
    //! \retval CM_INVALID_ARG_VALUE if any parameter is invalid.
    //! \retval CM_SUCCESS if successful.
    //!
    CMRT_UMD_API virtual int32_t SetReadSyncFlag(bool bReadSync) = 0;

    //!
    //! \brief Set the UMD Resource and MOS Resource in the CmSurface2D
    //! \details A callback function which allows CM callers to change the UMD Resource and 
    //!          MOS Resource embedded in the CmSurface2D.
    //! \param [in] umdResource
    //!        the UMD Resource set to the CmSurface2D
    //! \param [in] updateMosResource
    //!        a flag indicating whether MOS resource needs updating. 0 mean keeping it
    //!        unchanged. Otherwise, set the MOS resource to parameter pMosResource. Default
    //!        is 0.
    //! \param [in] pMosResource
    //!        the MOS Resource set to the CmSurface2D
    //! \retval CM_SUCCESS always.
    //!
    CMRT_UMD_API virtual int32_t
    NotifyUmdResourceChanged(UMD_RESOURCE umdResource,
                             int updateMosResource = 0,
                             PMOS_RESOURCE pMosResource = nullptr) = 0;
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_LINUX_COMMON_CM_CMSURFACE2D_H_
