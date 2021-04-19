/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     media_libva_interface_next.h
//! \brief    libva interface next head file
//!

#ifndef __MEDIA_LIBVA_INTERFACE_NEXT_H__
#define __MEDIA_LIBVA_INTERFACE_NEXT_H__

#include <va/va.h>
#include <va/va_backend.h>

class MediaLibvaInterfaceNext
{
public:
    //!
    //! \brief  Initialize
    //! 
    //! \param  [in] ctx
    //!         Pointer to VA driver context
    //! \param  [in] devicefd
    //!         Devoce fd
    //! \param  [out] major_version
    //!         Major version
    //! \param  [out] minor_version
    //!         Minor version
    //!
    //! \return VAStatus
    //!     VA_STATUS_SUCCESS if success, else fail reason
    //!
    static VAStatus Initialize (
        VADriverContextP ctx,
        int32_t          devicefd,
        int32_t         *major_version,     /* out */
        int32_t         *minor_version      /* out */
    );
};

#endif //__MEDIA_LIBVA_INTERFACE_NEXT_H__