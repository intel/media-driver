/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_blt_copy.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_H__
#define __MEDIA_BLT_COPY_H__

#include <stdint.h>
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "mos_resource_defs.h"
#include "mos_util_debug.h"
#include "media_interfaces_mhw.h"
#include "mhw_blt_legacy.h"
#include "mos_os.h"
#include "media_class_trace.h"
#include "media_copy_common.h"
class MhwCpInterface;
class MhwMiInterface;

#define BLT_CHK_STATUS(_stmt)               MOS_CHK_STATUS(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _stmt)
#define BLT_CHK_STATUS_RETURN(_stmt)        MOS_CHK_STATUS_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _stmt)
#define BLT_CHK_NULL(_ptr)                  MOS_CHK_NULL(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _ptr)
#define BLT_CHK_NULL_RETURN(_ptr)           MOS_CHK_NULL_RETURN(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _ptr)
#define BLT_ASSERTMESSAGE(_message, ...)    MOS_ASSERTMESSAGE(MOS_COMPONENT_MCPY, MOS_MCPY_SUBCOMP_BLT, _message, ##__VA_ARGS__)

class BltState
{
public:
    //!
    //! \brief    BltState constructor
    //! \details  Initialize the BltState members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltState(PMOS_INTERFACE     osInterface);
    BltState(PMOS_INTERFACE    osInterface, MhwInterfaces* mhwInterfaces);

    virtual ~BltState();

    //!
    //! \brief    BltState initialize
    //! \details  Initialize the BltState, create BLT context.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize();

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination surface
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_SURFACE src,
        PMOS_SURFACE dst);

    //!
    //! \brief    Copy main surface
    //! \details  BLT engine will copy source surface to destination surface
    //! \param    src
    //!           [in] Pointer to source resource
    //! \param    dst
    //!           [in] Pointer to destination resource
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS CopyMainSurface(
        PMOS_RESOURCE src,
        PMOS_RESOURCE dst);
    //!
    //! \brief    Setup blt copy parameters
    //! \details  Setup blt copy parameters for BLT Engine
    //! \param    mhwParams
    //!           [in/out] Pointer to MHW_FAST_COPY_BLT_PARAM
    //! \param    inputSurface
    //!           [in] Pointer to input surface
    //! \param    outputSurface
    //!           [in] Pointer to output surface
    //! \param    planeIndex
    //!           [in] Pointer to YUV(RGB) plane index
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SetupBltCopyParam(
      PMHW_FAST_COPY_BLT_PARAM mhwParams,
      PMOS_RESOURCE            inputSurface,
      PMOS_RESOURCE            outputSurface,
      int                      planeIndex);

    //!
    //! \brief    Submit command
    //! \details  Submit BLT command
    //! \param    pBltStateParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD(
        PBLT_STATE_PARAM pBltStateParam);

    //!
    //! \brief    Get control surface
    //! \details  BLT engine will copy aux data of source surface to destination
    //! \param    src
    //!           [in] Pointer to source surface
    //! \param    dst
    //!           [in] Pointer to destination buffer which is created for aux data
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS GetCCS(
        PMOS_SURFACE src,
        PMOS_SURFACE dst)
    {
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Get Block copy color depth.
    //! \details  get different format's color depth.
    //! \param    Gmm format and bits per Pixel
    //!           [in] Gmm format, Bits per Pixel;
    //! \return   color depth
    //!           Return color depth
    //!
    uint32_t GetBlkCopyColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            BitsPerPixel);

    //!
    //! \brief    Get Fast copy color depth.
    //! \details  get different format's color depth.
    //! \param    Gmm format and bits per Pixel
    //!           [in] Gmm format, Bits per Pixel;
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    uint32_t GetFastCopyColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            BitsPerPixel);
    //!
    //! \brief    Get plane's byte per texel
    //! \details  Get plane's byte per texel
    //! \param    MOS_FORMAT format
    //!           [in] GMM resource format
    //! \return   int
    //!           return the scaling ratio;
    //!
    int GetBytesPerTexelScaling(MOS_FORMAT format);

    //!
    //! \brief    Get plane number
    //! \details  Get plane number
    //! \param    MOS_FORMAT format
    //!           [in] GMM resource format
    //! \return   int
    //!           return the plane number
    //!
    int GetPlaneNum(MOS_FORMAT format);

    //!
    //! \brief    SetPrologParamsforCmdbuffer
    //! \details  Set PrologParams for Cmdbuffer
    //! \param    PMOS_COMMAND_BUFFER
    //!           [in] Pointer to PMOS_COMMAND_BUFFER
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS SetPrologParamsforCmdbuffer(PMOS_COMMAND_BUFFER cmdBuffer);

public:
    bool           m_blokCopyon       = false;
    PMOS_INTERFACE m_osInterface      = nullptr;
    MhwInterfaces *m_mhwInterfaces    = nullptr;
    MhwMiInterface *m_miInterface     = nullptr;
    PMHW_BLT_INTERFACE m_bltInterface = nullptr;
    MhwCpInterface *m_cpInterface     = nullptr;
    MhwInterfaces::CreateParams params;
MEDIA_CLASS_DEFINE_END(BltState)
};

#endif // __MEDIA_BLT_COPY_H__
