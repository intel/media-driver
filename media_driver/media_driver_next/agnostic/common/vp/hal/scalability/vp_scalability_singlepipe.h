/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     vp_scalability_singlepipe.h
//! \brief    Defines the common interface for media scalability singlepipe mode.
//! \details  The media scalability singlepipe interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __VP_SCALABILITY_SINGLEPIPE_H__
#define __VP_SCALABILITY_SINGLEPIPE_H__
#include "mos_defs.h"
#include "mos_os.h"
#include "media_scalability_singlepipe.h"
#include "vp_scalability_option.h"
#include "vp_pipeline_common.h"

namespace vp
{
class VpScalabilitySinglePipe : public MediaScalabilitySinglePipe
{

public:
    //!
    //! \brief  VP scalability singlepipe constructor
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] componentType
    //!         Component type
    //!
    VpScalabilitySinglePipe(void *hwInterface, MediaContext *mediaContext, uint8_t componentType);

    //!
    //! \brief  Encode scalability singlepipe destructor
    //!
    virtual ~VpScalabilitySinglePipe();

    //!
    //! \brief    Copy constructor
    //!
    VpScalabilitySinglePipe(const VpScalabilitySinglePipe&) = delete;

    //!
    //! \brief    Copy assignment operator
    //!
    VpScalabilitySinglePipe& operator=(const VpScalabilitySinglePipe&) = delete;

    //!
    //! \brief   Initialize the vp single scalability
    //! \details It will prepare the resources needed in scalability
    //!          and initialize the state of scalability
    //! \param   [in] option
    //!          Input scalability option
    //! \return  MOS_STATUS
    //!          MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Initialize(const MediaScalabilityOption &option) override;

protected:

    virtual MOS_STATUS SendAttrWithFrameTracking(MOS_COMMAND_BUFFER &cmdBuffer, bool frameTrackingRequested) override
    {
        return MOS_STATUS_SUCCESS;
    }

private:
    PVP_MHWINTERFACE  m_hwInterface = nullptr;
};
}
#endif // !__VP_SCALABILITY_SINGLEPIPE_H__

