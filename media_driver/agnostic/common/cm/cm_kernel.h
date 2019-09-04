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
//! \file      cm_kernel.h
//! \brief     Contains Class CmKernel definitions
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNEL_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNEL_H_

#include "cm_def.h"

namespace CMRT_UMD
{
class CmThreadSpace;
class CmThreadGroupSpace;

//! \details    Before enqueuing a CmKernel, it has to be set up.To set up a
//!             kernel the application needs to specify the number of kernel
//!             threads to be created, the values of all arguments to the
//!             kernel, and optionally the dependency mask specifying the
//!             dependency among threads. Arguments of a kernel may be
//!             specified as per kernel arguments or per thread arguments.
//!             Per kernel arguments are arguments that have the same value
//!             for all threads of a kernel, whereas per thread arguments may
//!             have a different value for different threads. After a CmKernel
//!             is set up, the kernel may be Enqueued for execution
//!             once or multiple times. Kernel settings are preserved across
//!             multiple Enqueue calls. After enqueuing a CmKernel the
//!             application may explicitly calling CmKernel member functions
//!             to modify the setup subject to implementation restrictions
//!             described below.\n
//!             Implementation Restrictions:\n
//!             The thread count cannot be changed once it has been set. Also
//!             an argument that has been previously set using SetKernelArg
//!             cannot be subsequently set using SetThreadArg, and vice versa.
//!             These are current restrictions imposed by the API
//!             implementation and may be removed in a future release.
class CmKernel
{
public:
    //! \brief      Set the number of threads for this kernel.
    //! \details    This function specifies the number of threads to create for
    //!             execution of the kernel. For media object, the number of
    //!             threads of all kernels in a task should be no more than
    //!             CAP_USER_DEFINED_THREAD_COUNT_PER_TASK. For media walker,
    //!             the number of threads of the media walker kernel should be
    //!             no more than 261121(511x511) pre-SKL and 4190209(2047x2047)
    //!             SKL+. This function is not necessary if a thread space is
    //!             defined. The thread count set by calling this funciton will
    //!             be overwritten by the thread space dimension.
    //! \param      [in] count
    //!             number of threads.
    //! \retval     CM_SUCCESS if thread number is set successfully.
    //! \retval     CM_INVALID_ARG_VALUE if the thread count exceeds the maximum.
    //! \note       If this function is called more than once with different
    //!             count value, all argument values become invalidated, i.e.
    //!             application needs to call SetKernelArg or SetThreadArg again
    //!             for all arguments.
    CM_RT_API virtual int32_t SetThreadCount(uint32_t count) = 0;

    //! \brief      Set a per-kernel argument.
    //! \details    The total size in bytes of all
    //!             per kernel arguments and per thread arguments should be
    //!             less than or equal to CAP_ARG_SIZE_PER_KERNEL.
    //!             Per kernel arguments are set by calling SetKernelArg.
    //!             Per thread arguments are set by calling SetThreadArg.
    //!             Calling SetThreadArg for a kernel triggers media object
    //!             command. Otherwise media object walker command is used.
    //! \param      [in] index
    //!             Index of argument in MDF kernel function. The index is
    //!             global for per kernel arguments and per thread arguments.
    //! \param      [in] size
    //!             Size of the argument.
    //! \param      [in] value
    //!             Pointer to argument value, could be CM_NULL_SURFACE if the
    //!             arg is not used in kernel.
    //! \retval     CM_SUCCESS if the per-kernel argument is set successfully.
    //! \retval     CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL if the indirect data set
    //! \retval     CM_INVALID_ARG_INDEX if the argument 'index' is incorrect
    //! \retval     CM_INVALID_ARG_VALUE if the argument 'value' is incorrect
    //! \retval     CM_INVALID_ARG_SIZE if the argument 'size' is incorrect
    //! \retval     CM_FAILURE otherwise
    CM_RT_API virtual int32_t SetKernelArg(uint32_t index,
                                           size_t size,
                                           const void *value) = 0;

    //! \brief      Set a per thread argument.
    //! \details    The total size in bytes of all
    //!             per kernel arguments and per thread arguments should be
    //!             less than or equal to CAP_ARG_SIZE_PER_KERNEL.
    //!             Per kernel arguments are set by calling SetKernelArg.
    //!             Per thread arguments are set by calling SetThreadArg.
    //!             Calling SetThreadArg for a kernel triggers media object
    //!             command. Otherwise media object walker command is used.
    //! \param      [in] threadId
    //!             Index of the thread.
    //! \param      [in] index
    //!             Index of argument in CM kernel function. The index is
    //!             global for per kernel arguments and per thread arguments.
    //! \param      [in] size
    //!             Size of the argument.
    //! \param      [in] value
    //!             Pointer to argument. Setting a value more than once for the
    //!             same threadId and index is allowed, but the sizes must be
    //!             the same.
    //! \retval     CM_SUCCESS if the per-thread argument is set successfully.
    //! \retval     CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL if the indirect data set
    //! \retval     CM_INVALID_THREAD_INDEX if the argument 'threadId' is incorrect
    //! \retval     CM_INVALID_ARG_INDEX if the argument 'index' is incorrect
    //! \retval     CM_INVALID_ARG_VALUE if the argument 'value' is incorrect
    //! \retval     CM_INVALID_ARG_SIZE if the argument 'size' is incorrect
    //! \retval     CM_FAILURE otherwise
    //! \note       This API is not recommended to be used. Using threadspace to indicate
    //!             thread indexes is a better choice.
    CM_RT_API virtual int32_t SetThreadArg(uint32_t threadId,
                                           uint32_t index,
                                           size_t size,
                                           const void *value) = 0;

    //! \brief      Set a buffer to be a static buffer.
    //! \details    value points to the buffer's surface index. In CM host
    //!             program, currently we can have at most 4 static buffers
    //!             which are indexed as 0~3. These static buffers can be
    //!             accessed by all functions in the kernel. For SKL and newer
    //!             and newer platforms, these static buffers have surface
    //!             binding table index 1~4. For platforms older than SKL,
    //!             these static buffers are binded at index 0xf3~0xf6.
    //! \param      [in] index
    //!             Index of the global buffer, valid in 0~3.
    //! \param      [in] value
    //!             Pointer to the CmBuffer's SurfaceIndex.
    //! \retval     CM_SUCCESS is the static buffer is set successfully.
    //! \retval     CM_INVALID_GLOBAL_BUFFER_INDEX if the index is not in 0~3
    //! \retval     CM_INVALID_BUFFER_HANDLER if value is invalid
    //! \retval     CM_INVALID_ARG_INDEX if the surfaceIndex pointed by value is invalid
    //! \retval     CM_FAILURE otherwise
    //! \note       The print buffer occupies static buffer index one. If
    //!             CmDevice::InitPrintBuffer is called, static buffer 1 can't be used.
    CM_RT_API virtual int32_t SetStaticBuffer(uint32_t index,
                                              const void *value) = 0;

    //! \brief      Set the binding table index directly for a surface.
    //! \details    The assigned binding table index should be a valid value
    //!             for buffer, surface 2D, surface2D UP, sampler surface ,
    //!             or sampler 8x8 surface, otherwise, this call will return failure.
    //!             By calling this fucntion, the surfce can be accessed by
    //!             kernel using the btIndex directly. No need to pass surface
    //!             from host to kernel via kernel argument.
    //! \param      [in] surface
    //!             The surface whose binding table index will be set.
    //! \param      [in] bti
    //!             The binding table index. 1~242 on Gen8 and 8~239 on Gen9+ platforms.
    //! \retval     CM_SUCCESS if the binding table index is set successfully.
    //! \retval     CM_KERNELPAYLOAD_SURFACE_INVALID_BTIINDEX if btIndex is invalid
    //! \retval     CM_FAILURE otherwise
    CM_RT_API virtual int32_t SetSurfaceBTI(SurfaceIndex* surface,
                                            uint32_t bti) = 0;

    //! \brief      Associate a thread space to this kernel.
    //! \details    This is per kernel thread space.
    //! \param      [in] threadSpace
    //!             the pointer to the CmThreadSpace.
    //! \retval     CM_SUCCESS if the association is successful.
    //! \retval     CM_INVALID_ARG_VALUE if threadSpace is invalid
    //! \retval     CM_INVALID_KERNEL_THREADSPACE if thread group space is set
    //! \note       It is exclusive with AssociateThreadGroupSpace().
    CM_RT_API virtual int32_t AssociateThreadSpace(CmThreadSpace* &threadSpace) = 0;

    //! \brief      Associates a thread group space with this kernel.
    //! \details    This is per kernel thread group space. Each kernel will
    //!             tri gger a gpgpu walker command.
    //! \param      [in] threadGroupSpace
    //!             A pointer ot the CmThreadGroupSpace.
    //! \retval     CM_SUCCESS if the association is successful.
    //! \retval     CM_INVALID_ARG_VALUE if threadSpace is invalid
    //! \retval     CM_INVALID_KERNEL_THREADSPACE if thread space is set
    //! \note       It is exclusive with AssociateThreadSpace().
    CM_RT_API virtual int32_t
    AssociateThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace) = 0;

    //! \brief      Set sampler heap position by user.
    //! \details    Unlike surface state, each type of sampler state occupies
    //!             different size of space in the sampler heap. The offset in
    //!             the heap is the BTI index times the size of the sampler.
    //! \param      [in] sampler
    //!             The SamplerIndex whose binding table index will be set.
    //! \param      [in] nIndex
    //!             The binding table index.
    //! \retval     CM_SUCCESS if the setting is successful.
    //! \retval     CM_NULL_POINTER if sampler is nullptr
    //! \retval     CM_KERNELPAYLOAD_SAMPLER_INVALID_BTINDEX if nIndex is invalid
    //! \retval     CM_FAILURE otherwise
    CM_RT_API virtual int32_t
    SetSamplerBTI(SamplerIndex *sampler, uint32_t nIndex) = 0;

    //! \brief      De-associate the thread space from the kernel.
    //! \details    Coupled with AssociateThreadSpace().
    //! \param      [in] threadSpace
    //!             The pointer to CmThreadSpace.
    //! \retval     CM_SUCCESS if the de-associate operation is successful.
    //! \retval     CM_NULL_POINTER if threadSpace is nullptr
    //! \retval     CM_INVALID_ARG_VALUE if threadSpace has not been set before
    CM_RT_API virtual int32_t DeAssociateThreadSpace(CmThreadSpace* &threadSpace) = 0;

    //! \brief      De-associate the thread group space from the kernel.
    //! \details    Coupled with AssociateThreadGroupSpace().
    //! \param      [in] threadGroupSpace
    //!             the pointer to CmThreadGroupSpace.
    //! \retval     CM_SUCCESS if the de-associate operation is successful.
    //! \retval     CM_NULL_POINTER if threadGroupSpace is nullptr
    //! \retval     CM_INVALID_ARG_VALUE if threadGroupSpace has not been set before
    CM_RT_API virtual int32_t
    DeAssociateThreadGroupSpace(CmThreadGroupSpace* &threadGroupSpace) = 0;

    //! \brief      Query the kernel spill memory size.
    //! \details    During Just-In-Time compilation of kernel, if compiler
    //!             detects that more registers are needed than allowed,
    //!             spill happens. This function is to return the spill size.
    //!             This function will return failure if JIT compilation
    //!             doesn't happen.
    //! \param      [out] spillMemorySize
    //!             The spill memory size in bytes.
    //! \retval     CM_SUCCESS if the query is successful.
    //! \retval     CM_FAILURE otherwise.
    CM_RT_API virtual int32_t QuerySpillSize(uint32_t &spillMemorySize) = 0;

    //! \brief      Set SVM or stateless buffer pointer as per-kernel argument.
    //! \details    The total size in bytes of all
    //!             per kernel arguments and per thread arguments should be
    //!             less than or equal to CAP_ARG_SIZE_PER_KERNEL.
    //!             Per kernel arguments are set by calling SetKernelArg.
    //!             Per thread arguments are set by calling SetThreadArg.
    //!             Calling SetThreadArg for a kernel triggers media object
    //!             command. Otherwise media object walker command is used.
    //! \param      [in] index
    //!             Index of argument in MDF kernel function. The index is
    //!             global for per kernel arguments and per thread arguments.
    //! \param      [in] size
    //!             The size of kernel argument.
    //! \param      [in] value
    //!             The SVM or stateless buffer pointer that should be used as
    //!             the argument value for argument specified by index.The
    //!             SVM buffer pointer value specified as the argument
    //!             value can be the pointer returned by CreateBufferSVM(). And
    //!             the stateless buffer pointer value specified as the argument
    //!             value can be the pointer returned by CmBufferStateless::
    //!             GetGfxAddress() or CmBufferStateless::GetSysAddress().
    //!             Or can be a pointer + offset into the SVM and stateless
    //!             buffer region.
    //! \retval     CM_SUCCESS if the per-kernel argument is set successfully.
    //! \retval     CM_INVALID_ARG_INDEX if the argument 'index' is incorrect
    //! \retval     CM_INVALID_KERNEL_ARG_POINTER if the argument 'value' is incorrect
    //! \retval     CM_KERNELPAYLOAD_PERKERNELARG_MUTEX_FAIL if the indirect data set
    //! \retval     CM_FAILURE otherwise
    CM_RT_API virtual int32_t SetKernelArgPointer(uint32_t index,
                                                  size_t size,
                                                  const void *value) = 0;

public:
    //! \brief      Get the kernel binary of this kernel.
    //! \param      [in,out] binary
    //!             Vector to store kernel binary.
    //! \returns    CM_SUCCESS.
    //! \note       This API is implemented for debug purpose.
    //!
    CMRT_UMD_API virtual int32_t GetBinary(std::vector<char> &binary) = 0;

    //! \brief      Replace the kernel binary of this kernel.
    //! \param      [in] binary
    //!             Vector to store kernel binary.
    //! \retval     CM_SUCCESS if the kernel binary is replaced successfully.
    //! \retval     CM_INVALID_ARG_VALUE if input argument is invalid.
    //! \note       This API is implemented for debug purpose.
    //!
    CMRT_UMD_API virtual int32_t ReplaceBinary(std::vector<char> &binary) = 0;

    //! \brief      Reset the kernel binary of this kernel.
    //! \returns    CM_SUCCESS.
    //! \note       This API is implemented for debug purpose.
    //!
    CMRT_UMD_API virtual int32_t ResetBinary() = 0;
};
};//namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMKERNEL_H
