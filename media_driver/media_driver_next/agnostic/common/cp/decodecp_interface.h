/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     decodecp_interface.h
//! \brief    Impelements the public interface for CodecHal Secure Decode
//!
#ifndef __DECODE_CP_INTERFACE_H__
#define __DECODE_CP_INTERFACE_H__

#include "mos_util_debug.h"
#include "codechal_setting.h"
#include "mos_os_specific.h"
#include "codechal_hw.h"
#include "mos_utilities.h"

#define DECODE_CP_ASSERT(_ptr) \
    MOS_ASSERT(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _ptr)

#define DECODE_CP_ASSERTMESSAGE(_message, ...) \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DECODE_CP_NORMALMESSAGE(_message, ...) \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DECODE_CP_VERBOSEMESSAGE(_message, ...) \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _message, ##__VA_ARGS__)

#define DECODE_CP_CHK_STATUS(_stmt) \
    MOS_CHK_STATUS_RETURN(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _stmt)

#define DECODE_CP_CHK_STATUS_MESSAGE(_stmt, _message, ...) \
    MOS_CHK_STATUS_MESSAGE_RETURN(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _stmt, _message, ##__VA_ARGS__)

#define DECODE_CP_CHK_NULL(_ptr) \
    MOS_CHK_NULL(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _ptr)

#define DECODE_CP_CHK_NULL_RETURN(_ptr) \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CODEC, MOS_CODEC_SUBCOMP_DECODE, _ptr)

#define DECODE_CP_CHK_NULL_NO_STATUS_RETURN(_ptr) \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, _ptr)

#define DECODE_CP_CHK_COND(_condition, retVal, _message, ...) \
    MOS_CHK_COND_RETURN_VALUE(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, (_condition), (retVal), (_message), ##__VA_ARGS__)

#define DECODE_CP_FUNCTION_ENTER \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC)

#define DECODE_CP_FUNCTION_EXIT(eStatus) \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_CP, MOS_CP_SUBCOMP_CODEC, eStatus)

class DecodeCpInterface
{
public:
    virtual ~DecodeCpInterface(){};
    //!
    //! \brief  Add Hcp SecureState
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!

    virtual MOS_STATUS AddHcpState(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE presDataBuffer,
        uint32_t length,
        uint32_t startoffset,
        uint32_t dwsliceIndex) = 0;
    //!
    //! \brief  Add Huc State
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddHucState(
        PMOS_COMMAND_BUFFER cmdBuffer,
        PMOS_RESOURCE presDataBuffer,
        uint32_t length,
        uint32_t startoffset,
        uint32_t dwsliceIndex) = 0;

    virtual MOS_STATUS SetHucDmemS2LPicBss(
        void *hucS2LPicBss,
        PMOS_RESOURCE presDataBuffer) = 0;

    virtual MOS_STATUS SetHucDmemS2LSliceBss(
        void *hucS2LSliceBss,
        uint32_t index,
        uint32_t dataSize,
        uint32_t dataOffset) = 0;

    virtual MOS_STATUS RegisterParams(void *settings) = 0;

    virtual MOS_STATUS UpdateParams(bool input) = 0;
};

//!
//! \brief    Create DecodeCpInterface Object according CPLIB loading status
//!           Must use Delete_DecodeCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//
DecodeCpInterface *Create_DecodeCpInterface(
    CodechalSetting *      codechalSettings,
    CodechalHwInterface *  hwInterfaceInput);

//!
//! \brief    Delete the DecodeCpInterface Object according CPLIB loading status
//!
//! \param    [in] *pMhwCpInterface
//!           MhwCpInterface
//!
void Delete_DecodeCpInterface(DecodeCpInterface* pDecodeCpInterface);

#endif // __DECODE_CP_INTERFACE_H__