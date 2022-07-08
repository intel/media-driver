/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_sfc_interface_legacy.h
//! \brief    Common interface and structure used in sfc interface
//! \details  Common interface and structure used in sfc interface which are platform independent
//!
#ifndef __MEDIA_SFC_INTERFACE_LEGACY_H__
#define __MEDIA_SFC_INTERFACE_LEGACY_H__

#include "mos_os_specific.h"
#include "media_sfc_interface.h"

class MediaSfcInterfaceLegacy : public MediaSfcInterface
{
public:
    //!
    //! \brief    MediaSfcInterface constructor
    //! \details  Initialize the MediaSfcInterface members.
    //! \param    osInterface
    //!           [in] Pointer to MOS_INTERFACE.
    //! \param    mmc
    //!           [in] Pointer to MediaMemComp.
    //!
    MediaSfcInterfaceLegacy(PMOS_INTERFACE osInterface, MediaMemComp *mmc = nullptr);

    //!
    //! \brief    MediaSfcInterface initialize
    //! \details  Initialize the MediaSfcInterface.
    //! \param    mode
    //!           [in] 1: VEBOX-SFC only, 2: VDBOX-SFC only, 3: Both VEBOX-SFC and VDBOX-SFC.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    virtual MOS_STATUS Initialize(MEDIA_SFC_INTERFACE_MODE mode) override;

MEDIA_CLASS_DEFINE_END(MediaSfcInterfaceLegacy)
};

#endif // __MEDIA_SFC_INTERFACE_H__
