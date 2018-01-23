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
//! \file     media_libva_caps_cp.h
//! \brief    This file defines the C++ class/interface for encryption related capbilities. 
//!

#ifndef __MEDIA_LIBVA_CAPS_CP_H__
#define __MEDIA_LIBVA_CAPS_CP_H__

#include "media_libva.h"

class MediaLibvaCapsCp
{
public:
    //!
    //! \brief    Return if decode encrytion is supported 
    //!
    //! \param    [in] mediaCtx 
    //!           Pointer to DDI_MEDIA_CONTEXT
    //!
    //! \return   false: decode encrytion isn't supported on current platform
    //!           true: decode encrytion is supported on current platform
    //!
    static bool IsDecEncryptionSupported(DDI_MEDIA_CONTEXT *mediaCtx)
    {
        return false;
    }

    //!
    //! \brief    Get the supported decode encrytion types 
    //!
    //! \param    [in] profile 
    //!           VAProfile
    //!
    //! \param    [in/out] encrytionType 
    //!           Pointer to a array of uint32_t, which will be filled with result. 
    //!
    //! \param    [in] arraySize 
    //!           The array size of encrytionType. 
    //!
    //! \return   Return the real number of supported decode encrytion types 
    //!           Return -1 if arraySize is too small or profile is invalide 
    //!

    static int32_t GetEncryptionTypes(VAProfile profile, uint32_t *encrytionType, uint32_t arraySize)
    {
        return -1;
    }
};
#endif
