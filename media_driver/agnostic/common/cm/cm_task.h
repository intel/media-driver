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
//! \file      cm_task.h
//! \brief     Contains CmTask declatation.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASK_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASK_H_

#include "cm_def.h"

enum CM_CONDITIONAL_END_OPERATOR_CODE
{
    MAD_GREATER_THAN_IDD = 0,
    MAD_GREATER_THAN_OR_EQUAL_IDD,
    MAD_LESS_THAN_IDD,
    MAD_LESS_THAN_OR_EQUAL_IDD,
    MAD_EQUAL_IDD,
    MAD_NOT_EQUAL_IDD
};

//!
//! \brief CM_CONDITIONAL_END_PARAM
//! \details
//!     The parameters of conditional batch buffer end command, for platforms
//!     till CNL, only opValue and opMask fields are used.
//!
struct CM_CONDITIONAL_END_PARAM
{
    uint32_t opValue;                           //!< operand of comparing operation
    CM_CONDITIONAL_END_OPERATOR_CODE  opCode;   //!< operation type
    bool  opMask;                               //!< mask of operand
    bool  opLevel;                              //!< batch buffer level to end
};

namespace CMRT_UMD
{
class CmKernel;

//! \brief      CmTask Class to manage task parameters.
//! \details    CmTask contains one or multiple CmKernels, optional
//!             synchronization point between two consecutive kernels,
//!             and some execution parameters. CmTask is the unit to
//!             enqueue.
//!             If there is no synchronization point. kernels will run
//!             concurrently. If there is a synchronization point, kernels
//!             after the synchronization point will not start until kernels
//!             before the synchronization point finishes.
class CmTask
{
public:
    //!
    //! \brief      Add a CmKernel pointer to CmTask.
    //! \details    Same kernel can appear in the task multiple times as long
    //!             as all the value of its arguments are the same for multiple
    //!             copies of the kernel.
    //! \param      [in] pKernel
    //!             A pointer to CmKernel object.
    //! \retval     CM_SUCCESS if pKernel is added.
    //! \retval     CM_EXCEED_MAX_KERNEL_PER_ENQUEUE trying to add more kernels
    //!             than CAP_KERNEL_COUNT_PER_TASK.
    //! \retval     CM_INVALID_ARG_VALUE if pKernel is NULL.
    //!
    CM_RT_API virtual int32_t AddKernel(CmKernel *pKernel) = 0;

    //!
    //! \brief      Resets a CmTask object
    //! \details    All contents contained in CmTask get reset. Application need
    //!             add kernel, optional synchronization points, etc. again to
    //!             the CmTask. This function is to reuse CmTask for diffrent
    //!             contents so CmTask creation/destroy overhead can be avoided.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t Reset() = 0;

    //!
    //! \brief      Inserts a synchronization point among kernels.
    //! \details    Kernels after the synchronization point will not start
    //!             execution untill kernels before the synchronization
    //!             point finishes execution. A CmTask can have multiple
    //!             synchronization points.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t AddSync() = 0;

    //!
    //! \brief      Set a per-task based power option to current task.
    //! \details    The power option includes the hardware configuration of
    //!             slice, subslice and EU number. The setting takes effect
    //!             only for current task; the value needs to be set again if
    //!             user wants it to take effect for the next task.
    //! \param      [in] pCmPowerOption
    //!             A pointer to CM_POWER_OPTION struct
    //! \retval     CM_SUCCESS.
    //! \retval     CM_EXCEED_MAX_POWER_OPTION_FOR_ PLATFORM if the any of the
    //!             settings exceeds the limit of current platform
    //!             configuration.
    //!
    CM_RT_API virtual int32_t
    SetPowerOption(PCM_POWER_OPTION pCmPowerOption) = 0;

    //!
    //! \brief      This API is used for the conditional end feature.
    //! \details    It adds a conditional batch buffer end command between two
    //!             kernels in a task. The pConditionalSurface + offset is the
    //!             address storing its data against the dword value provided in
    //!             pCondParam. If the data at the compare memory address is
    //!             greater than the dword set in pCondParam, the execution of
    //!             the command buffer will continue. If not, the remaining
    //!             kernels in the task will be skipped. The user can call this
    //!             API multiple times to insert multiple conditional ends
    //!             between different kernels. When opMask in pCondParam is 1,
    //!             the actual comparison value is the result of bitwise-and of
    //!             the value in memory and mask.
    //! \param      [in] pConditionalSurface
    //!             Pointer to the surface used to store comparison
    //!             dword by kernel.
    //! \param      [in] offset
    //!             The offset pointered by pSurface where stores comparison
    //!             dword value, and mask if opMask in pCondParam is set to 1.
    //! \param      [in] pCondParam
    //!             Pointer to the parameters of conditional batch buffer end
    //!             command, for platforms till CNL, only opValue and opMask fields are
    //!             used.
    //! \retval     CM_SUCCESS if successfully add a conditional batch buffer
    //!             end.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    AddConditionalEnd(SurfaceIndex* pConditionalSurface,
                      uint32_t offset,
                      CM_CONDITIONAL_END_PARAM *pCondParam) = 0;

    //!
    //! \brief      Expose bitfield for task related property.
    //! \details    Currently this function can be used to expose the 
    //!             bitfield for turbo boost.
    //! \param      [out] taskConfig
    //!             specify which bitfield will be exposed.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t SetProperty(const CM_TASK_CONFIG &taskConfig) = 0;
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTASK_H_
