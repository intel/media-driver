/*
* Copyright (c) 2009-2017, Intel Corporation
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

#include "mos_util_user_interface.h"

//!
//! \brief mutex for mos util user interface multi-threading protection
//!
static MOS_MUTEX mosUtilUserIntrMutex = PTHREAD_MUTEX_INITIALIZER;

MOS_STATUS MosUtilUserInterfaceInit(PRODUCT_FAMILY productFamily)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    // lock mutex to avoid multi init and close in multi-threading env
    MOS_LockMutex(&mosUtilUserIntrMutex);
    MosUtilUserInterface* mosUtilInst = MosUtilUserInterface::GetInstance(productFamily);
    MOS_UnlockMutex(&mosUtilUserIntrMutex);

    return eStatus;
}

MOS_STATUS MosUtilUserInterfaceClose()
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;

    MOS_LockMutex(&mosUtilUserIntrMutex);
    MosUtilUserInterface::Destroy();
    MOS_UnlockMutex(&mosUtilUserIntrMutex);

    return eStatus;
}
