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
//! \file     media_blt_copy_next.h
//! \brief    Common interface and structure used in Blitter Engine
//! \details  Common interface and structure used in Blitter Engine which are platform independent
//!
#ifndef __MEDIA_BLT_COPY_NEXT_H__
#define __MEDIA_BLT_COPY_NEXT_H__

#include "media_interfaces_mhw_next.h"
#include "mhw_blt.h"
#include "mhw_mi.h"
#include "mhw_cp_interface.h"
#include "mos_os.h"
#include "media_copy.h"
#include "media_copy_common.h"

class BltStateNext
{
public:
    //!
    //! \brief    BltStateNext constructor
    //! \details  Initialize the BltStateNext members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //!
    BltStateNext(PMOS_INTERFACE     osInterface);
    BltStateNext(PMOS_INTERFACE    osInterface, MhwInterfacesNext* mhwInterfaces);

    virtual ~BltStateNext();

    //!
    //! \brief    BltStateNext initialize
    //! \details  Initialize the BltStateNext, create BLT context.
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
    //! \param    plane index
    //!           [in] plan index, e.g Y plane index 0, UV plane index 1.
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
    //! \param    pBltStateNextParam
    //!           [in] Pointer to BLT_STATE_PARAM
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS SubmitCMD(
        PBLT_STATE_PARAM pBltStateNextParam);

    //!
    //! \brief    Get Block copy color depth.
    //! \details  get different format's color depth.
    //! \param    surface 
    //!           [in] input or output surface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    uint32_t GetBlkCopyColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            BytesPerTexel);

        //!
    //! \brief    Get Fast copy color depth.
    //! \details  get different format's color depth.
    //! \param    surface
    //!           [in] input or output surface.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    uint32_t GetFastCopyColorDepth(
        GMM_RESOURCE_FORMAT dstFormat,
        uint32_t            BytesPerTexel);
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

public:
    bool               m_blokCopyon       = false;
    PMOS_INTERFACE     m_osInterface      = nullptr;
    MhwInterfacesNext *m_mhwInterfaces    = nullptr;
    MhwCpInterface    *m_cpInterface      = nullptr;

    std::shared_ptr<mhw::mi::Itf>   m_miItf  = nullptr;
    std::shared_ptr<mhw::blt::Itf>  m_bltItf = nullptr;
    
    MEDIA_CLASS_DEFINE_END(BltStateNext)
};

#endif // __MEDIA_BLT_COPY_H__
