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
//! \file      cm_thread_space.h
//! \brief     Contains CmThreadSpace declarations.
//!

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACE_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACE_H_

#include "cm_def.h"

struct CM_DEPENDENCY
{
    uint32_t count;
    int32_t deltaX[CM_MAX_DEPENDENCY_COUNT];
    int32_t deltaY[CM_MAX_DEPENDENCY_COUNT];
};

struct CM_COORDINATE
{
    int32_t x;
    int32_t y;
};

struct CM_THREAD_PARAM
{
    CM_COORDINATE scoreboardCoordinates;     //[X, Y] terms of the scoreboard values of the current thread.
    uint8_t scoreboardColor;           // dependency color the current thread.
    uint8_t sliceDestinationSelect;    //select determines the slice of the current thread must be sent to.
    uint8_t subSliceDestinationSelect;    //select determines the sub-slice of the current thread must be sent to.
};

//| CM different dispatch patterns for 26ZI media object
enum CM_26ZI_DISPATCH_PATTERN
{
    VVERTICAL_HVERTICAL_26        = 0,
    VVERTICAL_HHORIZONTAL_26      = 1,
    VVERTICAL26_HHORIZONTAL26     = 2,
    VVERTICAL1X26_HHORIZONTAL1X26 = 3
};

namespace CMRT_UMD
{
class CmKernel;

//!
//! \brief      CmThreadSpace Class.
//! \details    CmThreadSpace is a 2-dimensional space.Each unit is notated as
//!             a pair of X/Y coordinates,which is in the range of [0, width -1]
//!             or [0, heigh-1]. CmThreadSpace can be per task or per kernel.
//!             If the thread space is used in CmQueue::Enqueue(CmTask * Task,
//!             CmEvent *& pEvent, const CmThreadSpace * pTS = nullptr), it is a
//!             per task thread space. In this case, each kernel in the task
//!             uses the same thread space, i.e.same dimension, same dependency
//!             etc.For per task CmThreadSpace, if  CmThreadSpace::AssociateThread
//!             or CmThreadSpace::AssociateThreadWithMask is called, the task
//!             can only has one kernel. If the thread space is used in
//!             CmKernel::AssociateThreadSpace(CmThreadSpace *& pTS), it is a
//!             per kernel thread space. Per kernel thread space overwrites
//!             per task thread space. \n
//!             CmThreadSpace::AssociateThread or CmThreadSpace::AssociateThreadWithMask
//!             are usually used together with CmKernel::SetThreadArg. If
//!             anyone of these three functions are called, media object command
//!             will be triggered. If none of CmThreadSpace::AssociateThread,
//!             CmThreadSpace::AssociateThreadWithMask, or CmKernel::SetThreadArg
//!             is called, media object walker command will be triggered. Details
//!             of media object command and media object walker command can be
//!             found at
//!             https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-skl-vol07-3d_media_gpgpu.pdf
//!             or https://01.org/sites/default/files/documentation/intel-gfx-prm-osrc-kbl-vol07-3d_media_gpgpu.pdf. \n
//!             In the case of media object, each thread gets coordinates set
//!             through CmThreadSpace::AssociateThread or
//!             CmThreadSpace::AssociateThreadWithMask. If CmThreadSpace::AssociateThread
//!             or CmThreadSpace::AssociateThreadWithMask is not called
//!             (i.e.only CmKernel::SetThreadArg is called), the coordinates
//!             will be(0, 0), (1, 0), ..., (width - 1, 0), (0, 1), (1, 1), ...,
//!             (width - 1, 1), ..., (0, height - 1), (1, height - 1), ...,
//!             (width - 1, height1) for threadId 0, 1, 2, ..threadCount - 1. \n
//!             In the case of media object walker, thread coordinates will be
//!             (0, 0), (1, 0), ..., (width - 1, 0), (0, 1), (1, 1), ...,
//!             (width - 1, 1), ..., (0, height - 1), (1, height - 1), ...,
//!             (width - 1, height1), threadId or per thread arg can be calculated
//!             using thread coordinates.
//!
class CmThreadSpace
{
public:
    //!
    //! \brief      Associate the thread to the unit with cooridinates(x, y)
    //!             in thread space.
    //! \details    Threads in a thread space must come from a single kernel.
    //!             Each unit in thread space must be associated with a thread
    //!             (can't be empty) and associated with a unique thread.A
    //!             kernel's threads should be either all in thread space or
    //!             none in thread space. This function will trigger media
    //!             object command
    //! \param      [in] x
    //!             X coordinate of the unit in thread space.
    //! \param      [in] y
    //!             Y coordinate of the unit in thread space.
    //! \param      [in] pKernel
    //!             Pointer to CmKernel.
    //! \param      [in] threadId
    //!             Thread index.
    //! \retval     CM_SUCCESS if the association is successful.
    //! \retval     CM_INVALID_ARG_VALUE if input parameters are invalid.
    //!
    CM_RT_API virtual int32_t AssociateThread(uint32_t x,
                                              uint32_t y,
                                              CmKernel *pKernel,
                                              uint32_t threadId) = 0;

    //!
    //! \brief      Select from  predefined dependency patterns.
    //! \details    There are 9 kinds of thread dependency patterns.Each
    //!             dependency pattern has specific delta in X coordinate
    //!             and the delta in Y coordinate.By default, the dependency
    //!             pattern is set CM_NONE_DEPENDENCY.
    //! \param      [in] pattern
    //!             Pattern index.CM_DEPENDENCY_PATTERN is a type-defined
    //!             enum value.
    //! \retval     CM_SUCCESS if the pattern is selected.
    //! \retval     CM_OUT_OF_HOST_MEMORY if the necessary memory allocation is
    //!             failed.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    SelectThreadDependencyPattern(CM_DEPENDENCY_PATTERN pattern) = 0;

    //!
    //! \brief      Associate the thread to the unit with cooridinates(x, y)
    //!             in thread space with dependency mask.
    //! \details    Threads in a thread space must come from a single kernel.
    //!             Each unit in thread space must be associated with a thread
    //!             (can't be empty) and associated with a unique thread. A
    //!             kernel's threads should be either all in thread space or
    //!             none in thread space.The dependency mask is used to turn
    //!             off some or all of the dependencies set by the selected
    //!             dependency pattern for that thread. This function will
    //!             trigger media object command
    //! \param      [in] x
    //!             X coordinate of the unit in thread space.
    //! \param      [in] y
    //!             Y coordinate of the unit in thread space.
    //! \param      [in] pKernel
    //!             Pointer to CmKernel.
    //! \param      [in] threadId
    //!             Thread index.
    //! \param      [in] dependencyMask
    //!             Dependency mask for the thread.
    //! \retval     CM_SUCCESS if the association is successful.
    //! \retval     CM_INVALID_ARG_VALUE if input parameters are invalid.
    //!
    CM_RT_API virtual int32_t
    AssociateThreadWithMask(uint32_t x,
                            uint32_t y,
                            CmKernel* pKernel,
                            uint32_t threadId,
                            uint8_t dependencyMask) = 0;

    //!
    //! \brief      Set the color count the CmThreadSpace.
    //! \details    When color count is specified, the total thread count is
    //!             thread_space_width x thread_space_height x color_count.
    //!             This function is for media object walker only for now.
    //!             The CM instrinsic, unsigned short get_color(void) can be
    //!             used within the kernel to get the
    //!             thread's color count value. The return
    //!             value is from 0 to predefined thread space color value - 1.
    //!             Each color count specifies one set of threads that can have
    //!             dependency among them. The depenendcy is set by calling
    //!             SelectThreadDependencyPattern. There is no dependnecy
    //!             across set( or across color count).
    //!             The walking pattern set by calling SelectMediaWalkingPattern
    //!             is applied to thread coordinates(x,y). For each
    //!             cooridnates(x,y) there is a loop for color count.
    //! \param      [in] colorCount
    //!             Color count value,valid values 1~16 for platforms till CNL.
    //! \retval     CM_SUCCESS if the color count was set successfully.
    //! \retval     CM_INVALID_ARG_VALUE if invalid color count values.
    //!
    CM_RT_API virtual int32_t SetThreadSpaceColorCount(uint32_t colorCount) = 0;

    //!
    //! \brief      Select the media walking pattern when there is no thread
    //!             space dependency.
    //! \details    API is only valid when no thread dependency has been
    //!             selected for the thread space.There are 7 kinds of media
    //!             walking patterns. By default, the media walking pattern
    //!             is set CM_WALK_DEFAULT.
    //! \param      [in] pattern
    //!             Media walking pattern index.CM_WALKING_PATTERN is a
    //!             type-defined enum value.
    //! \retval     CM_SUCCESS if media walking pattern is set successfully.
    //! \retval     CM_INVALID_MEDIA_WALKING_PATTERN if invalid walking pattern.
    //! \retval     CM_INVALID_DEPENDENCY_WITH_WALKING_PATTERN if thread
    //!             space dependency pattern does not equal NONE.
    //! \note       This API is implemented for media walker only.
    //!
    CM_RT_API virtual int32_t
    SelectMediaWalkingPattern(CM_WALKING_PATTERN pattern) = 0;

    //!
    //! \brief      Set the dispatch pattern for 26ZI dependency pattern.
    //! \details    There are 4 kinds of CM dispatch patterns for
    //!             26ZI media objects.
    //! \param      [in] pattern
    //!             Dispatch pattern index. CM_26ZI_DISPATCH_PATTERN is a
    //!             type-defined enum value.
    //! \retval     CM_SUCCESS if 26ZI dependency pattern is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t
    Set26ZIDispatchPattern(CM_26ZI_DISPATCH_PATTERN pattern) = 0;

    //!
    //! \brief      Set the macro block size for 26ZI dependency pattern
    //! \details    The width and height of the macro block are used to
    //!             generate the 26ZI dispatch order.
    //! \param      [in] width
    //!             The width of the macro block, by default 16.
    //! \param      [in] height
    //!             The height of the macro block, by default 8.
    //! \retval     CM_SUCCESS if the macro block size is set successfully.
    //! \retval     CM_FAILURE otherwise.
    //!
    CM_RT_API virtual int32_t Set26ZIMacroBlockSize(uint32_t width,
                                                    uint32_t height) = 0;

    //!
    //! \brief      Set the media walker groupIDSelect field.
    //! \details    GroupIDSelect chooses which of the nested loops of the
    //!             walker are used to identify threads which share a group id
    //!             and therefore a shared barrier and SLM. There are 6 kinds
    //              media walker group selects.By default, the groupIDSelect
    //!             is set CM_MW_GROUP_NONE.
    //! \param      [in] groupSelect
    //!             Goup ID Loop Select.CM_MW_GROUP_SELECT is a type-defined
    //!             enum value.
    //! \returns    CM_SUCCESS.
    //! \note       This API is implemented for media walker only.
    //!
    CM_RT_API virtual int32_t
    SetMediaWalkerGroupSelect(CM_MW_GROUP_SELECT groupSelect) = 0;

    //!
    //! \brief      Set the media walker parameters for the CmThreadSpace.
    //! \details    This API is used to set each field of MEDIA_OBJECT_WALKER
    //!             from an array of DWORDs.
    //! \param      [in] parameters
    //!             CM media walking parameters.CM_WALKING_PARAMETERS is a
    //!             structure which defines an array of Dword values.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t
    SelectMediaWalkingParameters(CM_WALKING_PARAMETERS parameters) = 0;

    //!
    //! \brief      Set the dependency vectors for the CmThreadSpace.
    //! \details    The depencency vector contains the vector of delta in X
    //!             coordinate, the vector of delta in Y coordinate and the array
    //!             size of vectors.The max array size is 8.
    //! \param      [in] dependVectors
    //!             CM_DEPENDENCY is a structure contains CM dependency
    //!             information.
    //! \returns    CM_SUCCESS.
    //!
    CM_RT_API virtual int32_t
    SelectThreadDependencyVectors(CM_DEPENDENCY dependVectors) = 0;

    //!
    //! \brief      Set the thread space order for the CmThreadSpace.
    //! \details    This API is used for enqueuing media-objects with explicited
    //!             scoreboard (x,y).
    //! \param      [in] threadCount
    //!             The count of total threads.
    //! \param      [in] pThreadSpaceOrder
    //!             CM_THREAD_PARAM is a structure defines the thread dispatch
    //!             order list
    //! \retval     CM_SUCCESS if thread space order is successfully set.
    //! \retval     CM_OUT_OF_HOST_MEMORY if the necessary memory allocation is
    //!             failed.
    //! \retval     CM_INVALID_ARG_VALUE if the input arg is not correct.
    //! \note       This API is implemented for media objects only.
    //!
    CM_RT_API virtual int32_t
    SetThreadSpaceOrder(uint32_t threadCount,
                        const CM_THREAD_PARAM* pThreadSpaceOrder) = 0;
};
}; //namespace

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMTHREADSPACE_H_
