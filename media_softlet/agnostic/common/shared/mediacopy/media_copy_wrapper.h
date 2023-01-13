/*
* Copyright (c) 2023, Intel Corporation
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
//! \file     media_copy_wrapper.h
//! \brief    Media Copy Base State Wrapper is created in pipeline init
//! \details  Media Copy Base State Wrapper which is created in pipeline init
//!

#ifndef __MEDIA_COPY_WRAPPER_H__
#define __MEDIA_COPY_WRAPPER_H__

#include "media_copy.h"

class MediaCopyWrapper
{
public:
    //!
    //! \brief    constructor
    //!
    MediaCopyWrapper(PMOS_INTERFACE osInterface);

    //!
    //! \brief    destructor
    //!
    virtual ~MediaCopyWrapper();

    //!
    //! \brief    Create Media copy state
    //! \details  Create Media copy state
    //! \return   void
    //!
    void CreateMediaCopyState();

    //!
    //! \brief    Set Media copy Base State which is created outside
    //! \details  Put Media copy Base State into the Media Copy Wrapper
    //! \param    [in] mediaCopyState
    //!           Media Copy Base State
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error.
    //!
    MOS_STATUS SetMediaCopyState(MediaCopyBaseState *mediaCopyState);

    //!
    //! \brief    Get Media copy state
    //! \details  Get Media copy state
    //! \return   The pointer to MediaCopyBaseState
    //!
    MediaCopyBaseState *GetMediaCopyState()
    {
        return m_mediaCopyState;
    }

    //!
    //! \brief    Media copy Base State is nullptr
    //! \details  Media copy Base State is nullptr
    //! \return   true if it is nullptr.
    //!
    bool MediaCopyStateIsNull()
    {
        return (nullptr == m_mediaCopyState);
    }

    //!
    //! \brief    Media copy
    //! \details  Entry point to copy media memory, input can support both compressed/uncompressed
    //! \param    [in] inputResource
    //!            The surface resource will be decompressed
    //! \param    [out] outputResource
    //!            The target uncompressed surface resource will be copied to
    //! \param    [in] preferMethod
    //!            The preferred copy mode
    //! \return   MOS_STATUS_SUCCESS if succeeded, else error code.
    //!
    MOS_STATUS MediaCopy(
        PMOS_RESOURCE inputResource,
        PMOS_RESOURCE outputResource,
        MCPY_METHOD   preferMethod);

private:
    PMOS_INTERFACE     m_osInterface     = nullptr;
    MediaCopyBaseState *m_mediaCopyState = nullptr;
MEDIA_CLASS_DEFINE_END(MediaCopyWrapper)
};
#endif // !__MEDIA_COPY_WRAPPER_H__
