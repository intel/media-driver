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
//! \file     encode_avc_header_packer.h
//! \brief    Defines header packing logic for avc encode
//!
#ifndef __ENCODE_AVC_HEADER_PACKER_H__
#define __ENCODE_AVC_HEADER_PACKER_H__

#include "codec_def_encode_avc.h"

namespace encode
{

class AvcEncodeHeaderPacker
{
public:

    //!
    //! \brief  AvcEncodeHeaderPacker constructor
    //!
    AvcEncodeHeaderPacker() {}

    //!
    //! \brief  AvcEncodeHeaderPacker deconstructor
    //!
    virtual ~AvcEncodeHeaderPacker() {}

    //!
    //! \brief    Use to pack picture header related params
    //! \param    [in] params
    //!           picture header pack params
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    static MOS_STATUS PackPictureHeader(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    static MOS_STATUS PackSliceHeader(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params);

protected:

    //!
    //! \brief    Pack AUD parameters
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode Avc pack picture header parameter
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    static MOS_STATUS PackAUDParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    //!
    //! \brief    Pack HRD data
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode Avc pack picture header parameter
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    static MOS_STATUS PackHrdParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    //!
    //! \brief    Pack VUI data
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode Avc pack picture header parameter
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    static MOS_STATUS PackVuiParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    //!
    //! \brief    Pack sequence parameters
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode Avc pack picture header parameter
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    static MOS_STATUS PackSeqParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    //!
    //! \brief    Pack picture parameters
    //!
    //! \param    [in] params
    //!           Pointer to codechal encode Avc pack picture header parameter
    //!
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if call success, else fail reason
    //!
    static MOS_STATUS PackPicParams(PCODECHAL_ENCODE_AVC_PACK_PIC_HEADER_PARAMS params);

    static MOS_STATUS RefPicListReordering(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params);

    static MOS_STATUS PredWeightTable(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params);

    static MOS_STATUS MMCO(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params);

    static void SetInitialRefPicList(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params);

    static MOS_STATUS SetRefPicListParam(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params, uint8_t list);

    static void GetPicNum(PCODECHAL_ENCODE_AVC_PACK_SLC_HEADER_PARAMS params, uint8_t list);

MEDIA_CLASS_DEFINE_END(encode__AvcEncodeHeaderPacker)
};

}  // namespace encode

#endif  // !__ENCODE_AVC_HEADER_PACKER_H__
