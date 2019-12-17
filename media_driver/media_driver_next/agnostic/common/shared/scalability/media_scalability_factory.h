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
//! \file     media_scalability.h
//! \brief    Defines the common interface for media scalability
//! \details  The media scalability interface is further sub-divided by component,
//!           this file is for the base interface which is shared by all components.
//!

#ifndef __MEDIA_SCALABILITY_FACTORY_H__
#define __MEDIA_SCALABILITY_FACTORY_H__
#include "mos_os.h"
#include "media_scalability_defs.h"
#include "media_scalability.h"

//class MediaScalability;
class MediaScalabilityFactory
{
public:

    //!
    //! \brief  Create scalability, it should be invoked when new scalability mode needed
    //! \param  [in] componentType
    //!         component type for create related scalability
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to Media context
    //! \param  [in, out] gpuCtxCreateOption
    //!         Pointer to the option for GPU ctx create.
    //! \return pointer of media scalability
    //!
    MediaScalability* CreateScalability(
        uint8_t componentType,
        ScalabilityPars *params,
        void *hwInterface,
        MediaContext *mediaContext,
        MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption);

protected:
    //!
    //! \brief  Create encode scalability, it should be invoked when new scalability mode needed
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to Media context
    //! \param  [in, out] gpuCtxCreateOption
    //!         Pointer to the option for GPU ctx create.
    //! \return pointer of media scalability
    //!
    MediaScalability *CreateEncodeScalability(ScalabilityPars *params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption);

    //!
    //! \brief  Create decode scalability, it should be invoked when new scalability mode needed
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to Media context
    //! \param  [in, out] gpuCtxCreateOption
    //!         Pointer to the option for GPU ctx create.
    //! \return pointer of media scalability
    //!
    MediaScalability *CreateDecodeScalability(ScalabilityPars *params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption);

    //!
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to Media context
    //! \param  [in, out] gpuCtxCreateOption
    //!         Pointer to the option for GPU ctx create.
    //! \return pointer of media scalability
    //!
    MediaScalability *CreateVpScalability(ScalabilityPars *params, void *hwInterface, MediaContext *mediaContext, MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption);

    //!
    //! \brief  Create scalability with Mdf interfaces, it should be invoked when new scalability mode needed
    //!
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \return pointer of media scalability
    //!
    MediaScalability *CreateScalabilityMdf(ScalabilityPars *params);

    //!
    //! \brief  Create scalability with CMD Buf interfaces, it should be invoked when new scalability mode needed
    //! \param  [in] componentType
    //!         component type for create related scalability
    //! \param  [in] params
    //!         Pointer to the input parameters for scalability mode decision
    //! \param  [in] hwInterface
    //!         Pointer to HwInterface
    //! \param  [in] mediaContext
    //!         Pointer to Media context
    //! \param  [in, out] gpuCtxCreateOption
    //!         Pointer to the option for GPU ctx create.
    //! \return pointer of media scalability
    //!
    MediaScalability *CreateScalabilityCmdBuf(
        uint8_t componentType,
        ScalabilityPars *params,
        void *hwInterface,
        MediaContext *mediaContext,
        MOS_GPUCTX_CREATOPTIONS *gpuCtxCreateOption);

};

#endif // !__MEDIA_SCALABILITY_FACTORY_H__
