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
//! \file     ddi_cp_caps_interface.h
//! \brief    cp caps interface head file
//!

#ifndef __DDI_CP_CAPS_INTERFACE_H__
#define __DDI_CP_CAPS_INTERFACE_H__

#include "media_libva_common_next.h"
#include "capstable_data_linux_definition.h"
#include "media_factory.h"

class DdiCpCapsInterface
{
public:
    DdiCpCapsInterface() {}

    virtual ~DdiCpCapsInterface() {}

    //!
    //! \brief    InitProfileMap
    //!
    //! \param    [in] mediaCtx
    //!           media context for skutable get
    //! \param    [in out] profileMap
    //!           profile map table
    //!
    //! \return   VAStatus
    //!           VA_STATUS_SUCCESS if success
    //!
    virtual VAStatus InitProfileMap(DDI_MEDIA_CONTEXT *mediaCtx, ProfileMap *profileMap) { return VA_STATUS_SUCCESS; };

    //!
    //! \brief    Check if the configID is a valid cp config id
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   True if the configID is a valid cp config, otherwise false
    //!
    virtual bool IsCpConfigId(VAConfigID configId) { return false; }

    //!
    //! \brief    Get the vaild cp ConfigID
    //!
    //! \param    [in] configId
    //!           Specify the VAConfigID
    //!
    //! \return   Cp config ID
    //!
    virtual uint32_t GetCpConfigId(VAConfigID configId) { return 0xFFFFFFFF; }

MEDIA_CLASS_DEFINE_END(DdiCpCapsInterface)
};

enum CapsType
{
    CapsInterface = 0,
    CapsImp       = 1
};

typedef MediaFactory<CapsType, DdiCpCapsInterface> CpCapsFactory;

DdiCpCapsInterface *CreateDdiCpCaps();

#endif //__DDI_CP_CAPS_INTERFACE_H__