/*
* Copyright (c) 2026, Intel Corporation
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
//! \file     encode_huc_slbb_update_pkt.h
//! \brief    Defines the HucSLBBUpdatePkt class for Second Level Batch Buffer updates using HuC
//! \details  This class provides a shared component for AV1, HEVC, and AVC codecs to update SLBB
//!           using HuC microcontroller. It inherits from both EncodeHucPPGTTPkt and EncodeHucPkt.
//!

#ifndef __HUC_SLBB_UPDATE_PKT_H__
#define __HUC_SLBB_UPDATE_PKT_H__

#include "encode_huc.h"
#include "encode_huc_ppgtt.h"
#include "mhw_vdbox_huc_ppgtt_itf.h"
#include "huc_kernel_source.h"

namespace encode
{
//!
//! \class HucSLBBUpdatePkt
//! \brief HuC packet for Second Level Batch Buffer updates
//! \details This base class provides centralized DMEM buffer management for SLBB updates.
//!          Derived classes implement codec-specific DMEM population through SetDmem().
//!
class HucSLBBUpdatePkt : public EncodeHucPkt, public EncodeHucPPGTTPkt
    {
    public:
        //!
        //! \brief  Constructor
        //!
        HucSLBBUpdatePkt(MediaPipeline *pipeline, MediaTask *task, CodechalHwInterfaceNext *hwInterface);

        //!
        //! \brief  Destructor
        //!
        virtual ~HucSLBBUpdatePkt();

        //!
        //! \brief  Initialize the HucSLBBUpdatePkt
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Execute HuC SLBB Update kernel
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \param  [in] storeHucStatus2Needed
        //!         Indicate if need to store Huc status 2 register
        //! \param  [in] prologNeeded
        //!         Indicate if prolog needed to be added
        //! \param  [in] function
        //!         HuC function
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function = NONE_BRC) override;

        //!
        //! \brief  Calculate Command Size
        //! \param  [in, out] commandBufferSize
        //!         requested size
        //! \param  [in, out] requestedPatchListSize
        //!         requested patch list size
        //! \return MOS_STATUS
        //!         status
        //!
        virtual MOS_STATUS CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize) override;

        //!
        //! \brief  Allocate Resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS AllocateResources() override;

        //!
        //! \brief  Construct batch buffer for codec-specific SLBB updates
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS ConstructBatchBuffer() = 0;

    protected:
#if _SW_BRC
        //!
        //! \brief  Initialize SW SLBB instance for SLBB update operations
        //! \param  [in] function
        //!         HuC function type
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS InitSwSLBB(HuCFunction function);
#endif  // !_SW_BRC

        //!
        //! \brief  Set HUC_IMEM_STATE command parameters
        //! \param  [in] cmdBuffer
        //!         Pointer to command buffer
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MHW_SETPAR_DECL_HDR(HUC_IMEM_STATE);

    //!
    //! \brief  Set HUC_VIRTUAL_ADDR_STATE command parameters
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_VIRTUAL_ADDR_STATE);

    //!
    //! \brief  Set HUC_DMEM_STATE command parameters
    //! \details Base implementation that calls codec-specific SetDmem() and configures DMEM state
    //! \param  [in] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MHW_SETPAR_DECL_HDR(HUC_DMEM_STATE);

    //!
    //! \brief  Set DMEM buffer with codec-specific data
    //! \details Pure virtual function to be implemented by derived classes for codec-specific DMEM population
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetDmem() const = 0;

        //!
        //! \brief  HuC interface extension for PPGTT support
        //!
        std::shared_ptr<mhw::vdbox::huc::ItfPPGTT> m_itfPPGTT = nullptr;

        //!
        //! \brief  Flag to indicate PPGTT mode
        //!
        bool m_isPPGTT = false;

        //!
        //! \brief  HuC kernel source pointer for kernel management
        //!
        HucKernelSource *m_hucKernelSource = nullptr;

    //!
    //! \brief  SLBB buffer resource
    //!
    PMOS_RESOURCE m_slbbBuffer = nullptr;

    //!
    //! \brief  SLBB update DMEM buffer array for recycled buffers
    //! \details Centralized buffer managed by base class, populated by derived classes via SetDmem()
    //!
    MOS_RESOURCE m_slbbUpdateDmemBuffer[CODECHAL_ENCODE_RECYCLED_BUFFER_NUM] = {};

    //!
    //! \brief  Size of SLBB update DMEM buffer (default 1024 bytes)
    //! \details Can be overridden by derived classes if different size is needed
    //!
    uint32_t m_slbbUpdateDmemBufferSize = 1024;

    uint32_t m_vdboxHucKernelDescriptor = 0;

#if _SW_BRC
        //!
        //! \brief  SW SLBB instance for SLBB update operations
        //!
        std::shared_ptr<EncodeSwBrc> m_swSLBB = nullptr;
#endif  // !_SW_BRC

    MEDIA_CLASS_DEFINE_END(encode__HucSLBBUpdatePkt)
    };

}  // namespace encode

#endif  // __HUC_SLBB_UPDATE_PKT_H__