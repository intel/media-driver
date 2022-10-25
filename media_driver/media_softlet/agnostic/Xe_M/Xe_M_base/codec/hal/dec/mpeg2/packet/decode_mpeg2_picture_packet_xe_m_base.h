/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_mpeg2_picture_packet_xe_m_base.h
//! \brief    Defines the implementation of mpeg2 decode picture packet for Xe_M_base
//!

#ifndef __DECODE_MPEG2_PICTURE_PACKET_XE_M_BASE_H__
#define __DECODE_MPEG2_PICTURE_PACKET_XE_M_BASE_H__

#include "media_cmd_packet.h"
#include "decode_mpeg2_pipeline.h"
#include "decode_utils.h"
#include "decode_mpeg2_basic_feature.h"
#include "mhw_vdbox_g12_X.h"
#include "codechal_hw_g12_X.h"

namespace decode {

    class Mpeg2DecodePicPktXe_M_Base : public DecodeSubPacket
    {
    public:
        //!
        //! \brief  Mpeg2DecodePicPktXe_M_Base constructor
        //!
        Mpeg2DecodePicPktXe_M_Base(Mpeg2Pipeline *pipeline, CodechalHwInterface *hwInterface)
            : DecodeSubPacket(pipeline, *hwInterface), m_mpeg2Pipeline(pipeline)
        {
            m_hwInterface = hwInterface;
            if (m_hwInterface != nullptr)
            {
                m_miInterface  = m_hwInterface->GetMiInterface();
                m_osInterface  = m_hwInterface->GetOsInterface();
                m_mfxInterface = static_cast<CodechalHwInterfaceG12*>(hwInterface)->GetMfxInterface();
            }
        }

        //!
        //! \brief  Mpeg2DecodePicPktXe_M_Base deconstructor
        //!
        virtual ~Mpeg2DecodePicPktXe_M_Base();

        //!
        //! \brief  Initialize the media packet, allocate required resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Init() override;

        //!
        //! \brief  Prepare interal parameters, should be invoked for each frame
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Prepare() override;

        //!
        //! \brief  Execute av1 picture packet
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS Execute(MOS_COMMAND_BUFFER& cmdBuffer) = 0;

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
        MOS_STATUS CalculateCommandSize(
            uint32_t& commandBufferSize,
            uint32_t& requestedPatchListSize) override;

    protected:

        virtual MOS_STATUS AllocateFixedResources();

        virtual MOS_STATUS SetMfxSurfaceParams(MHW_VDBOX_SURFACE_PARAMS& dstSurfaceParams);
        virtual MOS_STATUS SetMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);
        virtual void       SetMfxPipeModeSelectParams(MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12& vdboxPipeModeSelectParams);
        virtual void       SetMfxIndObjBaseAddrParams(MHW_VDBOX_IND_OBJ_BASE_ADDR_PARAMS& indObjBaseAddrParams);
        virtual void       SetMfxBspBufBaseAddrParams(MHW_VDBOX_BSP_BUF_BASE_ADDR_PARAMS& bspBufBaseAddrParams);
        virtual void       SetMfxMpeg2PicStateParams(MHW_VDBOX_MPEG2_PIC_STATE& mpeg2PicState);
        virtual void       SetMfxQmParams(MHW_VDBOX_QM_PARAMS& qmParams);

        MOS_STATUS         FixMfxPipeBufAddrParams(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

        virtual MOS_STATUS AddMfxSurfacesCmd(MOS_COMMAND_BUFFER& cmdBuffer);
        virtual MOS_STATUS AddMfxIndObjBaseAddrCmd(MOS_COMMAND_BUFFER& cmdBuffer);
        virtual MOS_STATUS AddMfxBspBufBaseAddrCmd(MOS_COMMAND_BUFFER& cmdBuffer);
        virtual MOS_STATUS AddMfxMpeg2PicCmd(MOS_COMMAND_BUFFER& cmdBuffer);
        virtual MOS_STATUS AddMfxQmCmd(MOS_COMMAND_BUFFER& cmdBuffer);

        virtual MOS_STATUS AddMfxPipeBufAddrCmd(MOS_COMMAND_BUFFER& cmdBuffer) = 0;
        virtual MOS_STATUS AddMfxPipeModeSelectCmd(MOS_COMMAND_BUFFER& cmdBuffer) = 0;

        //!
        //! \brief  Free resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS FreeResources();

        //!
        //! \brief  Dump resources
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        MOS_STATUS DumpResources(MHW_VDBOX_PIPE_BUF_ADDR_PARAMS& pipeBufAddrParams);

        //Interfaces
        Mpeg2Pipeline* m_mpeg2Pipeline = nullptr;
        MhwVdboxMfxInterface* m_mfxInterface = nullptr;
        Mpeg2BasicFeature* m_mpeg2BasicFeature = nullptr;
        DecodeAllocator* m_allocator = nullptr;
        DecodeMemComp* m_mmcState = nullptr;

        CodecDecodeMpeg2PicParams* m_mpeg2PicParams = nullptr; //!< Pointer to picture parameter

        PMOS_BUFFER                 m_resMfdDeblockingFilterRowStoreScratchBuffer = nullptr; //!< Handle of MFD Deblocking Filter Row Store Scratch data surface
        PMOS_BUFFER                 m_resBsdMpcRowStoreScratchBuffer = nullptr;              //!< Handle of MPR Row Store Scratch data surface

        uint32_t m_pictureStatesSize = 0;    //!< Picture states size
        uint32_t m_picturePatchListSize = 0;    //!< Picture patch list size

        CodechalHwInterface *m_hwInterface = nullptr;
        MhwMiInterface      *m_miInterface = nullptr;

    private:
        //!
        //! \enum  DecodeRefAddrIndex
        //! \brief Reference address indexes
        //!
        enum DecodeRefAddrIndex
        {
            // MPEG2 reference address indexes
            CodechalDecodeFwdRefTop = 0,    //!< forward reference top field
            CodechalDecodeBwdRefTop = 1,    //!< backward reference top field
            CodechalDecodeFwdRefBottom = 2,    //!< forward reference bottom field
            CodechalDecodeBwdRefBottom = 3,    //!< backward reference bottom field
        };
        //!
        //! \enum     Mpeg2Vc1PictureStructure
        //! \brief    MPEG2 VC1 picture structure
        //!
        enum Mpeg2Vc1PictureStructure
        {
            mpeg2Vc1TopField = 1,
            mpeg2Vc1BottomField,
            mpeg2Vc1Frame
        };
    MEDIA_CLASS_DEFINE_END(decode__Mpeg2DecodePicPktXe_M_Base)
    };

}  // namespace decode
#endif
