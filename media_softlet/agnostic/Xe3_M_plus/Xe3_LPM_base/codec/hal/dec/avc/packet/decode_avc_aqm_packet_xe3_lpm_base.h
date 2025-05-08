/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     decode_avc_aqm_packet_xe3_lpm_base.h
//! \brief    Defines the implementation of avc decode aqm packet
//!

#ifndef __DECODE_AVC_AQM_PACKET_XE3_LPM_BASE_H__
#define __DECODE_AVC_AQM_PACKET_XE3_LPM_BASE_H__

#include "decode_avc_aqm_packet.h"
#ifdef _MEDIA_RESERVED
#include "mhw_vdbox_aqm_impl_xe3_lpm.h"
#endif
#include "decode_avc_downsampling_feature.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

using namespace mhw::vdbox::mfx;
namespace decode
{
    class AvcDecodeAqmPktXe3LpmBase : public AvcDecodeAqmPkt
    {
    public:
        //!
        //! \brief  AvcDecodeAqmPktXe3LpmBase constructor
        //!
        AvcDecodeAqmPktXe3LpmBase(AvcPipeline* pipeline, CodechalHwInterfaceNext* hwInterface)
            : AvcDecodeAqmPkt(pipeline, hwInterface)
        {
            if (m_hwInterface != nullptr)
            {
                m_mfxItf = std::static_pointer_cast<mhw::vdbox::mfx::Itf>(m_hwInterface->GetMfxInterfaceNext());
                m_miItf = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->GetMiInterfaceNext());
#ifdef _MEDIA_RESERVED
                m_aqmItf = std::make_shared<mhw::vdbox::aqm::xe3_lpm::Impl>(m_osInterface);
#endif
            }
        }

        //!
        //! \brief  AvcDecodeAqmPktXe3LpmBase deconstructor
        //!
        virtual ~AvcDecodeAqmPktXe3LpmBase();

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //! \brief  Execute avc aqm packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer);

        //! \brief  Flush avc aqm packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Flush(MOS_COMMAND_BUFFER& cmdBuffer);

        //!
        //! \brief  Calculate Command Size
        //!
        //! \param  [in, out] commandBufferSize
        //!         requested size
        //! \param  [in, out] requestedPatchListSize
        //!         requested size
        //! \return MOS_STATUS
        //!         status
        //!
        //MOS_STATUS CalculateCommandSize(
        //    uint32_t& commandBufferSize,
        //    uint32_t& requestedPatchListSize) override;

        MHW_SETPAR_DECL_HDR(AQM_HIST_BUFF_ADDR_STATE);

        AvcDownSamplingFeature *m_downSampling = nullptr;

    protected:
#ifdef _MEDIA_RESERVED
        std::shared_ptr<mhw::vdbox::aqm::Itf> m_aqmItf = nullptr;
#endif

        MEDIA_CLASS_DEFINE_END(decode__AvcDecodeAqmPktXe3_Lpm_Base)
    };

}  // namespace decode

#endif //_DECODE_PROCESSING_SUPPORTED

#endif //__DECODE_AVC_AQM_PACKET_XE3_LPM_BASE_H__