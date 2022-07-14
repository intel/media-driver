/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     media_scalability_singlepipe.h
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_SINGLEPIPE_H__
#define __MEDIA_SCALABILITY_SINGLEPIPE_H__
#include <stdint.h>
#include "mos_defs.h"
#include "mos_os_specific.h"
#include "media_scalability_singlepipe_next.h"
class MediaContext;
class MediaScalabilityOption;
class MhwMiInterface;

class MediaScalabilitySinglePipe : public MediaScalabilitySinglePipeNext
{

public:
    //!
    //! \brief  Media scalability singlepipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] componentType
    //!         Component type
    //!
    MediaScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Media scalability singlepipe destructor
    //!
    virtual ~MediaScalabilitySinglePipe(){};

    //!
    //! \brief  Submit command buffer
    //! \param  [in, out] cmdBuffer
    //!         Pointer to command buffer
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SubmitCmdBuffer(PMOS_COMMAND_BUFFER cmdBuffer) override;

    virtual MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

    //!
    //! \brief  Set hint parameters
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetHintParams() override;

    //!
    //! \brief  Destroy the media scalability
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Destroy() override;

protected:
    MhwMiInterface *         m_miInterface          = nullptr;  //!< Mi interface used to add BB end
    MEDIA_CLASS_DEFINE_END(MediaScalabilitySinglePipe)
};

#endif // !__MEDIA_SCALABILITY_SINGLEPIPE_LEGACY_H__

