/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     decode_huc_authcheck_packet_m12.h
//! \brief    Defines the implementation of HuC authentication check packet
//!

#ifndef __DECODE_HUC_AUTHCHECK_PACKET_M12_H__
#define __DECODE_HUC_AUTHCHECK_PACKET_M12_H__

#include "decode_allocator.h"
#include "decode_utils.h"
#include "decode_pipeline.h"
#include "codechal_hw_g12_X.h"

namespace decode
{
    class DecodeHucAuthCheckPktM12
    {
    public:
        DecodeHucAuthCheckPktM12(MediaPipeline *pipeline, CodechalHwInterface *hwInterface)
        {
            m_hwInterface = dynamic_cast<CodechalHwInterfaceG12 *>(hwInterface);
            m_pipeline = dynamic_cast<DecodePipeline *>(pipeline);

            if (hwInterface != nullptr)
            {
                m_miInterface = hwInterface->GetMiInterface();
                m_osInterface = hwInterface->GetOsInterface();
                m_hucInterface = hwInterface->GetHucInterface();
            }
        }
        virtual ~DecodeHucAuthCheckPktM12() {}

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Init();

        //!
        //! \brief  Destroy allocated resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Destroy();

        //!
        //! \brief  Execute hevc picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS Execute(MOS_COMMAND_BUFFER &cmdBuffer);

        PMHW_BATCH_BUFFER GetSecondLvlBB() { return m_batchBuf; };

    private:
        MOS_STATUS Init2ndLevelCmdBuffer(MHW_BATCH_BUFFER &batchBuffer, uint8_t *batchBufBase);

        MOS_STATUS PackHucAuthCmds(MOS_COMMAND_BUFFER &cmdBuffer);

        PMOS_INTERFACE          m_osInterface  = nullptr;
        CodechalHwInterfaceG12 *m_hwInterface  = nullptr;
        MhwMiInterface         *m_miInterface  = nullptr;
        MhwVdboxHucInterface   *m_hucInterface = nullptr;

        DecodePipeline    *m_pipeline           = nullptr;
        DecodeAllocator   *m_allocator          = nullptr;
        MOS_BUFFER        *m_hucAuthCheckBuf    = nullptr;  //!< Pointer to Huc authentication buffer
        BatchBufferArray  *m_secondLevelBBArray = nullptr;  //!< Pointer to second level batch buffer
        PMHW_BATCH_BUFFER  m_batchBuf           = nullptr;
        MOS_COMMAND_BUFFER m_hucAuthCmdBuffer   = {};

MEDIA_CLASS_DEFINE_END(decode__HucAuthCheckPktM12)
    };
}  // namespace decode
#endif
