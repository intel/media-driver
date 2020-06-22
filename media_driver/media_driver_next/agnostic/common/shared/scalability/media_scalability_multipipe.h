/*
* Copyright (c) 2018, Intel Corporation
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
//! \file     media_scalability_multipipe.h
//! \brief    Defines the common interface for media scalability mulitpipe mode.
//! \details  The media scalability mulitpipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_MULTIPIPE_H__
#define __MEDIA_SCALABILITY_MULTIPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "media_scalability.h"

class MediaScalabilityMultiPipe: public MediaScalability
{

public:
    MediaScalabilityMultiPipe(MediaContext *mediaContext) : MediaScalability(mediaContext){};
    
    //!
    //! \brief  Media scalability mulitipipe destructor
    //!
    virtual ~MediaScalabilityMultiPipe(){};

    //!
    //! \brief  Update the media scalability mulitipipe state mode
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS UpdateState();

protected:
    inline bool IsFirstPipe() { return (m_currentPipe == 0) ? true : false; }
    inline bool IsFirstPass() { return (m_currentPass == 0) ? true : false; }
    inline bool IsLastPipe() { return (m_currentPipe == (m_pipeNum - 1)) ? true : false; }

    inline bool IsPipeReadyToSubmit() { return (m_currentPipe == (m_pipeIndexForSubmit - 1)) ? true : false; }
};
#endif // !__MEDIA_SCALABILITY_MULTIPIPE_H__
