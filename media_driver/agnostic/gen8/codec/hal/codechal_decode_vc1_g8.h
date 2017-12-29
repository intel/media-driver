/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     codechal_decode_vc1_g8.h
//! \brief    Defines the decode interface extension for Gen8 VC1.
//! \details  Defines all types, macros, and functions required by CodecHal for Gen8 VC1 decoding.
//!           Definitions are not externally facing.
//!

#ifndef __CODECHAL_DECODER_VC1_G8_H__
#define __CODECHAL_DECODER_VC1_G8_H__

#include "codechal.h"
#include "codechal_decode_vc1.h"

//!
//! \struct CODECHAL_DECODE_VC1_KERNEL_HEADER
//! \brief Define VC1 Kernel Header
//!
typedef struct _CODECHAL_DECODE_VC1_KERNEL_HEADER {
    int nKernelCount;

    CODECHAL_KERNEL_HEADER Vc1KernelHeader1[30];
    CODECHAL_KERNEL_HEADER OLP;
    CODECHAL_KERNEL_HEADER Vc1KernelHeader2[16];
} CODECHAL_DECODE_VC1_KERNEL_HEADER, *PCODECHAL_DECODE_VC1_KERNEL_HEADER;

//!
//! \struct CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8
//! \brief Define VC1 OLP Static Data for gen8
//!
typedef struct _CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   Reserved                : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t   BlockWidth              : 16;   // in byte
            uint32_t   BlockHeight             : 16;   // in byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t   Profile                         : 1;
            uint32_t   RangeExpansionFlag              : 1;    // Simple & Main Profile only
            uint32_t   PictureUpsamplingFlag           : 2;    // 2:H, 3:V
            uint32_t                                   : 1;
            uint32_t   InterlaceFieldFlag              : 1;
            uint32_t                                   : 2;
            uint32_t   RangeMapUV                      : 3;
            uint32_t   RangeMapUVFlag                  : 1;
            uint32_t   RangeMapY                       : 3;
            uint32_t   RangeMapYFlag                   : 1;
            uint32_t                                   : 4;
            uint32_t   ComponentFlag                   : 1;
            uint32_t                                   : 11;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3
    union
    {
        struct
        {
            uint32_t                                   : 4;
            uint32_t   ComponentFlag                   : 1;
            uint32_t                                   : 27;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // uint32_t 4
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // uint32_t 5
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    // uint32_t 6
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW6;

    // uint32_t 7
    union
    {
        struct
        {
            uint32_t   Reserved;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW7;

} CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8, *PCODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8;

//!
//! \struct CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8
//! \brief Define VC1 OLP Inline Data for gen8
//!
typedef struct _CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8
{
    // uint32_t 0
    union
    {
        struct
        {
            uint32_t   BlockOriginX : 16;  // in Byte
            uint32_t   BlockOriginY : 16;  // in Byte
        };
        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // uint32_t 1
    union
    {
        struct
        {
            uint32_t                 : 4;
            uint32_t   ComponentFlag : 1;
            uint32_t                 : 27;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // uint32_t 2
    union
    {
        struct
        {
            uint32_t   SourceDataBindingIndex   : 8;
            uint32_t   DestDataBindingIndex     : 8;
            uint32_t                            : 16;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // uint32_t 3 - 7
    uint32_t Reserved[5];
} CODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8, *PCODECHAL_DECODE_VC1_OLP_INLINE_DATA_G8;

//!
//! \def CODECHAL_DECODE_VC1_CURBE_SIZE_OLP_G8
//! VC1 Curbe Size for Gen8 Olp
//!
#define CODECHAL_DECODE_VC1_CURBE_SIZE_OLP_G8             \
    (sizeof(CODECHAL_DECODE_VC1_OLP_STATIC_DATA_G8))

//!
//! \class CodechalDecodeVc1G8
//! \brief This class defines the member fields, functions etc used by Gen8 VC1 decoder.
//!
class CodechalDecodeVc1G8 : public CodechalDecodeVc1
{
public:
    //!
    //! \brief  Constructor
    //! \param    [in] hwInterface
    //!           Hardware interface
    //! \param    [in] debugInterface
    //!           Debug interface
    //! \param    [in] standardInfo
    //!           The information of decode standard for this instance
    //!
    CodechalDecodeVc1G8(
        CodechalHwInterface   *hwInterface,
        CodechalDebugInterface* debugInterface,
        PCODECHAL_STANDARD_INFO standardInfo);

    ~CodechalDecodeVc1G8();

    MOS_STATUS AllocateResources();

    MOS_STATUS SetCurbeOlp();

    //!
    //! \brief    Add VC1 Olp MediaObjects to a batch buffer
    //! \details  Populate the OLP Media Objects and adds them to a batch buffer
    //! \param    [in,out] batchBuffer
    //!           Pointer of Batch Buffer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AddVc1OlpMediaObjectsBB(
        PMHW_BATCH_BUFFER               batchBuffer);

    MOS_STATUS UpdateVc1KernelState();

    MOS_STATUS AddVc1OlpCmd(
        PCODECHAL_DECODE_VC1_OLP_PARAMS vc1OlpParams);

protected:
    MHW_BATCH_BUFFER m_olpBatchBuffer;  //!< Olp Batch Buffer
};
#endif  // __CODECHAL_DECODER_VC1_G8_H__
