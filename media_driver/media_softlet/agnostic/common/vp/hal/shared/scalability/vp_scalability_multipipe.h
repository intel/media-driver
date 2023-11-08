/*
* Copyright (c) 2020-2023, Intel Corporation
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
//! \file     vp_scalability_multipipe.h
//! \brief    Defines the common interface for vp scalability mulitpipe mode.
//! \details  The vp scalability mulitpipe interface is further sub-divided by codecs,
//!           this file is for the base interface which is shared by all codecs.
//!

#ifndef __VP_SCALABILITY_MULTIPIPE_H__
#define __VP_SCALABILITY_MULTIPIPE_H__
#include "vp_scalability_multipipe_next.h"

namespace vp
{
class VpScalabilityMultiPipe : public VpScalabilityMultiPipeNext
{
public:
    //!
    //! \brief  Vp scalability mulitipipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to MediaContext
    //! \param  [in] componentType
    //!         Component type
    //!
    VpScalabilityMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Vp scalability mulitipipe destructor
    //!
    ~VpScalabilityMultiPipe();

    //!
    //! \brief    Copy constructor
    //!
    VpScalabilityMultiPipe(const VpScalabilityMultiPipe &) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    VpScalabilityMultiPipe &operator=(const VpScalabilityMultiPipe &) = delete;

    //!
    //! \brief  Sync pipes with first pipe wait for others
    //! \detials Only support to use this sync once per frame.
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \param  [in] pipeIdx
    //!         The index of pipeline which wait for others
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    //virtual MOS_STATUS SyncOnePipeWaitOthers(PMOS_COMMAND_BUFFER cmdBuffer, uint32_t pipeIdx);

    //!
    //! \brief    Send hw semphore wait cmd
    //! \details  Send hw semphore wait cmd for sync perpose
    //!
    //! \param    [in] semaMem
    //!           Reource of Hw semphore
    //! \param    [in] offset
    //!           offset of semMem
    //! \param    [in] semaData
    //!           Data of Hw semphore
    //! \param    [in] opCode
    //!           Operation code
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendHwSemaphoreWaitCmd(
        PMOS_RESOURCE                             semaMem,
        uint32_t                                  offset,
        uint32_t                                  semaData,
        MHW_COMMON_MI_SEMAPHORE_COMPARE_OPERATION opCode,
        PMOS_COMMAND_BUFFER                       cmdBuffer) override;

    //!
    //! \brief    Send mi atomic dword cmd
    //! \details  Send mi atomic dword cmd for sync perpose
    //!
    //! \param    [in] resource
    //!           Reource used in mi atomic dword cmd
    //! \param    [in] offset
    //!           offset of resource
    //! \param    [in] immData
    //!           Immediate data
    //! \param    [in] opCode
    //!           Operation code
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SendMiAtomicDwordCmd(
        PMOS_RESOURCE               resource,
        uint32_t                    offset,
        uint32_t                    immData,
        MHW_COMMON_MI_ATOMIC_OPCODE opCode,
        PMOS_COMMAND_BUFFER         cmdBuffer) override;

    //!
    //! \brief    Send mi flush dword cmd
    //! \details  Send mi flush dword cmd for sync perpose
    //!
    //! \param    [in] semMem
    //!           Reource used in mi flush dword cmd
    //! \param    [in] semaData
    //!           Immediate data
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiFlushDwCmd(
        PMOS_RESOURCE                             semaMem,
        uint32_t                                  semaData,
        PMOS_COMMAND_BUFFER                       cmdBuffer) override;

    //!
    //! \brief    Send mi store data dword cmd
    //! \details  Send mi store dat dword cmd for sync perpose
    //!
    //! \param    [in] resource
    //!           Reource used in mi store dat dword cmd
    //! \param    [in] offset
    //!           offset of resource
    //! \param    [in,out] cmdBuffer
    //!           command buffer
    //!
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMiStoreDataImmCmd(
        PMOS_RESOURCE               resource,
        uint32_t                    offset,
        PMOS_COMMAND_BUFFER         cmdBuffer) override;

    //!
    //! \brief   Initialize the media scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

    //!
    //! \brief  Get command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS GetCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer, bool frameTrackingRequested = true) override;

    //!
    //! \brief  Set hint parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHintParams() override;

    //!
    //! \brief  Destroy the media scalability
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

    //!
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    static MOS_STATUS CreateMultiPipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    VpPhase                               *m_phase = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpScalabilityMultiPipe)
};
}  // namespace vp
#endif  // __VP_SCALABILITY_MULTIPIPE_H__
