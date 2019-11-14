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
//! \file     mos_util_user_interface_next.h
//! \brief    Common MOS util user feature key service across different platform 
//! \details  Common MOS util user feature key service across different platform
//!
#ifndef __MOS_UTIL_USER_INTERFACE_NEXT_H__
#define __MOS_UTIL_USER_INTERFACE_NEXT_H__

#include "igfxfmid.h"
#include "mos_utilities.h"
#include <map>

class MosUtilUserInterfaceNext
{
public:
    MosUtilUserInterfaceNext();
    virtual ~MosUtilUserInterfaceNext() {}

    static MosUtilUserInterfaceNext *GetInstance(PRODUCT_FAMILY productFamily = IGFX_MAX_PRODUCT);

    static void Destroy();

    static MOS_STATUS AddEntry(uint32_t keyId, PMOS_USER_FEATURE_VALUE userFeatureKey);
    static MOS_STATUS DelEntry(uint32_t keyId);

    static PMOS_USER_FEATURE_VALUE GetValue(uint32_t keyId);

    virtual MOS_STATUS Initialize() {return MOS_STATUS_SUCCESS;}
    static bool  IsDefaultValueChanged() { return m_defaultValueChanged; }

    //!
    //! \brief    Init Function for MOS utilities user interface
    //! \details  Initial MOS utilities user interface related structures, and only execute once for multiple entries
    //! \param    [in] platform
    //!           Pointer to current platform info. Will use default platform when it's nullptr
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MosUtilUserInterfaceInit(PRODUCT_FAMILY productFamily = IGFX_MAX_PRODUCT);

    //!
    //! \brief    Close Function for MOS util user interface
    //! \details  close/remove MOS utilities user interface related structures, and only execute once for multiple entries
    //! \return   MOS_STATUS
    //!           Returns one of the MOS_STATUS error codes if failed,
    //!           else MOS_STATUS_SUCCESS
    //!
    static MOS_STATUS MosUtilUserInterfaceClose();

private:
    static MosUtilUserInterfaceNext* m_inst;
    static uint32_t              m_refCount;    // UMD entry could be multi-threaded, need reference count to keep only do initialization once.
    static std::map<uint32_t, PMOS_USER_FEATURE_VALUE>  m_userFeatureKeyMap;

protected:
    static bool  m_defaultValueChanged;
private:
    static MosMutexNext m_mutexLock;
};





#endif // __MOS_UTIL_USER_INTERFACE_NEXT_H__
