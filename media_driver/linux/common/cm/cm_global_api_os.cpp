/*
* Copyright (c) 2017, Intel Corporation
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
//! \file  cm_global_api_os.cpp
//! \brief Contains implementations of gloabl CM APIs which are Linux-dependent.
//!

#include "cm_device_rt.h"

//!
//! \brief    Creates a CmDevice from a MOS context.
//! \details  If an existing CmDevice has already associated to the MOS context,
//!           the existing CmDevice will be returned. Otherwise, a new CmDevice
//!           instance will be created and associatied with that MOS context.
//! \param    pMosContext
//!           [in] pointer to MOS conetext.
//! \param    pDevice
//!           [in,out] reference to the pointer to the CmDevice.
//! \param    devCreateOption
//!           [in] option to customize CmDevice.
//! \retval   CM_SUCCESS if the CmDevice is successfully created.
//! \retval   CM_NULL_POINTER if pMosContext is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t CreateCmDevice(MOS_CONTEXT *pMosContext,
                                 CmDevice* &pDevice,
                                 uint32_t devCreateOption)
{
    if (pMosContext == nullptr)
    {
        return CM_NULL_POINTER;
    }
    CmDeviceRT* pDeviceRT = nullptr;
    //Check reference count
    if(pMosContext->cmDevRefCount > 0)
    {
        pDevice = (CmDevice *)pMosContext->pCmDev;
        if (pDevice == nullptr)
        {
            return CM_NULL_POINTER;
        }
        pMosContext->cmDevRefCount ++;
        pDeviceRT = static_cast<CmDeviceRT*>(pDevice);
        return pDeviceRT->RegisterSyncEvent(nullptr);
    }

    int32_t ret = CmDeviceRT::Create(pMosContext, pDeviceRT, devCreateOption);
    if(ret == CM_SUCCESS)
    {
        pDevice = pDeviceRT;
        pMosContext->pCmDev = pDeviceRT;
        pMosContext->cmDevRefCount ++;
        return pDeviceRT->RegisterSyncEvent(nullptr);
    }

    return CM_FAILURE;
}//===================

//!
//! \brief    Destroys the CmDevice associated with MOS context. 
//! \details  This function also destroys surfaces, kernels, programs, samplers,
//!           threadspaces, tasks and the queues that were created using this
//!           device instance but haven't explicitly been destroyed by calling
//!           respective destroy functions. 
//! \param    pMosContext
//!           [in] pointer to MOS conetext.
//! \retval   CM_SUCCESS if CmDevice is successfully destroyed.
//! \retval   CM_NULL_POINTER if MOS context is null.
//! \retval   CM_FAILURE otherwise.
//!
CM_RT_API int32_t DestroyCmDevice(MOS_CONTEXT *pMosContext)
{
    if (pMosContext == nullptr)
    {
        return CM_NULL_POINTER;
    }

    if(pMosContext->cmDevRefCount > 1)
    {
       pMosContext->cmDevRefCount --;
       return CM_SUCCESS;
    }
    
    pMosContext->cmDevRefCount -- ;
   
    pMosContext->SkuTable.reset();
    pMosContext->WaTable.reset();
 
    CmDevice *pDevice =  (CmDevice *)(pMosContext->pCmDev);
    CmDeviceRT* pDeviceRT = static_cast<CmDeviceRT*>(pDevice);
    int32_t ret = CmDeviceRT::Destroy(pDeviceRT);
    if (ret != CM_SUCCESS)
    {
        return ret;
    }

    pMosContext->pCmDev = nullptr;
    
    return CM_SUCCESS;
}//===================
