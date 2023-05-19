/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     encode_packet_utilities.h
//! \brief    Defines encode packrt utilities.
//!

#ifndef __ENCODE_PACKET_UTILITIES_H__
#define __ENCODE_PACKET_UTILITIES_H__

#include "mhw_mi.h"
#include "mos_util_debug.h"
#include "encode_utils.h"
#include "codec_hw_next.h"
#include "codechal_debug.h"
#include "encode_basic_feature.h"
#include "mhw_vdbox.h"

namespace encode
{
class PacketUtilities
{
public:
    PacketUtilities(CodechalHwInterfaceNext *hwInterface, MediaFeatureManager *featureManager);

    ~PacketUtilities();

    MOS_STATUS AddMemCopyCmd(PMOS_COMMAND_BUFFER cmdBuf, PMOS_RESOURCE pDst, PMOS_RESOURCE pSrc, uint32_t size);
    MOS_STATUS AddStoreDataImmCmd(PMOS_COMMAND_BUFFER cmdBuf, PMOS_RESOURCE pSrc, uint32_t offset, uint32_t flag);

    MOS_STATUS Init();

    MOS_STATUS SendMarkerCommand(PMOS_COMMAND_BUFFER cmdBuffer, PMOS_RESOURCE presSetMarker);

    MOS_STATUS SendPredicationCommand(PMOS_COMMAND_BUFFER cmdBuffer);

#if USE_CODECHAL_DEBUG_TOOL
    //!
    //! \brief  Modify the frame size with fake header size
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] fakeHeaderSizeInByte
    //!         fake header size in bytes
    //! \param  [in] resBrcUpdateCurbe
    //!         Curebe/Dmem for brcupdate kernel
    //! \param  [in] targetSizePos
    //!         offset of targetSize in resBrcUpdateCurbe
    //! \param  [in] resPakStat
    //!         Pak stastics
    //! \param  [in] slcHrdSizePos
    //!         offset of slcHrdSizePos in resPakStat
    //!
    //! \return MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ModifyEncodedFrameSizeWithFakeHeaderSize(
        PMOS_COMMAND_BUFFER     cmdBuffer,
        uint32_t                fakeHeaderSizeInByte,
        PMOS_RESOURCE           resBrcUpdateCurbe,
        uint32_t                targetSizePos,
        PMOS_RESOURCE           resPakStat,
        uint32_t                slcHrdSizePos);

    //!
    //! \brief  Modify the frame size with fake header size for AVC
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] fakeHeaderSizeInByte
    //!         fake header size in bytes
    //! \param  [in] resBrcUpdateCurbe
    //!         Curebe/Dmem for brcupdate kernel
    //! \param  [in] targetSizePos
    //!         offset of targetSize in resBrcUpdateCurbe
    //! \param  [in] resPakStat
    //!         Pak stastics
    //! \param  [in] slcHrdSizePos
    //!         offset of slcHrdSizePos in resPakStat
    //!
    //! \return MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ModifyEncodedFrameSizeWithFakeHeaderSizeAVC(
        PMOS_COMMAND_BUFFER cmdBuffer,
        uint32_t            fakeHeaderSizeInByte,
        PMOS_RESOURCE       *resBrcUpdateCurbe,
        uint32_t            targetSizePos,
        PMOS_RESOURCE       resPakStat,
        uint32_t            slcHrdSizePos);

    //!
    //! \brief  Add/Subtract a value to specified gfx memory
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] presStoreBuffer
    //!         buffer to modify
    //! \param  [in] offset
    //!         member offset in the buffer
    //! \param  [in] value
    //!         value to add/subtract
    //! \param  [in] bAdd
    //!         add or subtract
    //!
    //! \return MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddBufferWithIMMValue(
        PMOS_COMMAND_BUFFER     cmdBuffer,
        PMOS_RESOURCE           presStoreBuffer,
        uint32_t                offset,
        uint32_t                value,
        bool                    bAdd);

    //!
    //! \brief  Set a 16 bit value to specified gfx memory dword
    //!
    //! \param  [in] cmdBuffer
    //!         command buffer
    //! \param  [in] presStoreBuffer
    //!         buffer to modify
    //! \param  [in] offset
    //!         member offset in the buffer
    //! \param  [in] value
    //!         value to set
    //! \param  [in] bSecond
    //!         second or first word in dword
    //!
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetBufferWithIMMValueU16(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE       presStoreBuffer,
        uint32_t            offset,
        uint32_t            value,
        bool                bSecond);

    bool GetFakeHeaderSettings(uint32_t &fakeHeaderSizeInByte, bool isIframe);
#endif

protected:
    MediaFeatureManager          *m_featureManager = nullptr;
    CodechalHwInterfaceNext      *m_hwInterface    = nullptr;
    std::shared_ptr<mhw::mi::Itf> m_miItf          = nullptr;
    MHW_VDBOX_NODE_IND            m_vdboxIndex     = MHW_VDBOX_NODE_1;
    MediaUserSettingSharedPtr     m_userSettingPtr = nullptr;

#if USE_CODECHAL_DEBUG_TOOL
    bool    m_enableFakeHrdSize     = false;
    int32_t m_fakeIFrameHrdSize     = 0;
    int32_t m_fakePBFrameHrdSize    = 0;
#endif

MEDIA_CLASS_DEFINE_END(encode__PacketUtilities)
};

}  // namespace encode
#endif
