/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     cp_interfaces_next.h
//! \brief    Defines CP interfaces for DecodeCpInterface and CpStreamOutInterface
//!

#ifndef _CP_INTERFACES_NEXT_H_
#define _CP_INTERFACES_NEXT_H_

#include <stdint.h>
#include <va/va.h>
#include "cp_factory.h"
#include "cplib_utils.h"
#include "cp_streamout_interface.h"
#include "decodecp_interface.h"
#include "cp_interfaces.h"

//!
//! \class  CpInterfaces
//! \brief  Ddi media protected
//!
class CpInterfacesNext : public CpInterfaces
{
public:
    //!
    //! \brief Constructor
    //!
    CpInterfacesNext() {}

    //!
    //! \brief Destructor
    //!
    virtual ~CpInterfacesNext() {}

    //!
    //! \brief   Create CpStreamOutInterface Object
    //!          Must use Delete_CpStreamOutInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] pipeline
    //!          MediaPipeline*
    //! \param   [in] task
    //!          MediaTask*
    //! \param   [in] hwInterface
    //!          CodechalHwInterface*
    //!
    //! \return  CpStreamOutInterface*
    //!          Return CP Wrapper Object
    //!
    virtual CpStreamOutInterface *Create_CpStreamOutInterface(
        MediaPipeline *pipeline,
        MediaTask *task,
        CodechalHwInterface *hwInterface);

    //!
    //! \brief   Delete the CpStreamOutInterface Object
    //!
    //! \param   [in] pInterface
    //!          CpStreamOutInterface
    //!
    virtual void Delete_CpStreamOutInterface(CpStreamOutInterface *pInterface);

    //!
    //! \brief   Create DecodeCpInterface Object
    //!          Must use Delete_DecodeCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] codechalSettings
    //!          CodechalSetting*
    //! \param   [in] hwInterfaceInput
    //!          CodechalHwInterface*
    //!
    //! \return  DecodeCpInterface*
    //!          Return CP Wrapper Object
    //!
    virtual DecodeCpInterface *Create_DecodeCpInterface(
        CodechalSetting *    codechalSettings,
        CodechalHwInterface *hwInterfaceInput);

    //!
    //! \brief   Delete the DecodeCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          DecodeCpInterface
    //!
    virtual void Delete_DecodeCpInterface(DecodeCpInterface *pInterface);

};

#endif /*  _CP_INTERFACES_NEXT_H_ */
