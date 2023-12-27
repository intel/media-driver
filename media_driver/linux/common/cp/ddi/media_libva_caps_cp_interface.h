/*
* Copyright (c) 2017-2020, Intel Corporation
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
#include "media_libva_caps.h"

class MediaLibvaCapsCpInterface
{
public:
    MediaLibvaCapsCpInterface(DDI_MEDIA_CONTEXT *mediaCtx, MediaLibvaCaps *mediaCaps);
    virtual ~MediaLibvaCapsCpInterface() {}

    virtual bool IsDecEncryptionSupported(DDI_MEDIA_CONTEXT *mediaCtx);
    virtual int32_t GetEncryptionTypes(
        VAProfile profile,
        uint32_t *encrytionType,
        uint32_t arraySize);

    virtual VAStatus LoadCpProfileEntrypoints();

    virtual bool IsCpConfigId(VAConfigID configId);

    virtual VAStatus CreateCpConfig(
        int32_t profileTableIdx,
        VAEntrypoint entrypoint,
        VAConfigAttrib *attribList,
        int32_t numAttribs,
        VAConfigID *configId);

protected:
    DDI_MEDIA_CONTEXT *m_mediaCtx; //!< Pointer to media context
    MediaLibvaCaps *m_mediaCaps;

    MediaLibvaCaps::ProfileEntrypoint* GetProfileEntrypoint(int32_t profileTableIdx);

    uint16_t GetProfileEntryCount();

    VAStatus GetProfileEntrypointFromConfigId(VAConfigID configId,
            VAProfile *profile,
            VAEntrypoint *entrypoint,
            int32_t *profileTableIdx);

    VAStatus AddProfileEntry(
        VAProfile profile,
        VAEntrypoint entrypoint,
        AttribMap *attributeList,
        int32_t configIdxStart,
        int32_t configNum);

    MEDIA_CLASS_DEFINE_END(MediaLibvaCapsCpInterface)
};

//!
//! \brief    Create MediaLibvaCapsCpInterface Object
//!           Must use Delete_MediaLibvaCapsCpInterface to delete created Object to avoid ULT Memory Leak errors
//!
//! \return   Return CP Wrapper Object
//!
MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCpInterface(DDI_MEDIA_CONTEXT *mediaCtx, MediaLibvaCaps *mediaCaps);

//!
//! \brief    Delete the MediaLibvaCapsCpInterface Object
//!
//! \param    [in] pMediaLibvaCapsCpInterface
//!           MediaLibvaCapsCpInterface
//!
void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pMediaLibvaCapsCpInterface);
#endif
