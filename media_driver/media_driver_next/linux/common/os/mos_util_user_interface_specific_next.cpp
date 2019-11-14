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
//! \file     mos_util_user_interface_specific.cpp
//! \brief    Linux specific supporting code for mos_util_user_interface
//!

#include "mos_util_user_interface_next.h"

//!
//! \brief mutex for mos util user interface multi-threading protection
//!
MosMutexNext MosUtilUserInterfaceNext::m_mutexLock = PTHREAD_MUTEX_INITIALIZER;

MOS_STATUS MosUtilUserInterfaceNext::MosUtilUserInterfaceInit(PRODUCT_FAMILY productFamily)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    // lock mutex to avoid multi init and close in multi-threading env
    MosUtilities::MosLockMutex(&m_mutexLock);
    MosUtilUserInterfaceNext* mosUtilInst = GetInstance(productFamily);
    MosUtilities::MosUnlockMutex(&m_mutexLock);

    return eStatus;
}

MOS_STATUS MosUtilUserInterfaceNext::MosUtilUserInterfaceClose()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MosUtilities::MosLockMutex(&m_mutexLock);
    Destroy();
    MosUtilities::MosUnlockMutex(&m_mutexLock);

    return eStatus;
}
