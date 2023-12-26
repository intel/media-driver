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
//! \file        mos_mediacopy_base.h
//! \brief       This module defines MOS decompression on linux platform
//!
#ifndef __MOS_MEDIACOPY_BASE_H__
#define __MOS_MEDIACOPY_BASE_H__

#include "mos_os.h"
#include "media_copy.h"

class MosMediaCopyBase
{
public:
    //!
    //! \brief    constructor
    //!
    MosMediaCopyBase(PMOS_CONTEXT mosCtx);

    //!
    //! \brief    destructor
    //!
    virtual ~MosMediaCopyBase();

    virtual MediaCopyBaseState **GetMediaCopyState();

    //!
    //! \brief    Media copy
    //! \details  Entry point to copy media memory, input can support both compressed/uncompressed
    //! \param    [in] inputResource
    //!            The surface resource will be decompressed
    //! \param    [out] outputResource
    //!            The target uncompressed surface resource will be copied to
    //! \param    [in] preferMethod
    //!            The preferred copy mode
    //!
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS MediaCopy(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        MCPY_METHOD   preferMethod);

    //!
    //! \brief    Is AIL force opition.
    //! \details  Is AIL force opition.
    //! \return   bool
    //!           Return true if support, otherwise return false.
    bool IsAILForceOption();

    // Get mos interfae
    virtual PMOS_INTERFACE GetMediaCopyMosInterface();

protected:
    MediaCopyBaseState *m_mediaCopyState = nullptr;
    PMOS_CONTEXT        m_mosContext     = nullptr;
MEDIA_CLASS_DEFINE_END(MosMediaCopyBase)
};

#endif // __MOS_MEDIACOPY_BASE_H__
