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
//! \file     cp_interfaces.h
//! \brief    Defines base class for CP interfaces
//!

#ifndef _CP_INTERFACES_H_
#define _CP_INTERFACES_H_

#include <stdint.h>
#include <va/va.h>
#include "cp_factory.h"
#include "codechal_secure_decode_interface.h"
#include "mhw_cp_interface.h"
#include "mos_os_cp_interface_specific.h"
#ifndef WDDM_LINUX
#include "media_libva_cp_interface.h"
#include "media_libva_caps_cp_interface.h"
#else
class DdiCpInterface;
class MediaLibvaCaps;
class MediaLibvaCapsCpInterface;
struct DDI_MEDIA_CONTEXT;
#endif
#include "cp_streamout_interface.h"
#include "decodecp_interface.h"

#define CP_INTERFACE            1

//!
//! \class  DdiMediaProtected
//! \brief  Ddi media protected
//!
class CpInterfaces
{
public:
    //!
    //! \brief Constructor
    //!
    CpInterfaces() {}

    //!
    //! \brief Destructor
    //!
    virtual ~CpInterfaces() {}

    //!
    //! \brief   Create CodechalSecureDeocde Object
    //!          Must use Delete_SecureDecodeInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] codechalSettings
    //!          CodechalSetting*
    //! \param   [in] codechalSettings
    //!          CodechalHwInterface*
    //!
    //! \return  CodechalSecureDecodeInterface*
    //!          Return CP Wrapper Object
    //!
    virtual CodechalSecureDecodeInterface* Create_SecureDecodeInterface(
        CodechalSetting *    codechalSettings,
        CodechalHwInterface *hwInterfaceInput) = 0;

    //!
    //! \brief   Delete the CodechalSecureDecodeInterface Object
    //!
    //! \param   [in] pInterface
    //!          CodechalSecureDecodeInterface*
    //!
    virtual void Delete_SecureDecodeInterface(CodechalSecureDecodeInterface *pInterface) = 0;

    //!
    //! \brief   Create MhwCpInterface Object
    //!          Must use Delete_MhwCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] osInterface
    //!          PMOS_INTERFACE
    //!
    //! \return  MhwCpInterface*
    //!          Return CP Wrapper Object
    //!
    virtual MhwCpInterface* Create_MhwCpInterface(PMOS_INTERFACE osInterface) = 0;

    //!
    //! \brief   Delete the MhwCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          MhwCpInterface*
    //!
    virtual void Delete_MhwCpInterface(MhwCpInterface *pInterface) = 0;

    //!
    //! \brief   Create MosCpInterface Object
    //!          Must use Delete_MosCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] pvOsInterface
    //!          void*
    //! \param   [in] pInterface
    //!          MosCpInterface*
    //!
    //! \return  MosCpInterface*
    //!          Return CP Wrapper Object
    //!
    virtual MosCpInterface* Create_MosCpInterface(void* pvOsInterface) = 0;

    //!
    //! \brief   Delete the MosCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          MosCpInterface*
    //!
    virtual void Delete_MosCpInterface(MosCpInterface* pInterface) = 0;

    //!
    //! \brief   Create DdiCpInterface Object
    //!          Must use Delete_DdiCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] mosCtx
    //!          MOS_CONTEXT&
    //! \param   [in] pInterface
    //!          DdiCpInterface*
    //!
    //! \return  DdiCpInterface*
    //!          Return CP Wrapper Object
    //!
    virtual DdiCpInterface* Create_DdiCpInterface(MOS_CONTEXT& mosCtx) = 0;

    //!
    //! \brief   Delete the DdiCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          DdiCpInterface*
    //!
    virtual void Delete_DdiCpInterface(DdiCpInterface* pInterface) = 0;

    //!
    //! \brief   Create MediaLibvaCapsCpInterface Object
    //!          Must use Delete_MediaLibvaCapsCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \param   [in] mediaCtx
    //!          DDI_MEDIA_CONTEXT*
    //! \param   [in] mediaCaps
    //!          MediaLibvaCaps*
    //!
    //! \return  MediaLibvaCapsCpInterface*
    //!          Return CP Wrapper Object
    //!
    virtual MediaLibvaCapsCpInterface* Create_MediaLibvaCapsCpInterface(
        DDI_MEDIA_CONTEXT *mediaCtx,
        MediaLibvaCaps *mediaCaps) = 0;

    //!
    //! \brief   Delete the MediaLibvaCapsCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          MediaLibvaCapsCpInterface*
    //!
    virtual void Delete_MediaLibvaCapsCpInterface(MediaLibvaCapsCpInterface* pInterface) = 0;

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
        MhwCpInterface  *cpInterface,
        PMOS_INTERFACE   osInterface) = 0;

    //!
    //! \brief   Delete the DecodeCpInterface Object
    //!
    //! \param   [in] pInterface
    //!          DecodeCpInterface
    //!
    virtual void Delete_DecodeCpInterface(DecodeCpInterface *pInterface) = 0;

    MEDIA_CLASS_DEFINE_END(CpInterfaces)
};

typedef CpFactoryWithoutArgs<CpInterfaces> CpInterfacesFactory;

#endif /*  _CP_INTERFACES_H_ */
