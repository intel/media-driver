/*
* Copyright (c) 2017, Intel Corporation
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
//! \file     media_libva_caps_cp_interface.h
//! \brief    This file defines the C++ class/interface for encryption related capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_CP_INTERFACE_H__
#define __MEDIA_LIBVA_CAPS_CP_INTERFACE_H__

#include "media_libva.h"

class MediaLibvaCapsCpInterface
{
public:
    MediaLibvaCapsCpInterface() {}
    virtual ~MediaLibvaCapsCpInterface() {}
    
    virtual bool IsDecEncryptionSupported(DDI_MEDIA_CONTEXT *mediaCtx);
    virtual int32_t GetEncryptionTypes(
        VAProfile profile, 
        uint32_t *encrytionType, 
        uint32_t arraySize);
};

//!
//! \brief    Create MediaLibvaCapsCpInterface Object according CPLIB loading status
//!           Must use Delete_MediaLibvaCapsCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object if CPLIB not loaded
//!
MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCpInterface();

//!
//! \brief    Delete the MediaLibvaCapsCpInterface Object according CPLIB  loading status
//!
//! \param    [in] pMediaLibvaCapsCpInterface 
//!           MediaLibvaCapsCpInterface
//!
void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface);
#endif
